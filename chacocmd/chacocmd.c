
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
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

#define D(x)   (isgraph(buf[i+x]) ? buf[i+x] : '.')
#define M(x)   (buf[i+x])
void print_dump(unsigned char *buf, int len, int addr)
{
    int i;
    for (i = 0;i < len; i+= 0x10) {
            printf("%08x: %02x %02x %02x %02x %02x %02x %02x %02x  %02x %02x %02x %02x %02x %02x %02x %02x"
                   "  %c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n", addr + i,
                   M(0),M(1),M(2),M(3),M(4),M(5),M(6),M(7), M(8),M(9),M(10),M(11),M(12),M(13),M(14),M(15),
                   D(0),D(1),D(2),D(3),D(4),D(5),D(6),D(7), D(8),D(9),D(10),D(11),D(12),D(13),D(14),D(15)
            );
    }
}
#undef M
#undef D

void usage (void)
{
    printf(
    "chacocmd (beta %s)\n"
    "usage: chacocmd <options>\n"

    "-h --help                                  this help\n"

    "--verbose                                  enable verbose messages\n"
    "--debug                                    enable debug messages\n"
    "--noprogress                               disable the progress indicator\n"

    "--info                                     show flash info\n"
    "--status                                   show core status\n"

    "--addr <num>                               set start address\n"
    "--len <num>                                set length\n"

    "--dumpmem                                  show memory dump\n"
    "--readmem <name>                           read RAM to file\n"
    "--writemem <name>                          write file to RAM\n"

    "--dumpflash                                show flash dump\n"
    "--readslot <slot> <name>                   read slot image to file\n"
    "--readimage <name>                         read full image to file\n"
    "--flashrbf <slot> <corename> <romname>     flash .rbf/.rom\n"
    "--flashslot <slot> <name>                  flash slot image\n"
    "--flashimage <name>                        flash full image\n"
    "--eraseslot <slot>                         erase flash slot\n"

    "--start <slot>                             start core\n"
    "--jtagslot <slot>                          set jtag slot\n"
    "--bootloader                               start microcontroller bootloader\n"
    , __DATE__
    );
}

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

static void makename(char *d, char *s)
{
    int n;
    char *p;
    char ch;

    n = strlen(s);
    if (n) {
        --n;
        p = s + n;
        while ((n >= 0) && (*p != '/') && (*p != '\\')) {
            p--;
            --n;
        }
        if((n < 0) || (*p == '/') || (*p == '\\')) {
            p++;
        }

        n=0;
        while(*p) {
            ch = *p++;
            if (ch == '_') {
                ch = ' ';
            }
            if (ch == '.') {
                break;
            }
            *d++ = ch;
            n++;
            if (n == CORENAME_MAXLEN) {
                break;
            }
        }
    }

    *d=0;
}

int checkname(char *n)
{
    if (n == NULL) {
        return -1; /* invalid */
    }
    if ((n[0] == '-') && (n[1] == '-')) {
        return 0; /* is option */
    }
    return 1; /* is name */
}

unsigned int loadfile(unsigned char *buffer, char *name, unsigned int maxlen)
{
    unsigned int len;
    FILE *f;

    f = fopen(name, "rb");
    if (f == NULL) {
        fprintf(stderr, "error opening: '%s'\n", name);
        exit(EXIT_FAILURE);
    }
    len = fread(buffer, 1, maxlen, f);
    fclose(f);
    return len;
}

static void printstatus(int spiactive,int usbcap,int bricked,int cfgdone,int nstatus)
{
    printf("core is valid:        %s\n", bricked ? "no" : "yes");
    printf("core is USB capable:  %s\n", usbcap ? "yes" : "no");
    printf("SPI currently active: %s\n", spiactive ? "yes" : "no");
    printf("cfgdone: %02x\n", cfgdone);
    printf("nstatus: %02x\n", nstatus);
}

static void checksdcard(int sdinserted)
{
    if (!sdinserted) {
        LOGERR("please remove the sd card before flashing.\n");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[])
{
    char *core_name;
    char *rom_name;
    unsigned char *buffer;
    unsigned char *buffer2;
    int spiactive, usbcap, bricked, cfgdone, nstatus;
    int mcversion, sdinserted;
    int ret, addr, len, i, slotnum = 0, romlen, progressbar = 1;
    COREINFO cinfo;

    FILE *f;

    addr = 0x00000000;
    len = 0;

    if (argc < 2) {
        usage();
        exit (EXIT_FAILURE);
    }

    /* first check the options that should work before any other stuff is used */
    for (i = 1; i < argc; i++) {
        if (!strcmp("--verbose", argv[i])) {
            verbose = 1;
        } else if (!strcmp("--debug", argv[i])) {
            verbose = 2;
        } else if (!strcmp("--noprogress", argv[i])) {
            progressbar = 0;
        } else if (!strcmp("-h", argv[i]) || !strcmp("--help", argv[i]))  {
            usage();
            exit(EXIT_FAILURE);
        } else {
            break;
        }
    }

    if (i == argc) {
        usage();
        exit (EXIT_FAILURE);
    }

    if (chameleon_init() < 0) {
        LOGERR("USB initialization failed.\n");
        exit(EXIT_FAILURE);
    }
    if (chameleon_getversion(&mcversion, &sdinserted) < 0) {
        LOGERR("getversion failed.\n");
        exit(EXIT_FAILURE);
    }
    LOGVER("Firmware Version: %02x\n", mcversion);
    LOGVER("sd card detected: %s\n", sdinserted ? "no" : "yes");

    buffer = (unsigned char*)malloc(CHAMELEON_RAM_SIZE);
    buffer2 = (unsigned char*)malloc(CHAMELEON_RAM_SIZE);

#ifdef LINUX
    /* make sure that if the binary is setuid root, the created files will be
       owned by the user running the binary (and not root) */
    {
        int fsuid = setfsuid(getuid());
        int fsgid = setfsgid(getgid());
        LOGVER("fsuid: %d fsgid: %d\n", fsuid, fsgid);
    }
#endif

    if (progressbar) {
        chameleon_setprogressfunc(1000, (void (*)(unsigned int, unsigned int))progress);
    }
    chameleon_setlogfunc(logfunc);

    /* check the rest of the options */
    for (; i < argc; i++) {
        if (!strcmp("--addr", argv[i])) {
            i++;
            addr = strtoul(argv[i], NULL, 0);
        } else if (!strcmp("--len", argv[i])) {
            i++;
            len = strtoul(argv[i], NULL, 0);
        } else if (!strcmp("--writemem", argv[i])) {
            i++;
            f = fopen(argv[i], "rb");
            if (f == NULL) {
                fprintf(stderr, "error opening: '%s'\n", argv[i]);
                exit(EXIT_FAILURE);
            }
            if (len == 0) len = CHAMELEON_RAM_SIZE;
            len = fread(buffer, 1, len, f);
            printf("sending '%s' (%d bytes to %08x.)...\n", argv[i], len, addr);
            fclose(f);
            progressmsg = "Writing";
            ret = chameleon_writememory(buffer, len, addr);
        } else if (!strcmp("--readmem", argv[i])) {
            i++;
            f = fopen(argv[i], "wb");
            if (f == NULL) {
                fprintf(stderr, "error opening: '%s'\n", argv[i]);
                exit(EXIT_FAILURE);
            }
            if (len == 0) len = 0x100;
            printf("getting '%s' (%d bytes from %08x.)...\n", argv[i], len, addr);
            progressmsg = "Reading";
            ret = chameleon_readmemory(buffer, len, addr);
            len = fwrite(buffer, 1, len, f);
            fclose(f);
        } else if (!strcmp("--dumpmem", argv[i])) {
            if (len == 0) len = 0x100;
            progressmsg = "Reading";
            ret = chameleon_readmemory(buffer, len, addr);
            print_dump(buffer, len, addr);
        } else if (!strcmp("--flashrbf", argv[i])) {
            unsigned char *rom_buffer = (unsigned char*)malloc(CHAMELEON_RAM_SIZE);

            checksdcard(sdinserted);

            i++;
            slotnum = strtoul(argv[i], NULL, 0);
            addr = slotnum << 20;
            i++;
            core_name = argv[i];

            if (checkname(core_name) != 1) {
                fprintf(stderr, "error: core name expected\n");
                exit(EXIT_FAILURE);
            }
            makename(cinfo.core_name, core_name);

            printf("using core name: '%s'\n", cinfo.core_name);
            len = loadfile(buffer, core_name, CHAMELEON_SLOT_SIZE);

            cinfo.core_length = len;
            printf("flashing core from '%s' (%d bytes to %08x.)...\n", core_name, len, addr);

            rom_name = argv[i + 1];
            romlen = 0;
            if (checkname(rom_name) == 1) {
                i++;
                romlen = loadfile(rom_buffer, rom_name, CHAMELEON_SLOT_SIZE);
//                print_dump(&rom_buffer[romlen - 0x10000], 0x10000, romlen - 0x10000);
                printf("flashing roms from '%s' (%d bytes to %08x.)...\n", rom_name, romlen, addr + len);
            }
            cinfo.rom_length = romlen;
            if ((len + romlen + 3) > CHAMELEON_SLOT_SIZE) {
                fprintf(stderr, "error: binary size exceeds slot size.\n");
                exit(EXIT_FAILURE);
            }

            memset (buffer2, 0xff, CHAMELEON_SLOT_SIZE);
            len = chameleon_prepareslot(buffer2, buffer, &cinfo);

            if (romlen) {
                memcpy(&buffer2[len], rom_buffer, romlen);
                len += romlen;
            }
            progressmsg = "Flashing";
            ret = chameleon_writeflash(buffer2, (len + 0xffff) & ~0xffff, addr);
            free(rom_buffer);
        } else if (!strcmp("--flashslot", argv[i])) {
            checksdcard(sdinserted);
            i++;
            slotnum = strtoul(argv[i], NULL, 0);
            addr = slotnum << 20;
            i++;
            f = fopen(argv[i], "rb");
            if (f == NULL) {
                fprintf(stderr, "error opening: '%s'\n", argv[i]);
                exit(EXIT_FAILURE);
            }
            memset (buffer, 0xff, CHAMELEON_SLOT_SIZE);
            len = fread(buffer, 1, CHAMELEON_SLOT_SIZE, f);
            printf("flashing '%s' (%d bytes to %08x.)...\n", argv[i], len, addr);
            fclose(f);
            progressmsg = "Flashing";
            ret = chameleon_writeflash(buffer, (len + 0xffff) & ~0xffff, addr);
        } else if (!strcmp("--flashimage", argv[i])) {
            checksdcard(sdinserted);
            i++;
            f = fopen(argv[i], "rb");
            if (f == NULL) {
                fprintf(stderr, "error opening: '%s'\n", argv[i]);
                exit(EXIT_FAILURE);
            }
            memset (buffer, 0xff, CHAMELEON_FLASH_SIZE);
            len = fread(buffer, 1, CHAMELEON_FLASH_SIZE, f);
            printf("flashing '%s' (%d bytes to %08x.)...\n", argv[i], len, addr);
            fclose(f);
            progressmsg = "Flashing";
            ret = chameleon_writeflash(buffer, (len + 0xffff) & ~0xffff, 0);
        } else if (!strcmp("--eraseslot", argv[i])) {
            checksdcard(sdinserted);
            i++;
            slotnum = strtoul(argv[i], NULL, 0);
            addr = slotnum << 20;
            len = CHAMELEON_SLOT_SIZE;
            progressmsg = "Erasing";
            ret = chameleon_eraseflash((len + 0xffff) & ~0xffff, addr);
        } else if (!strcmp("--readslot", argv[i])) {
            i++;
            slotnum = strtoul(argv[i], NULL, 0);
            addr = slotnum << 20;
            i++;
            f = fopen(argv[i], "wb");
            if (f == NULL) {
                fprintf(stderr, "error opening: '%s'\n", argv[i]);
                exit(EXIT_FAILURE);
            }
            printf("getting '%s' (%d bytes from %08x.)...\n", argv[i], CHAMELEON_SLOT_SIZE, addr);
            progressmsg = "Reading Flash";
            ret = chameleon_readflash(buffer, CHAMELEON_SLOT_SIZE, addr);
            len = fwrite(buffer, 1, CHAMELEON_SLOT_SIZE, f);
            fclose(f);
        } else if (!strcmp("--readimage", argv[i])) {
            i++;
            f = fopen(argv[i], "wb");
            if (f == NULL) {
                fprintf(stderr, "error opening: '%s'\n", argv[i]);
                exit(EXIT_FAILURE);
            }
            printf("getting '%s' (%d bytes from %08x.)...\n", argv[i], CHAMELEON_FLASH_SIZE, addr);
            progressmsg = "Reading Flash";
            ret = chameleon_readflash(buffer, CHAMELEON_FLASH_SIZE, addr);
            len = fwrite(buffer, 1, CHAMELEON_FLASH_SIZE, f);
            fclose(f);
        } else if (!strcmp("--dumpflash", argv[i])) {
            if (len == 0) len = 0x100;
            progressmsg = "Reading Flash";
            ret = chameleon_readflash(buffer, len, addr);
            print_dump(buffer, len, addr);
        } else if (!strcmp("--start", argv[i])) {
            i++;
            slotnum = strtoul(argv[i], NULL, 0);
            ret = chameleon_startcore(slotnum);
            chameleon_getstatus(&spiactive,&usbcap,&bricked, &cfgdone, &nstatus);
            printstatus(spiactive,usbcap,bricked,cfgdone,nstatus);
        } else if (!strcmp("--jtagslot", argv[i])) {
            i++;
            slotnum = strtoul(argv[i], NULL, 0);
            ret = chameleon_setjtagslot(slotnum);
        } else if (!strcmp("--bootloader", argv[i])) {
            ret = chameleon_startbootloader();
        } else if (!strcmp("--status", argv[i])) {
            chameleon_getstatus(&spiactive,&usbcap,&bricked, &cfgdone, &nstatus);
            printstatus(spiactive,usbcap,bricked,cfgdone,nstatus);
        } else if (!strcmp("--info", argv[i])) {
            int ii;
            printf("Slot Core                                     Core Data     Core Info     ROM Data      \n");
            printf("Nr   Name                                     Offs   Len    Offs   Len    Offs   Len    \n");
            for (ii = 0;ii < CHAMELEON_NUM_SLOTS; ii++) {
                memset(&cinfo, 0, sizeof(COREINFO));
                if ((ret = chameleon_getcoreinfo(ii, &cinfo)) == 0) {
                    printf("%2d:  %-41s %06lx %06lx %06lx %06lx %06lx %06lx\n", ii,
                        cinfo.core_name,
                        cinfo.core_offset, cinfo.core_length,
                        cinfo.info_offset, cinfo.rom_offset - cinfo.info_offset,
                        cinfo.rom_offset, cinfo.rom_length
                       );
                } else {
#if 0
                    printf("%-41s %06lx %06lx %06lx %06lx %06lx %06lx  !!!!\n",
                        cinfo.core_name,
                        cinfo.core_offset, cinfo.core_length,
                        cinfo.info_offset, cinfo.rom_offset - cinfo.info_offset,
                        cinfo.rom_offset, cinfo.rom_length
                       );
#else
                    printf("%2d:  empty.\n", ii);
#endif
                }
            }
        } else {
            usage();

            chameleon_close();

            free(buffer);
            free(buffer2);

            exit (EXIT_FAILURE);
        }
    }

    chameleon_close();

    free(buffer);
    free(buffer2);

    return EXIT_SUCCESS;
}
