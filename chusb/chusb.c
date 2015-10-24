#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>

#ifdef POSIX
#include <unistd.h>
#endif
#ifdef LINUX
#include <sys/fsuid.h>
#endif

#include "chacolib.h"

#ifndef DEBUG
/* #define DEBUG */
#endif

#define LOGERR(...)       logfunc (LOGLVL_ERR, __VA_ARGS__ )
#define LOGVER(...)       logfunc (LOGLVL_VER, __VA_ARGS__ )
#define LOGMSG(...)       logfunc (LOGLVL_MSG, __VA_ARGS__ )
#define DBG(...)          logfunc (LOGLVL_DBG, __VA_ARGS__ )

int verbose = 0;

char *progressmsg = NULL;

void progress(unsigned int percent, unsigned int value)
{
    printf("\r%s: %d.%d%% (%d bytes).", progressmsg, percent / 10, percent % 10, value);
    if(percent==1000)printf("\n");
    fflush(stdout);
}

int logfunc (int lvl, const char * format, ...)
{
    int printed = 0;
    va_list ap;
    va_start (ap, format);

    switch (lvl) {
        case LOGLVL_ERR:
            printed = vfprintf (stderr, format, ap);
            break;
        case LOGLVL_VER:
            if (verbose >= 1) printed = vfprintf (stdout, format, ap);
            break;
        case LOGLVL_MSG:
            printed = vfprintf (stdout, format, ap);
            break;
        case LOGLVL_DBG:
            if (verbose >= 2) printed = vfprintf (stdout, format, ap);
            break;
    }

    va_end (ap);
    return printed;
}

// -----------------------------------------------------------------------------

unsigned char * buf = NULL;
unsigned char * buf2 = NULL;
unsigned char * filebuf = NULL;
unsigned char checksum;
unsigned char * bufread = NULL;
unsigned char * bufstart = NULL;
int written;
int fileptr;
unsigned char ft;
int entoff;

int shutdown(int n)
{
    chameleon_close();
    if(buf) {
        free(buf);
    }
    if(buf2) {
        free(buf2);
    }
    if(filebuf) {
        free(filebuf);
    }
    if(bufstart) {
        free(bufstart);
    }
    if(bufread) {
        free(bufread);
    }
    return n;
}

void execute_run(void)
{
    unsigned char buf[4];
    /* put RUN into keyboard buffer */
    buf[0] = 0;
    if (chameleon_writememory(buf, 1, 198) < 0) {
        LOGERR("error writing to chameleon memory.\n");
        exit(shutdown(-1));
    }
    buf[0] = 82;
    buf[1] = 85;
    buf[2] = 78;
    buf[3] = 13;
    if (chameleon_writememory(buf, 4, 631) < 0) {
        LOGERR("error writing to chameleon memory.\n");
        exit(shutdown(-1));
    }
    buf[0] = 4;
    if (chameleon_writememory(buf, 1, 198) < 0) {
        LOGERR("error writing to chameleon memory.\n");
        exit(shutdown(-1));
    }
}
// -----------------------------------------------------------------------------

#include "chbidir.c"

#define ftdi_write_data(x,y,z) usb_write_bytes(y,z)
#define ftdiwrite(y,z) usb_write_bytes(y,z)
#define ftdi_read_data(x,y,z) usb_read_bytes(y,z)
#define ftdiread(y,z) usb_read_bytes(y,z)

/*
 *      0 - execute (resets)
 *      1 - copy ?
 *      2 - copy disk image to c64 (00:kernal, ff:turbo)
 *      3 - copy disk image from c64 (00:kernal, ff:turbo)
 *      5 - usb loopback test
 *      6 - turbo format disk
 *      7 - copy files from c64
 * n/a  8 - tap writer
 * n/a  9 - send prg to ef3 menu
 *
 */
void startcommand_noack(unsigned char cmd)
{
    // SYNC BYTES
    bufstart[0] = 0xB3;
    bufstart[1] = 0x68;
    bufstart[2] = 0x92;

    bufstart[3] = cmd;

//    printf("startcommand %02x, %02x, %02x, %02x\n", bufstart[0], bufstart[1], bufstart[2], bufstart[3]);
    written = ftdiwrite(bufstart, 4);         // send sync & command
//    printf(" ok\n");
}

void startcommand(unsigned char cmd)
{
    startcommand_noack(cmd);
    written = ftdiread(bufstart, 1);          // get back ACK
//    printf("  startcommand ACK %02x\n", bufstart[0]);
}

// -----------------------------------------------------------------------------

int d81 = 0;
int nrTracks=40;
int usednum;
unsigned char usedtr[1024];
unsigned char usedse[1024];

static int sectorsPerTrack[] = {
    21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,
    19,19,19,19,19,19,19,
    18,18,18,18,18,18,
    17,17,17,17,17,
    17,17,17,17,17 };
static int sectorsPerTrack71[] = {
    21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,
    19,19,19,19,19,19,19,
    18,18,18,18,18,18,
    17,17,17,17,17,
    21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,
    19,19,19,19,19,19,19,
    18,18,18,18,18,18,
    17,17,17,17,17 };

char* filetypes[] = {
    "DEL\0","SEQ\0","PRG\0","USR\0","REL\0","CBM\0" };

unsigned char c64filename[22];

int linearSector(int track, int sector)
{
    int i;
    int* spt = sectorsPerTrack;

        if (nrTracks == 70) {
                spt = sectorsPerTrack71;
        }

	if ((track<1) || (track>nrTracks)) {
		fprintf(stderr, " - Illegal track %d\n", track);
//		exit(-1);
                return -1;
        
	}

        if (!d81) {
        	if ((sector<0) || (sector>=spt[track-1])) {
        		fprintf(stderr, " - Illegal sector %d for track %d (max is %d)\n", sector, track, sectorsPerTrack[track-1]);
//        		exit(-1);
                        return -1;
        	}
        } else if (sector>39) {
		fprintf(stderr, " - Illegal sector %d (max is %d)\n", sector, 39);
//		exit(-1);
                return -1;
        }

        if (d81) {
                return (track-1)*40 + sector;
        }

	int linearSector=0;
	for (i=0;i<track-1;i++) {
		linearSector+=spt[i];
        }
	linearSector+=sector;

	return linearSector;
}

int off(int track, int sector)
{
    int tr = linearSector(track, sector);
    if (tr != -1) {
        return tr*256;
    }
    return -1;
}

int sendfile()
{
        int i;
        
        printf(" - Len: %6d bytes. Sending file. Bytes left:", fileptr);
        
        bufstart[0] = ft;        // filetype
        written = ftdiwrite(bufstart, 1);

        // filename
        written = ftdiwrite(buf+entoff+0x05, 16);

        // filelen in 3 bytes
        bufstart[0] = (unsigned char) (fileptr&0xff);        // len lo
        bufstart[1] = (unsigned char) ((fileptr>>8)&0xff);   // len hi                                                
        bufstart[2] = (unsigned char) ((fileptr>>16)&0xff);  // len hi hi
        written = ftdiwrite(bufstart, 3);  
        
        // calc checksum                                                  
        checksum = 0;
        for (i=0; i < fileptr; i++) {
              checksum = checksum + filebuf[i];
        }
        bufstart[0] = checksum;
        written = ftdiwrite(bufstart, 1);

        // data
        for (i=0; i < fileptr; i++) {
                ftdiwrite(filebuf+i,1);
                printf("%6d%c%c%c%c%c%c",fileptr-i,8,8,8,8,8,8);
        }
        printf("%c%c%c%c%c%c%c%c%c%c%cDone.                    \n",8,8,8,8,8,8,8,8,8,8,8);
        //written = ftdiwrite(filebuf, fileptr);
        
        // get ack
        written = ftdiread(bufread, 1);
        if (bufread[0] != 0xff) {
                printf("Error on the C64 side ... exiting...\n");
                return 1;
        }
        return 0;
}

void sendindicator(int format, int track, int sector)
{
    // send the position of the '*' on the c64 screen
    int add;
    if (format == 80) {
        // d81
        add = 0x0400 + (3+(sector/2))*40 + ((track-1)/2);
        unsigned char vis;
        if (track%2) {
                // 1,2
                if (sector%2) {
                        vis = 0x32;
                } else {
                        vis = 0x31;
                }
        } else {
                // 3,*
                if (sector%2) {
                        vis = 0x2a;
                } else {
                        vis = 0x33;
                }
        }
        bufstart[2] = vis;     // 1,2,3,*
    } else {
        // d64
        add = 0x0400 + (3+sector)*40 + (track-1);
        bufstart[2] = 0x2a;     // '*'
    }
    bufstart[0] = (unsigned char) (add&0xff);
    bufstart[1] = (unsigned char) ((add>>8)&0xff);
    written = ftdiwrite(bufstart,3);        // send over the coordinates and character
}                                

//------------------------------------------------------------------------------

void printusage()
{
   printf("Usage: chusb [command] [file] [options]\n");
   printf(" e[xecute]  file.prg|p00                   - execute prg on c64\n");
   printf(" c[opy]     file.prg|p00|d64|d81|d71       - copy files to c64\n");
   printf(" x[fer]     [p00]                          - copy files from c64\n");
   printf(" w[rite]    file.d64|d81 [verify] [kernal] - write image on c64\n");
   printf(" r[ead]     file.d64|d81 [40] [kernal]     - read image from c64\n");
   printf(" d[ir]      file.d64|d81|d71               - display dir of file and check it\n");
   printf(" f[ormat]   [40]                           - turbo format 1541 floppy\n");
   printf(" s[end]     [file.prg]                     - send file.prg to C64\n");
   printf("                                             if no file then send chusb.prg\n");
   printf(" 0[test]                                   - test the usb connection\n");
   exit(1);
}

//------------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    char *fname = NULL;
    FILE *fp = NULL;
    int i;
    int read = 0;
    int mode = 0;
    int mode40 = 0;
    int command = -1;
    int addr = 0;
    int verify = 0;

    char p00start[8] = "C64File\0";
  
    printf("Chameleon USB Client v1.8\n");
    if (argc < 2) {
        printusage();
    }

    chameleon_setlogfunc(logfunc);

    if (chameleon_init() < 0) {
        LOGERR("initialization failed.\n");
        exit(-1);
    }

//    printf("\n");
    switch(argv[1][0]) {
        case 'e': command = 0;
                  printf(" - EXECUTE FILE\n");
                  if (argc != 3) {
                        printusage();
                  }
                  fname = argv[2];
                  break;
        case 'c': command = 1;
                  printf(" - COPY FILE(S) TO C64\n");
                  if (argc != 3) {
                        printusage();
                  }
                  fname = argv[2];
                  break;
        case 'w': command = 2;
                  printf(" - WRITE IMAGE\n");
                  if (argc == 5) {
                        mode = 1;
                        verify = 1;
                  }
                  else if (argc == 4) {
                        if (argv[3][0] == 'k') {
                                mode = 1;
                        } else {
                                verify = 1;
                        }
                  }
                  fname = argv[2];
                  break;
        case 'r': command = 3;
                  printf(" - READ IMAGE\n");
                  if (argc == 5) {
                        mode = 1;
                        mode40 = 1;
                  } else if (argc == 4) {
                        if (argv[3][0] == 'k') {
                                mode = 1;
                        } else {
                                mode40 = 1;
                        }
                  }
                  fname = argv[2];
                  break;
        case 'd': command = 1;
                  printf(" - DIR\n");
                  if (argc != 3) {
                        printusage();
                  }
                  fname = argv[2];
                  mode = 1;
                  break;        
        case 'f': command = 6;
                  printf(" - TURBO FORMAT\n");
                  if (argc == 3) {
                        if (argv[2][0] != '4') {
                                printusage();
                        }
                        mode40 = 1;
                  }
                  break;        
        case '0': command = 5;
                  printf(" - TEST USB CONNECTION\n");
                  break;
        case 'x': command = 7;
                  printf(" - COPY FILE(S) FROM C64\n");
                  if (argc == 3) {
                        mode = 1;
                  }
                  break;
#if 1 /* not supported! */
        case 't': command = 8;
                  printf(" - WRITE TAP FILE\n");
                  if (argc != 3) {
                        printusage();
                  }
                  fname = argv[2];
                  break;
#endif
        case 's': command = 9;
                  printf(" - SEND PRG TO C64\n");
                  if (argc == 3) {
						fname = argv[2];
                  }
                  break;
        default: printusage();
    }
//    printf("\n");
    //--------------------------------------------------------------------------

    buf = (unsigned char *) malloc(0x1000000);
    filebuf = (unsigned char *) malloc(0x1000000);
    bufread = (unsigned char *) malloc(30);
    bufstart = (unsigned char *) malloc(30);

    if ((fname != NULL) && (command != 3)) {
        if (!(fp = fopen(fname, "rb"))) {
            fprintf(stderr, "can't open %s\n", fname);
            return 0;
        }
        read = fread (buf, 1, 0x1000000, fp);
        fclose(fp);
    }
  
    if (command == 7) {
        //----------------------------------------------------------------------
        // COPY FILES FROM C64
        int fnlen;

        startcommand(0x07);

        printf("- Waiting for files from C64...\n");

        written = ftdiread(bufstart,1);                         // get status
        
        while (bufstart[0] == 0) {
                for (i=0; i < 19; i++) {
                        c64filename[i]=0;
                }
                written = ftdiread(bufstart,1);                         // get file length
                fnlen = bufstart[0];
                written = ftdiread(c64filename, fnlen);           // get filename

                printf("Transfering '%-17s'  Bytes: ", c64filename);
                
                int ok = 0;
                int len=0;
                while (!ok) {
                        written = ftdiread(buf+len,1);                 // get byte
                        len++;
                        printf("%6d%c%c%c%c%c%c",len,8,8,8,8,8,8);
                        written = ftdiread(bufstart, 1);          // status
                        ok = bufstart[0];
                }
                printf("\n");

                // fix filename
                for(i=0; i < fnlen; i++) {
                        if (c64filename[i]>=0x20 && c64filename[i]<=0x5f) {
                                bufstart[i]=c64filename[i];
                        } else {
                                bufstart[i]=0x5f;
                        }
                }
                
                bufstart[fnlen]='.';
                bufstart[fnlen+1]='P';
                bufstart[fnlen+4]=0;
                if (mode == 0) {
                        // PRG
                        bufstart[fnlen+2]='R';
                        bufstart[fnlen+3]='G';
                } else {
                        // P00
                        bufstart[fnlen+2]='0';
                        bufstart[fnlen+3]='0';
                }                
//                printf("writing file '%s'.\n", (char*)bufstart);
#if 1
                if (!(fp = fopen((char*)bufstart, "wb"))) {
                        printf("Can't open %s for writing !\n", bufstart);
                } else {
                        if (mode == 1) {
                                fwrite(p00start, 1 , 8, fp);
                                fwrite(c64filename, 1, 18, fp);
                        }
                        fwrite (buf, 1, len, fp);
                        fclose(fp);                
                }
#endif
                written = ftdiread(bufstart,1);                         // get status
        }
        printf("- End of transfer\n");
    } else if (command == 6) {
        //----------------------------------------------------------------------
        // TURBO FORMAT DISK
        startcommand(0x06);

        bufstart[0] = 35;                   // normal mode
        if (mode40 == 1) {
            bufstart[0] = 40;
        }
        written = ftdiwrite(bufstart,1);
        printf("Formating disk with %d tracks...\n", bufstart[0]);
        written = ftdiread(bufstart,1);
    } else if (command == 2) {
        //----------------------------------------------------------------------
        // WRITE IMAGE

          int format = 0;
          
          if (read == 174848 || read == 175531) {
                format = 35;
          } else if (read == 196608 || read == 197376) {
                format = 40;
          } else if (read == 819200 || read == 822400) {
                format = 80;
          }
          
          if (format == 40 && mode == 1) {
                printf("40 tracks D64 is not supported in Kernal mode!\n");
                exit(shutdown(-1));;
          }
          
          if (format == 0) {
                printf("Unsupported Image format!\n");
                exit(shutdown(-1));;
          } else {
                if (format == 35 || format == 40) {
                        printf (" - .D64 format %d tracks.\n\n", format);
                } else {
                        printf (" - .D81 format %d tracks.\n\n", format);        
                        mode = 1;
                }
          }
          
        int track;
        int sector;
        int pos = 0;
        
        if (mode == 1) {
                //--------------------------------------------------------------
                // KERNAL WRITE
        printf("Transferring image to C64. (kernal) ");

                  startcommand(0x02);
                  
                  bufstart[0] = 0x00;                   // kernal write
                  written = ftdiwrite(bufstart,1);

                  if (verify) {
                          bufstart[0] = 3;
                  } else {
                          bufstart[0] = 0;              // verify retries #
                  }
                  written = ftdiwrite(bufstart,1);
                  
                  for (track=1; track < format+1; track++) {
                        int numsec;
                        if (format != 80) {
                                numsec = sectorsPerTrack[track-1];
                        } else {
                                numsec = 40;
                        }
                        for (sector=0; sector < numsec; sector++) {
                                printf("T:%2d S:%2d%c%c%c%c%c%c%c%c%c",track,sector,8,8,8,8,8,8,8,8,8);
                                
                                bufstart[0] = 0xff;                     // indicate that we are transferring a sector
                                written = ftdiwrite(bufstart, 1);
                                
                                sprintf((char*)bufstart,"%2d %2d",track,sector);
                                written = ftdiwrite(bufstart, 5);       // send over the string containing track & sector
                                
                                // calc checksum                                                  
                                checksum = 0;
                                for (i=0; i < 256; i++) {
                                        checksum = checksum + buf[pos+i];
                                }
                                bufstart[0] = checksum;
                                written = ftdiwrite(bufstart, 1);                 

                                // send the sector over
                                //written = ftdiwrite(buf+pos, 256);
                                written = usb_write_block(buf+pos, 256);
                                pos += 256;

                                sendindicator(format, track, sector);                                
                                
                                // get ack
                                written = ftdiread(bufread, 1);
                                if (bufread[0] != 0xff) {
                                        printf("Error on the C64 side ... exiting...\n");
                                        exit(shutdown(-1));;
                                }
                        }
                  }
                  bufstart[0] = 0;        // end of transfer
                  written = ftdiwrite(bufstart, 1);
                  printf("\nDone\n");
                  
        } else {
                //--------------------------------------------------------------
                // TURBO WRITE
        printf("Transferring image to C64. (turbo) ");

                  startcommand(0x02);
                  
                  bufstart[0] = 0xff;                   // turbo write
                  written = ftdiwrite(bufstart,1);

                  if (verify) {
                          bufstart[0] = 3;
                  } else {
                          bufstart[0] = 0;              // verify retries #
                  }
                  written = ftdiwrite(bufstart,1);

                  for (track=1; track < format+1; track++) {
                        int numsec;
                        if (format != 80) {
                                numsec = sectorsPerTrack[track-1];
                        } else {
                                numsec = 40;
                        }
                        for (sector=numsec-1; sector >=0 ; sector--) {
                                printf("T:%2d S:%2d%c%c%c%c%c%c%c%c%c",track,sector,8,8,8,8,8,8,8,8,8);
                                
                                bufstart[0] = 0xff;                     // indicate that we are transferring a sector
                                written = ftdiwrite(bufstart, 1);
                                
                                // send the sector over
                                //written = ftdiwrite(buf + off(track,sector), 256);
                                written = usb_write_block(buf + off(track,sector), 256);
                
                                bufstart[0] = (unsigned char) track;
                                bufstart[1] = (unsigned char) sector;                
                                written = ftdiwrite(bufstart, 2);       // send over the string containing track & sector
                                
                                sendindicator(format, track, sector);                                
                                
                                // get ack
                                written = ftdiread(bufread, 1);
                                if (bufread[0] != 0xff) {
                                        printf("Error on the C64 side ... exiting...\n");
                                        exit(shutdown(-1));;
                                }
                        }
                  }
                  bufstart[0] = 0;        // end of transfer
                  written = ftdiwrite(bufstart, 1);
                  printf("\nDone\n");
                     
        }
    } else if (command == 3) {
        //----------------------------------------------------------------------
        // READ IMAGE
        
        int fnamelen = strlen(fname);
        int format = 0;
        if (fnamelen < 4 || fname[fnamelen-4]!='.') {
              format = 0;
        }
        else if (fname[fnamelen-1]=='1' && fname[fnamelen-2]=='8') {
                mode =1;
                format = 80;
        }
        else if (fname[fnamelen-1]=='4' && fname[fnamelen-2]=='6') {
                if (mode40 == 1) {
                        format = 40;
                } else {
                        format = 35;
                }
        }
        if (format == 0 || (format == 80 && mode == 0)) {
                printf("%d Unsupported format or mode!\n", format);
                exit(shutdown(-1));;
        }
        
        if (mode == 1 && format == 40) {
                printf("40 tracks D64 is not supported in Kernal mode!\n");
                exit(shutdown(-1));;
        }
        
        if (format == 35 || format == 40) {
                printf (" - .D64 format %d tracks.\n\n", format);
        } else {
                printf (" - .D81 format %d tracks.\n\n", format);        
        }
        
        int track;
        int sector;
        int pos = 0;
        
        if (mode == 1) {
                //--------------------------------------------------------------
                // KERNAL READ
                  
            printf("Transferring image from C64 (kernal) ");

                  startcommand(0x03);
                  
                  bufstart[0] = 0x00;                   // kernal read
                  written = ftdiwrite(bufstart,1);
                  
                  for (track=1; track < format+1; track++) {
                        int numsec;
                        if (format != 80) {
                                numsec = sectorsPerTrack[track-1];
                        } else {
                                numsec = 40;
                        }
                        for (sector=0; sector < numsec; sector++) {
                                printf("T:%2d S:%2d%c%c%c%c%c%c%c%c%c",track,sector,8,8,8,8,8,8,8,8,8);
                                
                                bufstart[0] = 0xff;                     // indicate that we are transferring a sector
                                written = ftdiwrite(bufstart, 1);
                                
                                sprintf((char*)bufstart,"%2d %2d",track,sector);
                                written = ftdiwrite(bufstart, 5);       // send over the string containing track & sector
                                
                                // send the sector over
                                //written = ftdiread(buf+pos, 256);
                                written = usb_read_block(buf+pos, 256);

                                // calc checksum                                                  
                                checksum = 0;
                                for (i=0; i < 256; i++) {
                                        checksum = checksum + buf[pos+i];
                                }
                                bufstart[0] = checksum;
                                written = ftdiwrite(bufstart, 1);     // send our checksum over
                                
                                pos += 256;
                                
                                sendindicator(format, track, sector);
                                
                                // get ack
                                written = ftdiread(bufread, 1);
                                if (bufread[0] != 0xff) {
                                        printf("Error on the C64 side ... exiting...\n");
                                        exit(shutdown(-1));;
                                }
                        }
                  }
                  bufstart[0] = 0;        // end of transfer
                  written = ftdiwrite(bufstart, 1);
                  printf("\nDone... Writing image with %d bytes length.\n", pos);
                
                
                   if (!(fp = fopen(fname, "wb"))) {
                    fprintf(stderr, "can't open %s\n", fname);
                    return 0;
                  }
                
                  written = fwrite (buf, 1, pos, fp);
                  fclose(fp);
                   
        } else {
                //--------------------------------------------------------------
                // TURBO READ

            printf("Transferring image from C64 (turbo) ");

                  startcommand(0x03);
                  
                  bufstart[0] = 0xff;                   // turbo read
                  written = ftdiwrite(bufstart,1);
                  int size = 0;

                  for (track=1; track < format+1; track++) {
                        int numsec;
                        if (format != 80) {
                                numsec = sectorsPerTrack[track-1];
                        } else {
                                numsec = 40;
                        }
                        for (sector=numsec-1; sector >=0; sector--) {
                                printf("T:%2d S:%2d%c%c%c%c%c%c%c%c%c",track,sector,8,8,8,8,8,8,8,8,8);
                                
                                bufstart[0] = 0xff;                     // indicate that we are transferring a sector
                                written = ftdiwrite(bufstart, 1);
                
                                bufstart[0] = (unsigned char) track;
                                bufstart[1] = (unsigned char) sector;                
                                written = ftdiwrite(bufstart, 2);       // send over the string containing track & sector
                                
                                // send the sector over
                                //written = ftdiread(buf + off(track,sector), 256);
                                written = usb_read_block(buf + off(track,sector), 256);

                                size = size+256;
                                
                                sendindicator(format, track, sector);
                                
                                // get ack
                                written = ftdiread(bufread, 1);
                                if (bufread[0] != 0xff) {
                                        printf("Error on the C64 side ... exiting...\n");
                                        exit(shutdown(-1));;
                                }
                        }
                  }
                  bufstart[0] = 0;        // end of transfer
                  written = ftdiwrite(bufstart, 1);
                  printf("\nDone... Writing image with %d bytes length.\n", size);  

                   if (!(fp = fopen(fname, "wb"))) {
                    fprintf(stderr, "can't open %s\n", fname);
                    return 0;
                  }
                
                  written = fwrite (buf, 1, size, fp);
                  fclose(fp);
                   
        }
                   
    } else if (command == 5) {
        //----------------------------------------------------------------------
        // USB Loopback Test
        int j;

        buf2 = (unsigned char *) malloc(0x10);
        printf("Started Testing...\n");

        startcommand(0x05);

        written = ftdiread(buf, 1);

        for (j=0; j < 64; j++) {
            for (i=0; i < 256; i++) {
                buf[0] = (unsigned char) i;
                written = ftdi_write_data(&ftdic, buf, 1);
                if (written != 1) {
                    printf("ERROR - could not write all test bytes to the device!\n");
                    exit(shutdown(-1));
                }
                written = ftdi_read_data(&ftdic, buf2, 1);
                if (written != 1) {
                    printf("ERROR - could not read back all test bytes to the device!\n");
                    exit(shutdown(-1));
                }
                if (buf2[0] != buf[0]) {
                    printf("ERROR - Read byte ($%02X) does not match written byte ($%02X) !\n", buf2[0], i);
                    exit(shutdown(-1));
                }
            }
        }

        free(buf2);
        printf("Testing was Succesfull ! Restart C64...\n");
    } else if (command == 0) {
        //----------------------------------------------------------------------
        // EXECUTE

          // P00 ?
          int fnamelen = strlen(fname);
          if ((fname[fnamelen-3]=='P' || fname[fnamelen-3]=='p') && fname[fnamelen-2]=='0') {
            read = read-0x1a;    // new filelength
            for (i=0; i <read; i++)      // copy file to filebuf
            {
                buf[i] = buf[0x1A + i];
            }
          }

            if (check_server_running()) {
                startcommand_noack(0x00);
                sleep(1);
                while (check_server_running()) {
                    /* wait for server to terminate */
                    sleep(1);
                }
            }
            addr = buf[0] + (buf[1] << 8); read-=2;
            printf("sending '%s' ($%04x bytes to $%04x.)...\n", fname, read, addr);
            if (chameleon_writememory(&buf[2], read, addr) < 0) {
                LOGERR("error writing to chameleon memory.\n");
                exit(shutdown(-1));
            }
            execute_run();
    } else if (command == 8) {
        //----------------------------------------------------------------------
        // TAP WRITER
        LOGERR("TAP writing not supported.");
    } else if (command == 9) {
        //----------------------------------------------------------------------
        // SEND PRG TO C64
            if (check_server_running()) {
                startcommand_noack(0x00);
                sleep(1);
                while (check_server_running()) {
                    /* wait for server to terminate */
                    sleep(1);
                }
            }
            if (fname == NULL) {
                fname = "chusb.prg";
            }
            if((fp = fopen(fname, "rb")) == NULL) {
                LOGERR("error could not open '%s'.\n", fname);
                exit(shutdown(-1));
            }
            read = fread (buf, 1, 0x1000000, fp);
            fclose(fp);
            addr = buf[0] + (buf[1] << 8); read-=2;
            printf("sending '%s' ($%04x bytes to $%04x.)...\n", fname, read, addr);
            if (chameleon_writememory(&buf[2], read, addr) < 0) {
                LOGERR("error writing to chameleon memory.\n");
                exit(shutdown(-1));
            }
            execute_run();
    } else if (command == 1) {
        //----------------------------------------------------------------------
        // COPY

          unsigned char nexttr = 18;
          unsigned char nextse = 1;
          
          int prgmode = 0;
          
          int fnamelen = strlen(fname);
          if (fnamelen < 5 || fname[fnamelen-4]!='.') {
                prgmode = 1;
          } else if (fname[fnamelen-1]=='1' && fname[fnamelen-2]=='8') {
                d81 = 1;
                nexttr = 40;
                nextse = 03;
                nrTracks = 80;
          } else if (fname[fnamelen-1]=='1' && fname[fnamelen-2]=='7') {
                nrTracks = 70;
          } else if ((fname[fnamelen-3]=='P' || fname[fnamelen-3]=='p') &&
                     (fname[fnamelen-2]=='R' || fname[fnamelen-2]=='r') &&
                     (fname[fnamelen-1]=='G' || fname[fnamelen-1]=='g')) {
                prgmode = 1;
          } else if ((fname[fnamelen-3]=='P' || fname[fnamelen-3]=='p') &&
                      fname[fnamelen-2]=='0') {
                prgmode = 2;
          }
          
          if (mode == 0) {
                  startcommand(0x01);
          }
          
          if (prgmode != 0) {
                if (mode != 0) {
                        printf("DIR only supported on disk images!\n");
                        exit(shutdown(-1));;
                }
                ft = 0x02;      // PRG filetype
                
                //buf+entoff+0x05 = filename
                
                if (prgmode == 2) {
                        // P00
                        entoff = 0x08 - 0x05; //filename
                        for (i=0; i < 16; i++) {
                                if (buf[entoff+0x05+i] == 0) {
                                        buf[entoff+0x05+i] = 0xA0;      // fix filename
                                }
                        }
                        fileptr = read-0x1a;    // filelength
                        for (i=0; i <fileptr; i++) {     // copy file to filebuf
                                filebuf[i] = buf[0x1A + i];
                        }
                } else {
                        // PRG
                        fileptr = read;
                        entoff = fileptr;
                        int stop = 0;
                        // check filename start
                        int fnamelen = strlen(fname);
                        int start = fnamelen-1;
                        while (start > 0) {
                                if (fname[start] == '\\' || fname[start] == '/') {
                                        start = start + 1;
                                        break;
                                }
                                start = start-1;
                        }
                        
                        for (i=start+0; i < start+16; i++) {
                                if ((i >= (int)strlen(fname)) || stop) {
                                        buf[entoff+0x05+(i-start)]=0xA0;            // pad
                                } else {
                                        if ((fname[i] == '.') && (i == (int)strlen(fname) - 4)) {
                                                stop = 1;
                                                buf[entoff+0x05+(i-start)]=0xA0;            // pad
                                        } else if (fname[i] < 0x60) {
                                                buf[entoff+0x05+(i-start)] = fname[i];
                                        } else {
                                                buf[entoff+0x05+(i-start)] = fname[i]-0x20;
                                        }
                                }
                        }
                        for (i=0; i <fileptr; i++)      // copy file to filebuf
                        {
                                filebuf[i] = buf[i];
                        }
                }
                for (i=0; i < 16; i++) {
                        c64filename[i] = buf[entoff+0x05+i];
                        if (c64filename[i] == 0xA0) {
                                c64filename[i] = ' ';
                        }
                }
                c64filename[i] = 0;
                
                printf(" PRG %s %3d", c64filename, fileptr/256);
        
                if (sendfile()) {
                        exit(shutdown(0));;
                }
          } else {
                  int tt;
                  tt = 0x90;
                  if (d81 == 1) {
                        tt = 0x04;
                  }
                  for (i=0; i < 23; i++) {
                          c64filename[i] = buf[off(nexttr,0)+tt+i];
                          if (c64filename[i] == 0xa0) {
                                c64filename[i] = ' ';
                          }
                  }
                  c64filename[16] = '"';
                  c64filename[17] = ',';
                  printf("    \"%s\n",c64filename);
                  
                  while (nexttr != 0) {
                          int diroff = off(nexttr,nextse);
                          if (diroff == -1) {
                                exit(shutdown(0));;
                          }
                          nexttr = buf[diroff+0];
                          nextse = buf[diroff+1];
                //          printf("lin: $%x next tr %d se %d\n", diroff, nexttr, nextse);
                          
                          int pos = 0;
                          while (pos < 8) {
                                entoff = diroff + pos*0x20;
                                ft = buf[entoff+0x02];
                                int locked = 0;
                                int splat = 0;
                                if (ft&0x40) {
                                        locked = 1;
                                }
                                if (ft&0x80) {
                                        splat = 1;
                                }
                                ft = ft & 0x07;
                                unsigned char lc = ' ';
                                if (locked) {
                                        lc = '>';
                                }
                                else if (splat) {
                                        lc = ' ';
                                }
                                unsigned char filetr = buf[entoff+0x03];
                                unsigned char filese = buf[entoff+0x04];
                                int sizese = buf[entoff+0x1e] + buf[entoff+0x1f]*256;
                                
                                for (i=0; i < 16; i++) {
                                        c64filename[i] = buf[entoff+0x05+i];
                //                        printf("%d\n", c64filename[i]);
                                        if (c64filename[i] == 0xA0) {
                                                c64filename[i] = ' ';
                                        }
                                }
                                c64filename[i] = 0;
                                
                                if (!(ft == 00 && !locked && !splat)) {
                                        printf("%c%s %s %3d", lc, filetypes[ft], c64filename, sizese);
                                        int valid = 1;
                                        if (!d81 && filetr == 18 && (filese == 0 || filese == 1)) {
                                                valid = 0;
                                        }
                                        if (valid && (ft == 0x02 || ft == 0x01))  // PRG or SEQ
                                        {
                                                usednum = 0;
                                                int ok = 1;
                                                int corrupted = 0;
                                                //unsigned char currtr = filetr;
                                                //unsigned char currse = filese;
                                                fileptr = 0;
                                                while (ok) {
                                                        for (i=0; i < usednum; i++) {
                                                                if (usedtr[usednum] == filetr && usedse[usednum] == filese) {
                                                                        corrupted = 1;
                                                                        ok = 0;
                                                                        break;
                                                                }
                                                        }
                                                        
                                                        if (!corrupted) {
                                                                usedtr[usednum] = filetr;
                                                                usedse[usednum] = filese;
                                                                usednum++;
                                                        
                                                                int cs = off(filetr, filese);
                                                                if (cs == -1) {
                                                                        corrupted = 2;
                                                                        ok = 0;
                                                                }
                                                                
                                                                // next file pointer
                                                                filetr = buf[cs+0x00];
                                                                filese = buf[cs+0x01];
                                                                
                                                                int secsize = 254;
                                                                
                                                                if (filetr == 0x00)     // last sector
                                                                {
                                                                        secsize = filese-1;        
                                                                        ok = 0;         // exit
                                                                }
                                                                for (i=0; i < secsize; i++) {
                                                                        filebuf[fileptr] = buf[cs+0x02+i];
                                                                        fileptr++;
                                                                }
                                                         }
                                                }
                                                if (corrupted) {
                                                        if (corrupted == 1) {
                                                                printf(" - File Loop detected - corrupted file!\n");
                                                        }
                                                } else {
                                                        if (mode == 0) {
                                                                if (sendfile()) {
                                                                        exit(shutdown(0));;
                                                                }
                                                                
                                                                //FILE * fp1 = fopen (c64filename, "wb");
                                                                //fwrite(filebuf, 1, fileptr, fp1);
                                                                //fclose(fp1);
                
                                                        } else {
                                                                printf("\n");
                                                        }
                                                }
                                        } else {
                                                if (mode != 1) {
                                                        printf(" - skipping\n");
                                                } else {
                                                        printf("\n");
                                                }
                                        }
                                }
                                
                                pos++;
                          }
                
                  }
          }  
          if (mode == 0) {
                   bufstart[0] = 0;        // end of transfer
                   written = ftdiwrite(bufstart, 1);
          }
    }
    return shutdown(0);
}
