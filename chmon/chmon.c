
/* TODO:
 * Page Boundaries Disassembly
 * implement Transfer Memory Command
 * TRACE ?!
 */

/* changelog:
 * 03.03.2007: fixed dir output from command line
 * 02.03.2007: added Makefile
 * 02.03.2007: added support for filtered directory
 * 02.03.2007: changd tokenizer, changes in string treatment, strings in quotation marks etc...
 * 01.03.2007: fixed wrong memory end address display at PUT
 * 01.03.2007: added libreadline functions for Linux
 * 01.03.2007: added feature: give cmomands at command-line
 * 28.02./ 01.03.2007: rewrote network stuff for also compiling for WIN32
 * some minor changes...
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#ifdef _WIN32
//#include <winsock.h>
#include <windows.h>
#include <io.h>
#else
#include <sys/types.h>
#include <unistd.h>
#ifdef LINUX
#include <readline/readline.h>
#include <readline/history.h>
#endif
extern int h_errno;

#endif

/* chmon Constant Data */

const char version[] = "v0.2";

#define COMMANDS 29

const char command_table[COMMANDS][8] = {
        "m",            /* 00 */
        "i",            /* 01 */
        "d",            /* 02 */
        "g",            /* 03 */
        "h",            /* 04 */
        "f",            /* 05 */
        "c",            /* 06 */
        "put",          /* 07 */
        "get",          /* 08 */
        "poke",         /* 09 */
        "$",            /* 10 */
        "@",            /* 11 */
        "#",            /* 12 */
        "read",         /* 13 */
        "write",        /* 14 */
        "run",          /* 15 */
        "basic",        /* 16 */
        "ram",          /* 17 */
        "rom",          /* 18 */
        "reu",          /* 19 */
        "help",         /* 20 */
        "?",            /* 21 */
        "quit",         /* 22 */
        "x",            /* 23 */
        "status",       /* 24 */
        "init",         /* 25 */
        "call",         /* 26 */
        "cmd"           /* 27 */
};

enum {
        INIT,
        PUT,
        GET,
        READ,
        WRITE,
        XWRITE,
        XDONE,
        RUN,
        JUMP,
        SYS,
        BASIC,
        DIR,
        DISKCOMMAND,
        SETRAM,
        POKE,
        TELLDEV,
        SETDEV,
        FILL,
        STATUS,
};

/* These are used by the disassembler */

enum {
        IMPL,IMM,ABS,ABSX,ABSY,IND,INDX,INDY,ZP,ZPX,ZPY,REL,AKKU
};

char mnemo[256][4] = {
        "brk","ora","???","???","???","ora","asl","???",
        "php","ora","asl","???","???","ora","asl","???",
        "bpl","ora","???","???","???","ora","asl","???",
        "clc","ora","???","???","???","ora","asl","???",
        "jsr","ans","???","???","bit","and","rol","???",
        "plp","and","rol","???","bit","and","rol","???",
        "bmi","and","???","???","???","and","rol","???",
        "sec","and","???","???","???","and","rol","???",
        "rti","eor","???","???","???","eor","lsr","???",
        "pha","eor","lsr","???","jmp","eor","lsr","???",
        "bvc","eor","???","???","???","eor","lsr","???",
        "cli","eor","???","???","???","eor","lsr","???",
        "rts","adc","???","???","???","adc","ror","???",
        "pla","adc","ror","???","jmp","adc","ror","???",
        "bvs","adc","???","???","???","adc","ror","???",
        "sei","adc","???","???","???","adc","ror","???",
        "???","sta","???","???","sty","sta","stx","???",
        "dey","???","txa","???","sty","sta","stx","???",
        "bcc","sta","???","???","sty","sta","stx","???",
        "tya","sta","txs","???","???","sta","???","???",
        "ldy","lda","ldx","???","ldy","lda","ldx","???",
        "tay","lda","tax","???","ldy","lda","ldx","???",
        "bcc","lda","???","???","ldy","lda","ldx","???",
        "clv","lda","tsx","???","ldy","lda","ldx","???",
        "cpy","cmp","???","???","cpy","cmp","dec","???",
        "iny","cmp","dex","???","cpy","cmp","dec","???",
        "bne","mcp","???","???","???","cmp","dec","???",
        "cld","cmp","???","???","???","cmp","dec","???",
        "cpx","sbc","???","???","cpx","sbc","inc","???",
        "inx","sbc","nop","???","cpx","sbc","inc","???",
        "beq","sbc","???","???","???","sbc","inc","???",
        "sed","sbc","???","???","???","sbc","inc","???"
};

int table1[32] = {
        IMPL,INDX,IMPL,IMPL,IMPL,ZP,ZP,IMPL,IMPL,IMM,AKKU,IMPL,ABS,ABS,ABS,IMPL,
        REL,INDY,IMPL,IMPL,IMPL,ZPX,ZPX,IMPL,IMPL,ABSY,IMPL,IMPL,IMPL,ABSX,ABSX,IMPL
};

int table2[32] = {
        IMM,INDX,IMPL,IMPL,ZP,ZP,ZP,IMPL,IMPL,IMM,IMPL,IMPL,ABS,ABS,ABS,IMPL,
        REL,INDY,IMPL,IMPL,ZPX,ZPX,ZPY,IMPL,IMPL,ABSY,IMPL,IMPL,IMPL,ABSX,ABSX,IMPL
};


/* textstrings */
const char helptext[COMMANDS][400] = {
        "m [from [to]]\n"
        " memory peek: transfer mem from c64 and display in hex, ascii and petascii\n",
        "i [from [to]]\n"
        " interrogate: transfer mem from c64 and display as petascii\n",
        "d [from [to]]\n"
        " disassembly: transfer mem from c64 and display as disassembly\n",
        "g addr\n"
        " goto: tell c64 to jump to specified address\n",
        "h from to pattern\n"
        " hunt: find occurances of the given search pattern in memory area specified by from and to\n"
        " search pattern is given either as string in quotes or as series of hex values\n",
        "f from to value\n"
        " fill: fill the c64's memory area specified by from and to with value\n",
        "c from to with\n"
        " compare: compare the memory segment specified by from and to with the one starting at with\n",
        "put filename [loadaddr]\n"
        " put: transfer the contents of a local file into c64 memory, if loadaddr is not specified\n"
        " the load address is taken from the file's first two bytes (cbm standard)\n",
        "get from to filename\n"
        " get: transfer the memory segment specified by from and to from c64 and store in filename\n",
        "poke addr string | value[value...]]\n"
        " poke: write to c64 memory address. Value(s) are given as a series of hex numbers or as a string in quotation marks\n",
        "$[pattern]\n"
        " directory: display disk directory of current device. If no pattern is given '$*' is assumed.\n"
        " Patterns are passed directly to the disk device, so everything that your device understands is allowed,\n"
        " e.g. '$*=seq' on a cbm1541 to list all SEQ files.\n",
        "@[string]\n"
        " disk command: send any following string to command channel, read command channel and print to stdout\n",
        "#[dev]\n"
        " device: set the current device to dev. if dev is not specified, the current device is displayed\n",
        "read remotefile [localfile]\n"
        " read: read remotefile from c64's current device, transfer the data and store in localfile\n"
        " if localfile is not specified remotefile will be used as filename.\n",
        "write localfile [remotefile]\n"
        " write: write contents of localfile to remotefile on c64's current device\n"
        " if remotefile is not specified localfile will be used as filename.\n",
        "run\n"
        " tell c64 to perform a BASIC RUN Statement\n",
        "basic\n"
        " c64 will return to basic (quit server)\n",
        "ram\n"
        " switch acces mode to RAM: all memory write accesses will go to c64 internal RAM ($01 = #$34)\n",
        "rom\n"
        " switch access mode to ROM: all memory read aceesses will read from ROM ($01 = #$37)\n",
        "reu\n"
        " switch access mode to REU: all memory read and write accesses will go to expanded memory\n",
        "help\n"
        " show some help: you may get help for a specific command by typing help command\n",
        "?\n"
        " show some help: you may get help for a specific command by typing ? command\n",
        "quit\n"
        " quit the netmon shell\n",
        "x\n"
        " quit the netmon shell\n",
        "status\n"
        " get server status\n",
        "init\n"
        " initialize chserv\n",
        "call addr [a[x[y[sr]]]]\n"
        " call subroutine:specify values for .A,.X,.Y and status register before call\n",
        "cmd command [parameter [parameter...]]\n"
        " system command: send the following string to the system's command interpreter\n"
};

const char usage[]      =
        "chmon\n\n"
        "-s --send [name]     send server program (chmon.prg)\n"
        "-c --command [cmd]   direct command, type netmon -c help for a full list of available commands or netmon -c help name for usage of the given command\n"
        "-v --version         print version number\n"
        "-h --help            this help message\n";

/* the commands table */


/* default values */

const char prompt[]     = "[:.]";
//const char default_ip[] = "192.168.0.64";
//const char default_port[] = "3172";

#include "chacolib.h"
#include "chbidir.c"

void send_memory_request(unsigned base,unsigned size);

// -----------------------------------------------------------------------------

int senddatablock(unsigned char *buf, unsigned int len)
{
    unsigned char x = len;
    usb_write_bytes(&x,1);
//    printf("senddatablock len:%d\n", x);
    len = usb_write_bytes(buf,x);
//    printf("after senddatablock len:%d\n", len);
    return len;
}
#define send(fd,buf,len,x) senddatablock(buf,len)
int recvdatablock(unsigned char *buf, unsigned int len)
{
    unsigned char x = len;
    usb_read_bytes(&x,1);
//    printf("recvdatablock len:%d\n", x);
    len = usb_read_bytes(buf,x);
//    printf("->'%s'\n", buf);
//    printf("after recvdatablock got:%d\n", len);
    return len;
}
#define recv(fd,buf,len,x) recvdatablock(buf,len)

// -----------------------------------------------------------------------------

void execute_sys(int addr)
{
    unsigned char buf[10];
    int n;
    /* put RUN into keyboard buffer */
    buf[0] = 0;
    if (chameleon_writememory(buf, 1, 198) < 0) {
        LOGERR("error writing to chameleon memory.\n");
        exit(cleanup(-1));
    }
    buf[0] = 83;
    buf[1] = 217;
    buf[2] = 32;
    buf[3] = 32;
    buf[4] = 32;
    buf[5] = 32;
    buf[6] = 32;
    buf[7] = 32;
    addr &= 0xffff;
    n = sprintf((char*)&buf[2], "%d", addr);
    buf[n+2] = 13;
    if (chameleon_writememory(buf, 8, 631) < 0) {
        LOGERR("error writing to chameleon memory.\n");
        exit(cleanup(-1));
    }
    buf[0] = n+3;
    if (chameleon_writememory(buf, 1, 198) < 0) {
        LOGERR("error writing to chameleon memory.\n");
        exit(cleanup(-1));
    }
}

void execute_run(void)
{
    unsigned char buf[4];
    /* put RUN into keyboard buffer */
    buf[0] = 0;
    if (chameleon_writememory(buf, 1, 198) < 0) {
        LOGERR("error writing to chameleon memory.\n");
        exit(cleanup(-1));
    }
    buf[0] = 82;
    buf[1] = 85;
    buf[2] = 78;
    buf[3] = 13;
    if (chameleon_writememory(buf, 4, 631) < 0) {
        LOGERR("error writing to chameleon memory.\n");
        exit(cleanup(-1));
    }
    buf[0] = 4;
    if (chameleon_writememory(buf, 1, 198) < 0) {
        LOGERR("error writing to chameleon memory.\n");
        exit(cleanup(-1));
    }
}

int read_memory_block(unsigned char *buf, unsigned int len, unsigned int addr)
{
    static int isrom [16] = {
        0,0,0,0, 0,0,0,0, 0,0,1,1, 0,1,1,1
    };
    int end, slow;
    /* we can only use the DMA read when actual RAM is read */
    end = addr + len - 1;

    slow = isrom[(addr >> 12) & 0xf] || isrom[(end >> 12) & 0xf];

    if (!slow) {
        if (chameleon_readmemory(buf, len, addr) < 0) {
            LOGERR("error reading chameleon memory.\n");
            exit(cleanup(-1));
        }
    } else {
        send_memory_request(addr,len);
        len = recv(fd, buf, len,0);
    }
    return len;
}

// -----------------------------------------------------------------------------

int cmd,old_cmd;
unsigned int tokens;
char string[100];
char *token[256];
unsigned char sendbuffer[512],filebuffer[512];
int next[4096];

/* kmp search algorithm */
void init_next(char *pattern,int m){
	int i,j;
	i = 0;
	j = next[0] = -1;
	while (i < m){
		while( j > -1 && pattern[i] != pattern[j])
			j = next[j];
		++i;++j;
		(pattern[i] == pattern[j]) ? (next[i] = next[j]) : (next[i] = j);
	}
}

int kmp_search(char *pattern, char *string, unsigned size, unsigned base){
	int i = 0, j = 0;
	int m = strlen(pattern);
	int n = size;
	
	init_next (pattern,m);
	
	while(i < n){
		while (j>=0 && string[i] != pattern[j])
			j = next[j];
		++i;
		++j;
		if (j == m){
			printf("%04x\n",base + i-j);
			j = next[j];
		}
	}
	return -1;
}

/* PetASCII <--> ASCII Converting */
char pet2asc(char c){
	c &= 0x7f;
	if (c == 13 || c == 10 || c == 9) return c;
	if (c < 32) return '.';
	if (c < 65) return c;
	if (c < 91) return c + 32;
	return c - 32;
}

int string2pet(char *s){
	int i,len = strlen(s);
	for (i=0;i<len;++i)
		s[i] = pet2asc(s[i]);
	return len;
}

/* parse input string into tokens */
unsigned int tokenize(char *s){
	unsigned int i = 0;
	
	while (1){
		while (*s == ' ') ++s;		//skip leading spaces

		if (*s == '\0') return i;	//end of input string? then return number of tokens read

		if (*s == '"'){
			token[i++] = s++;	//set the token to the first char after the quot.mark
			while (*s != '"')	//search the next quot.mark or end of input string
				if (*s++ == '\0') return i;
			*s++ = '\0';		//replace the quot.mark by delimiter, advance in input string and loop
			continue;
		}

		token[i++] = s;			//else we have a token
		while (*s != ' ')		//read the token until next space or end of input string
			if (*s++ == '\0') return i;
		*s++ = '\0';
	}
}

/* send a request for memory transfer of size bytes from address base to c64 */
void send_memory_request(unsigned base,unsigned size){
	sendbuffer[0] = GET;
	sendbuffer[1] = size;
	sendbuffer[3] = (base & 0x0000ff);
	sendbuffer[4] = (base & 0x00ff00) >> 8;
	sendbuffer[5] = (base & 0xff0000) >> 16;
	send (fd,sendbuffer,6,0);
}

// -----------------------------------------------------------------------------

/* $
 * - send request for directory dislplay of current device and print to stdout
 */
void directory(char *s){
	int i,len = string2pet(s);

	sendbuffer[0] = DIR;
	sendbuffer[1] = len;
	memcpy(&sendbuffer[2],s,len);
	send(fd,sendbuffer,len+2,0);

	do{
		len = recv(fd,filebuffer,sizeof(filebuffer),0);
		if (len){
			for(i=0;i<len;++i) {
                            if (filebuffer[i] == 0) {
                                printf("\n");
                                return;
                            } else if (filebuffer[i] == 13) {
                                printf("\n");
                            } else if (filebuffer[i] == 10) {
                                printf("\r");
                            } else {
				putchar(pet2asc(filebuffer[i]));
                            }
                        }
		}
	}while(len);
}

/* read
 * - read remotefile from c64's current device, transfer the data and store in localfile
 */
void read_file(void){
	int fnlen,len;
	char *localfile,*remotefile;
	FILE *f;
	
	if (tokens < 2 || tokens > 3) puts(helptext[cmd]);
	else {
		if (token[1][0] == '"')
			remotefile = &token[1][1];
		else
			remotefile = &token[1][0];
		if (tokens == 2)
			localfile = remotefile;
		else
			localfile = token[2];
		if ((f = fopen(localfile,"wb"))==NULL){
			perror(localfile);
			return;
		}
		string2pet(remotefile);
		fnlen = strlen(remotefile);
		
		sendbuffer[0] = READ;
		sendbuffer[1] = fnlen;
		memcpy(&sendbuffer[2],remotefile,fnlen);
		send(fd,sendbuffer,fnlen+2,0);
		do{
//			len = recv(fd,filebuffer,sizeof(filebuffer),0);
                        len = recv(fd, filebuffer, len,0);
			if (len)
				fwrite (filebuffer,1,len,f);
		}while(len == 128);
		fclose(f);
	}
}

/* write
 * - write contents of localfile to remotefile on c64's current device
 */
void write_file(void){
	int fnlen,len;
	char *localfile,*remotefile;
	FILE *f;

	if (tokens < 2 || tokens > 3) puts(helptext[cmd]);
	else {
		localfile = token[1];
		if (tokens == 2)
			remotefile = localfile;
		else{
			if (token[2][0] == '"')
				remotefile = &token[2][1];
			else
				remotefile = &token[2][0];
		}
		if ((f = fopen(localfile,"rb")) == NULL){
			perror(localfile);
			return;
		}
		string2pet(remotefile);
		fnlen = strlen(remotefile);

		/* first we send request to open the write file */
		sendbuffer[0] = WRITE;
		sendbuffer[1] = fnlen;
		memcpy(&sendbuffer[2],remotefile,fnlen);
		send (fd,sendbuffer,fnlen+2,0);
	
		/* and wait for OK from c64 */
//		len = recv(fd,sendbuffer,sizeof(sendbuffer),0);
                len = recv(fd, sendbuffer, 2,0);
		if (sendbuffer[0]){
			puts("an error occured...");
			fclose(f);
			return;
		}
		
		/* now we transfer the file */
		while (!feof(f)){
			len = fread(filebuffer,1,128,f);
			sendbuffer[0] = XWRITE;
			sendbuffer[1] = len;
			memcpy(&sendbuffer[2],filebuffer,len);
			send(fd,sendbuffer,len+2,0);
			/* and wait after each packet for a (any) response from c64 */
//			len = recv(fd,sendbuffer,sizeof(sendbuffer),0);
                        len = recv(fd, sendbuffer, 2,0);
		}
		/* at last we send a OK, so the c64 can close the file */
		sendbuffer[0] = XDONE;
		send(fd,sendbuffer,1,0);
		fclose(f);
	}
}

/* put
 * - transfer a file from pc to c64 memory
 */
void put(void){
	FILE *f;
	char *filename;
	unsigned load_addr,len,bytes = 0;
//	char hi,lo;

	if (tokens < 2 || tokens > 3) puts(helptext[cmd]);
	else{
		filename = token[1];
		
		if ((f = fopen(filename,"rb")) == NULL){
			perror(filename);
			return;
		}
		/* if no load addres is given read it from file, else use the given one */
		if (tokens == 3) sscanf(token[2],"%x",&load_addr);
		else load_addr = fgetc(f) + fgetc(f) * 256;
		
		printf("sending %s to $%04x ",filename,load_addr);


		/* now send the file's data packet-wise */
		while(!feof(f)){
			len = fread(filebuffer,1,128,f);	//read len bytes data from file
			sendbuffer[0] = PUT;			// setup "put" request...
			sendbuffer[1] = len;
			sendbuffer[2] = 0;			//unused...
			sendbuffer[3] = (load_addr & 0x0000ff);
			sendbuffer[4] = (load_addr & 0x00ff00) >> 8;
			sendbuffer[5] = (load_addr & 0xff0000) >> 16;
			memcpy(&sendbuffer[6],filebuffer,len);	//copy the actual data into the packet
			send(fd,sendbuffer,len+6,0);		//and send it...
			load_addr += len;
			bytes += len;
			len = recv(fd,filebuffer,2,0);	// wait for response and loop
		}
		
		printf("- $%04x\n",load_addr);
		fclose(f);
		printf("%d bytes transferred\n",bytes);
	}
}

/* get
 * - get memory from c64 and save to a file
 */
void get(void){
	unsigned from, to;
	char *filename;
	FILE *f;
	int len;
	
	if (tokens != 4) puts(helptext[cmd]);
	else{
		sscanf(token[1],"%x",&from);
		sscanf(token[2],"%x",&to);
		filename = token[3];
		if ((f = fopen(filename,"wb")) == NULL){
			perror(filename);
			return;
		}
		fputc(from & 0xff,f);
		fputc(from >> 8  ,f);
		while (from < to){
			if ((len = to - from) >= 128)
				len = 128;
#if 0
                        send_memory_request(from,len);
//			len = recv(fd,filebuffer,sizeof(filebuffer),0);
                        len = recv(fd, filebuffer, len > sizeof(filebuffer) ? sizeof(filebuffer) : len,0);
#else
                        read_memory_block(filebuffer, len, from);
#endif
			fwrite(filebuffer,len,1,f);
			from += len;
		}
		fclose(f);
	}
}	


/* g
 * - make c64 jump to the given address
 */
void jump(void){
	unsigned jump_addr;
	
	if (tokens != 2) puts(helptext[cmd]);
	else{
		sscanf(token[1],"%x",&jump_addr);
		sendbuffer[0] = JUMP;
		sendbuffer[1] = (jump_addr & 0xff);
		sendbuffer[2] = (jump_addr & 0xff00) >> 8;
		send(fd,sendbuffer,3,0);
	}
}

/* call
 * - call subroutine at the given address, wait for RTS
 */
void call(void){
	unsigned sys_addr;
	unsigned a,x,y,s,i;
	a = x = y = s = 0;
		
	if (tokens <2 || tokens > 6) puts(helptext[cmd]);
	else{
		sscanf(token[1],"%x",&sys_addr);
		
		if (tokens > 2)
			sscanf(token[2],"%x",&a);
		if (tokens > 3)
			sscanf(token[3],"%x",&x);
		if (tokens > 4)
			sscanf(token[4],"%x",&y);
		if (tokens > 5)
			sscanf(token[5],"%x",&s);
		
		sendbuffer[0] = SYS;
		sendbuffer[1] = sys_addr & 0xff;
		sendbuffer[2] = sys_addr >> 8;
		sendbuffer[3] = a;
		sendbuffer[4] = x;
		sendbuffer[5] = y;
		sendbuffer[6] = s;
		send(fd,sendbuffer,7,0);

		recv(fd,sendbuffer,4,0);
		a = sendbuffer[0];
		x = sendbuffer[1];
		y = sendbuffer[2];
		s = sendbuffer[3];
		
		printf("returned from $%04x with\n.A .X .Y SR NV-BDIZC\n%02x %02x %02x %02x ",sys_addr,a,x,y,s);
		for (i=0;i<8;++i){
			s & 0x80 ? putchar('1') : putchar('0');
			s <<= 1;
			}
		putchar('\n');
	}
}


/* detect address mode for a given opcode; used for disassembly */
int mode (unsigned char opcode){
		
	if (opcode == 0x20) return ABS;
	if (opcode < 0x80){
		if (opcode == 0x24) return ZP;
		if (opcode == 0x6c) return IND;
		opcode &= 0x1f;
			if (opcode == 0) return IMPL;
			if (opcode == 1) return INDX;
			return table1[opcode];
	}
	else{
		if (opcode == 0x80) return IMPL;
		if (opcode == 0xa2) return IMM;
		if (opcode == 0x89) return IMPL;
		if (opcode == 0xd4) return IMPL;
		if (opcode == 0xf4) return IMPL;
		if (opcode == 0xd6) return ZPX;
		if (opcode == 0xf6) return ZPX;
		if (opcode == 0xbc) return ABSX;
		if (opcode == 0x9e) return IMPL;
		if (opcode == 0xbe) return ABSY;
		opcode &= 0x1f;
		return table2[opcode];
	}
}

/* d
 * - get data from c64, disassemble and print to stdout
 */
/* this one is a late night hack, could suerly be optimized...! but it works.*/
void disassembly (void) {
	static unsigned from = 0;
	unsigned to;
	int len,i,j = 1;
	unsigned char *p;
//	unsigned char opcode;
	
	if (tokens > 3) puts (helptext[cmd]);
	else {
		if (tokens > 1)
			sscanf(token[1],"%x",&from);
		to = from + 0x30;
		if (tokens > 2)
			sscanf(token[2],"%x",&to);

		while (from < to){
			if ((len = to - from) >= 128)
				len = 128;
#if 0
			send_memory_request(from,len);
//			len = recv(fd, filebuffer, sizeof(filebuffer),0);
                        len = recv(fd, filebuffer, len > sizeof(filebuffer) ? sizeof(filebuffer) : len,0);
#else
                        read_memory_block(filebuffer, len, from);
#endif
			p = &filebuffer[0];
			for (i = 0; i < len; i += j){
				printf("%04x   ",from + i);

				switch (mode(*p)){
					case IMM:
						printf("%02x %02x    ",*p,*(p+1));
						printf("  %s #$%02x",mnemo[*p],*(p+1));
						j = 2;
						break;
					case ABS:
						printf("%02x %02x %02x ",*p,*(p+1),*(p+2));
						printf("  %s $%04x",mnemo[*p],*(p+1) + *(p+2) * 256);
						j = 3;
						break;
					case ABSX:
						printf("%02x %02x %02x ",*p,*(p+1),*(p+2));				
						printf("  %s $%04x,x",mnemo[*p],*(p+1) + *(p+2) * 256);
						j = 3;
						break;
					case ABSY:
						printf("%02x %02x %02x ",*p,*(p+1),*(p+2));				
						printf("  %s $%04x,y",mnemo[*p],*(p+1) + *(p+2) * 256);
						j = 3;
						break;
					case ZP:						
						printf("%02x %02x    ",*p,*(p+1));
						printf("  %s $%02x",mnemo[*p],*(p+1));
						j = 2;
						break;
					case ZPX:
						printf("%02x %02x    ",*p,*(p+1));
						printf("  %s $%02x,x",mnemo[*p],*(p+2));
						j = 2;
						break;
					case ZPY:
						printf("%02x %02x    ",*p,*(p+1));
						printf("  %s $%02x,y",mnemo[*p],*(p+1));
						j = 2;
						break;
					case AKKU:
						printf("%02x       ",*p);
						printf("  %s a",mnemo[*p]);
						j = 1;
						break;
					case INDX:
						printf("%02x %02x    ",*p,*(p+1));
						printf("  %s ($%02x,x)",mnemo[*p],*(p+1));
						j = 2;
						break;
					case INDY:
						printf("%02x %02x    ",*p,*(p+1));
						printf("  %s ($%02x),y",mnemo[*p],*(p+1));
						j = 2;
						break;
					case REL:
						printf("%02x %02x    ",*p,*(p+1));
						printf("  %s $%04x",mnemo[*p],(from + i + 2) + (signed char)*(p+1));
						j = 2;
						break;
					case IND:
						printf("%02x %02x %02x ",*p,*(p+1),*(p+2));
						printf("  %s ($%04x)",mnemo[*p],*(p+1) + *(p+2) * 256);
						j = 3;
						break;
					case IMPL:
						printf("%02x       ",*p);
						printf("  %s",mnemo[*p]);
						j = 1;
				}
				putchar('\n');
				p += j;
			}
			from += len;
		}
	}
}

/* m
 * - memory peek of c64 memory
 */
void mon(void){
	static unsigned from = 0;
	unsigned to;
	int len,i,j,col;
	char c;
	int width = 8;		//bytes per line, adjust to your preferences...!
	
	if (tokens > 3)	puts(helptext[cmd]);
	else{
		/* parse parameters: m [from [to]] */
		if (tokens > 1)
			sscanf(token[1],"%x",&from);
		to = from + 0x100;
		if (tokens > 2)
			sscanf(token[2],"%x",&to);
		
		/* request memory from c64 */	
		while (from < to){
			if ((len = to - from) >= 128)
				len = 128;
#if 0
			send_memory_request(from,len);
//                        len = recv(fd, filebuffer, sizeof(filebuffer),0);
                        len = recv(fd, filebuffer, len > sizeof(filebuffer) ? sizeof(filebuffer) : len,0);
#else
                        read_memory_block(filebuffer, len, from);
#endif
			/* display it to stdout */
			for(i=0;i<len;i+=j){
				printf("\n%04x : ",from + i);
				col = 7;
				for(j=0;j<width;++j){
					if (i+j >= len) break;
					printf("%02x ",filebuffer[i+j]);
					col += 3;
				}
				while (col++ < (7 + width*3)) putchar(' ');
				printf(" | ");
				col += 3;
				for(j=0;j<width;++j){
					if (i+j >= len) break;				
					putchar(isprint(filebuffer[i+j])?filebuffer[i+j]:'.');
					++col;
				}
				while(col++ < (10+width*3+width)) putchar(' ');
				printf(" | ");
				for(j=0;j<width;++j){
					if (i+j >= len) break;
					c = pet2asc(filebuffer[i+j]);
					putchar(isprint(c)?c:'.');
				}
			}
			from += len;
		}
		puts("\n");
	}
}

/* i
 * - petascii dump
 */
void interrogate(void){
	static unsigned from;
	unsigned to;
	int len,i;
	char c;
	
	if (tokens > 3)	puts(helptext[cmd]);
	else{
		if (tokens > 1)
			sscanf(token[1],"%x",&from);
		to = from + 0x400;
		if (tokens > 2)
			sscanf(token[2],"%x",&to);
			
		while (from < to){
			if ((len = to - from) >= 128)
				len = 128;
#if 0
			send_memory_request(from,len);
//			len = recv(fd, filebuffer, sizeof(filebuffer),0);
                        len = recv(fd, filebuffer, len > sizeof(filebuffer) ? sizeof(filebuffer) : len,0);
#else
                        read_memory_block(filebuffer, len, from);
#endif
			for (i=0;i<len;i++){
				if ((i & 63)==0)
					printf("\n%04x : ",from + i);
				c = pet2asc(filebuffer[i]);
				putchar(isprint(c)?c:'.');
			}
			from += len;
		}
		printf("\n");
	}
}

/* #
 * - set/ tell device number of current device
 */
void device(char *s){
	unsigned device;
	
	device = atoi(s);

	if (device == 0){
		sendbuffer[0] = TELLDEV;
		send(fd,sendbuffer,1,0);
		recv(fd,sendbuffer,2,0);
		printf("current device is #%d\n",sendbuffer[0]);
	}
	else{
		sendbuffer[0] = SETDEV;
		sendbuffer[1] = device;
		send(fd,sendbuffer,2,0);
	}
}

/* @
 * - send disk command/ read command channel
 */
void disk_command(char *s){
	unsigned len;

	string2pet(s);
	len = strlen(s);
	sendbuffer[0] = DISKCOMMAND;
	sendbuffer[1] = len;
	memcpy(&sendbuffer[2],s,len);
	send(fd,sendbuffer,len+2,0);
	len = recv(fd,sendbuffer,sizeof(sendbuffer),0);
	puts((char*)sendbuffer);
}

/* f
 * - send request to fill memory
 */
void fill(void){
	unsigned from,to,value;

	if (tokens != 4) puts(helptext[cmd]);
	else{
		sscanf(token[1],"%x",&from);
		sscanf(token[2],"%x",&to);
		sscanf(token[3],"%x",&value);
		sendbuffer[0] = FILL;
		sendbuffer[1] = value;
		sendbuffer[2] = (from & 0xff);
		sendbuffer[3] = (from >> 8);
		sendbuffer[4] = (to & 0xff);
		sendbuffer[5] = (to >> 8);
		send (fd,sendbuffer,6,0);
	}
}

/* h
 *  - get memory and hunt for occurances of search pattern
 */
void hunt(void){
	unsigned i,len;
	char pattern[128],*mem;
	unsigned from,to,size;
		
	if (tokens < 4) puts(helptext[cmd]);
	else {
		sscanf(token[1],"%x",&from);
		sscanf(token[2],"%x",&to);

		if (from >= to) return;
		size = to - from;
		if ((mem = (char*)malloc(size))==NULL){
			perror(NULL);
			return;
		}
		
		if (token[3][0] == '"'){
			strncpy(pattern,&token[3][1],sizeof(pattern));
			string2pet(pattern);
		}
		else{
			len = 0;
			for (i=3;i<tokens;++i) {
                            int tmp;
    			    sscanf(token[i],"%x",&tmp);
                            pattern[len++] = tmp;
                        }
			pattern[len++] = 0;
		}

		for (i = 0; i < size ;i += len){
			if ((len = size - i) >= 128)
				len = 128;
#if 0
			send_memory_request(from + i,len);
//			len = recv(fd,filebuffer,sizeof(filebuffer),0);
                        len = recv(fd, filebuffer, len > sizeof(filebuffer) ? sizeof(filebuffer) : len,0);
#else
                        read_memory_block(filebuffer, len, from + i);
#endif
			memcpy(mem + i,filebuffer,len);
		}
		kmp_search(pattern,mem,size,from);
		free(mem);		
	}
}

/* c
 * - get two memory areas and compare them, print differences to stdout
 */
void compare(void){
	unsigned from,to,with,size,i,len,rlen,missmatch;
	char *buf1,*buf2,*b1,*b2;

	if (tokens != 4) puts(helptext[cmd]);
	else{
		sscanf(token[1],"%x",&from);
		sscanf(token[2],"%x",&to);
		sscanf(token[3],"%x",&with);
		if (from >= to) return;
		size = to - from;
		if ((buf1 = (char*)malloc(size)) == NULL){
			perror(NULL);
			return;
		}
		if ((buf2 = (char*)malloc(size)) == NULL){
			perror(NULL);
			return;
		}
		for (i = 0; i < size; i += len){
			if ((len = size - i) >= 128)
				len = 128;
#if 0
			send_memory_request(from + i,len);
//			rlen = recv(fd,filebuffer,sizeof(filebuffer),0);
                        rlen = recv(fd, filebuffer, len > sizeof(filebuffer) ? sizeof(filebuffer) : len,0);
#else
                        rlen = read_memory_block(filebuffer, len, from + i);
#endif
			memcpy(buf1 + i,filebuffer,rlen);
#if 0
			send_memory_request(with + i,len);
//			rlen = recv(fd,filebuffer,sizeof(filebuffer),0);
                        rlen = recv(fd, filebuffer, len > sizeof(filebuffer) ? sizeof(filebuffer) : len,0);
#else
                        rlen = read_memory_block(filebuffer, len, with + i);
#endif
			memcpy(buf2 + i,filebuffer,rlen);
		}
		missmatch = 0;
		b1 = buf1; b2 = buf2;
		for (i=0;i<size;++i){
			if (*b1 != *b2){
				printf("%04x %02x - %04x %02x\n",from+i,*b1,with+i,*b2);
				++missmatch;
			}
			++b1; ++b2;
		}
		if (missmatch) printf("%d ",missmatch);
		else printf("no ");
		puts("mismatches found.");
		free(buf1); free(buf2);
	}
}

/* poke
 * - send memory write request
 */
void poke(void){
	unsigned address,len,i;
//      unsigned char values[128],*p;
        char values[128];

	if (tokens < 3)	puts(helptext[cmd]);
	else{
		//string...?!
		if (token[2][0] == '"'){
			strncpy(values,&token[2][1],sizeof(values));
			string2pet(values);
			len = strlen(values);
		}
		//values
		else{
			len = 0;
			for (i=2;i<tokens;++i) {
                            int tmp;
                            sscanf(token[i],"%x",&tmp);
                            values[len]= tmp;
                            len++;
                        }
		}

		sscanf(token[1],"%x",&address);
		
		sendbuffer[0] = POKE;
		sendbuffer[1] = len;
		sendbuffer[2] = address & 0xff;
		sendbuffer[3] = address >> 8;
		memcpy(&sendbuffer[4],values,len);
		send(fd,sendbuffer,len+4,0);
	}
}

/* help
 * ?
 * - help that poor user...!
 */
void help(void){
	int i;
	
	if (tokens == 1){
		for (i=0;i < COMMANDS;++i)
			puts(helptext[i]);
	}
	else{
		for (i = 0;i < COMMANDS; ++i)
			if (strcmp(token[1],command_table[i])==0) break;
		if (i == COMMANDS) printf("no help available for %s\n",token[1]);
		else puts(helptext[i]);
	}
}

/* ram
 * rom
 * reu
 * - set ram access mode
 */
void setram(void){
	int access;

	if (tokens > 2) puts (helptext[cmd]);

	if (strcmp(token[0],"rom") == 0)
		access = 0x37;
	else if (strcmp(token[0],"reu") == 0)
		access = 0xff;
	else{
		if (tokens == 1)
			access = 0x34;
		else
			sscanf(token[1],"%x",&access);
	}
	sendbuffer[0] = SETRAM;
	sendbuffer[1] = access;
	send(fd,sendbuffer,2,0);
}

/* status
 * - tell server status
 */
void status(void){
//	int len;
	sendbuffer[0] = STATUS;	
	send(fd,sendbuffer,1,0);
	/*len = */ recv(fd,sendbuffer,2,0);
	printf("server reports:\nmemory access is $%02x",sendbuffer[0]);
	if (sendbuffer[0] & 0x80) printf("(REU)");
	printf("\ncurrent device is %d\n",sendbuffer[1]);
}

/* cmd
 * - call system command
 */

void command(char *cmd) {
    int ret;
    if ((ret = system(cmd)) < 0) {
        printf("err: %s returned %d\n", cmd, ret);
    }
}

// -----------------------------------------------------------------------------

int cleanup(int n) {
    chameleon_close();
    return n;
}

#define C64_RAM_SIZE 0x10000
unsigned char buffer[C64_RAM_SIZE];

int main (int argc, char ** argv) {
	unsigned int n,direct_flag = 0,len,addr;
        int i;
	char *cline = NULL;
        char *name;

	/* set default values */
	old_cmd = 0;

    chameleon_setlogfunc(logfunc);

    if (chameleon_init() < 0) {
        LOGERR("initialization failed.\n");
        exit(-1);
    }


	/* parse command line for options (-ip, -p)*/
	for (i=1;i<argc;++i){

		if (strcmp (argv[i] ,"--help") == 0 || strcmp (argv[i],"-h") == 0 || strcmp(argv[i],"-?") == 0){
 			puts(usage);
			return 0;
		}
		else if (strcmp (argv[i],"--version") == 0 || strcmp (argv[i],"-v") == 0){
			printf("chmon version %s\n",version);
			return 0;
		}
		else if (strcmp (argv[i] ,"--command") == 0 || strcmp (argv[i],"-c") == 0){
			while (++i < argc){
				strncat(string, argv[i], sizeof(string) - 1);
				if (i != argc-1)
					strncat(string," ",sizeof(string) - 1);
			}
			direct_flag = 1;
			break;
                } else if (!strcmp (argv[i] ,"--send") || !strcmp("-s", argv[i])) {
                    FILE *f;
                    /* write .prg file to memory and SYS to its start addr */
                    i++;
                    name = argv[i];
                    if ((f = fopen(name, "rb")) == NULL) {
                        LOGERR("error opening: '%s'\n", name);
                        exit(cleanup(-1));
                    }
                    addr = fgetc(f);
                    addr += ((int)fgetc(f) << 8);

                    len = fread(buffer, 1, C64_RAM_SIZE - addr, f);
                    fclose(f);
                    printf("sending '%s' ($%04x bytes to $%04x.)...\n", name, len, addr);
                    if (chameleon_writememory(buffer, len, addr) < 0) {
                        LOGERR("error writing to chameleon memory.\n");
                        exit(cleanup(-1));
                    }
                    execute_sys(addr);
                } else {
			printf("invalid option: %s\ntry %s -h\n",argv[i],argv[0]);
			exit(1);
		}
	}

	//command interpreter loop
	do{

		if (!direct_flag){
#ifndef LINUX
			printf("\n%s",prompt);
			fgets(string,sizeof(string),stdin);
			for (i=0;i< strlen(string);++i){
				if (string[i] == '\n'){
					string[i] = '\0';
					break;
				}
			}
#else
			if (cline){
				free(cline);
				cline = NULL;
			}
			cline = readline(prompt);
			if (cline && *cline)
				add_history(cline);

			strncpy(string,cline,sizeof(string));
#endif
		}
		/* if not "m","i" or "d", ignore "empty" input */
		if (strlen(string) == 0){
			if (old_cmd > 2)
				continue;
			else cmd = old_cmd;
		}
		else {
			//treat special commands: $,#,@
			if (string[0] == '@'){
				disk_command(&string[1]);
				continue;
			}
			else if (string[0] == '$'){
				directory(string);
				continue;
			}
			else if (string[0] == '#'){
				device(&string[1]);
				continue;
			}

			// parse input into tokens
			tokens = tokenize(string);
			if (tokens == 0) continue;

			//let's look, if the first token is a known command...
			for(cmd = 0; cmd < COMMANDS; ++cmd){
				if (strcmp(token[0],command_table[cmd]) == 0) 
					break;
			}
			//not found...?! what a pity.
			if (cmd == COMMANDS){
				printf("what exactly do you mean by: '%s'\ntype 'help' to get some help\n",token[0]);
				continue;
			}
			old_cmd = cmd;
		}
		
		if (cmd == 22 || cmd == 23) break;	//"quit" or "x"
		
		// call the according function, then loop
		switch (cmd){
			case 0:	 mon(); 	break;
			case 1:  interrogate(); break;
			case 2:  disassembly(); break;
			case 3:  jump(); 	break;
			case 4:  hunt(); 	break;
			case 5:  fill(); 	break;
			case 6:  compare(); 	break;
			case 7:  put(); 	break;
			case 8:  get(); 	break;
			case 9:  poke(); 	break;
                        //10 $ directory
                        //11 @ disk_command
                        //12 # device
			case 13: read_file();	break;
			case 14: write_file();	break;
			case 15: sendbuffer[0] = RUN;	 send (fd,sendbuffer,1,0);	 break;
			case 16: sendbuffer[0] = BASIC;	 send (fd,sendbuffer,1,0);	 break;
			case 17: 
			case 18: 
			case 19: setram();	break;
			case 20: 
			case 21: help();	break;
                        //22 quit
                        //23 x
			case 24: status(); 	break;
			case 25: sendbuffer[0] = INIT;	 send (fd,sendbuffer,1,0);	 break;
			case 26: call(); 	break;
			case 27: string[0] = '\0';
				 for (n=1;n<tokens;++n){
					 strcat(string,token[n]);
					 strcat(string," ");
				 }
				 command(string);
		}
	}while(direct_flag == 0);
    return 0;
}

