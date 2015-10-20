
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

#define CHCODENET_VERSION "0.1"

#define LOGERR(...)       logfunc (LOGLVL_ERR, __VA_ARGS__ )
#define LOGVER(...)       logfunc (LOGLVL_VER, __VA_ARGS__ )
#define LOGMSG(...)       logfunc (LOGLVL_MSG, __VA_ARGS__ )
#define DBG(...)          logfunc (LOGLVL_DBG, __VA_ARGS__ )

int verbose = 0;
#define C64_RAM_SIZE    0x10000
unsigned char *buffer = NULL;

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

void usage (void)
{
    printf(
    "chcodenet %s (compiled on %s)\n"
    "usage: chcodenet <options>\n"
    "\n"
    "-h --help          this help\n"
    "--verbose          enable verbose messages\n"
    "--debug            enable debug messages\n"
    "\n"
    "-w Filename        Sends a PRG file to the C64.\n"
    "-wa Filename Addr  Sends a PRG file to the C64 to a specified address.\n"
    "-wb Filename Addr  Sends a binary file to the C64 to a specified address.\n"
    "-x Filename        Sends a PRG file to the C64 and executes it\n"
    "-f Start End Fill  Fills a block of C64 memory.\n"
    "-e Addr            Jumps to an address in memory.\n"
    "-r                 Executes a program via \"RUN\"."
    "\n"
    "-n, -p, -T, -R     ignored for compatibility\n"
    , CHCODENET_VERSION
    , __DATE__
    );
}

void execute_sys(int addr)
{
    unsigned char buf[10];
    int n;
    /* put RUN into keyboard buffer */
    buf[0] = 0;
    if (chameleon_writememory(buf, 1, 198) < 0) {
        LOGERR("error writing to chameleon memory.\n");
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
    }
    buf[0] = n+3;
    if (chameleon_writememory(buf, 1, 198) < 0) {
        LOGERR("error writing to chameleon memory.\n");
    }
}

void execute_run(void)
{
    unsigned char buf[4];
    /* put RUN into keyboard buffer */
    buf[0] = 0;
    if (chameleon_writememory(buf, 1, 198) < 0) {
        LOGERR("error writing to chameleon memory.\n");
    }
    buf[0] = 82;
    buf[1] = 85;
    buf[2] = 78;
    buf[3] = 13;
    if (chameleon_writememory(buf, 4, 631) < 0) {
        LOGERR("error writing to chameleon memory.\n");
    }
    buf[0] = 4;
    if (chameleon_writememory(buf, 1, 198) < 0) {
        LOGERR("error writing to chameleon memory.\n");
    }
}

int shutdown(int n) {
    chameleon_close();
    if (buffer) {
        free(buffer);
        buffer = NULL;
    }
    return n;
}

int main(int argc, char *argv[])
{
    char *name;
    int addr = 0, len = 0, end = 0, val = 0, i;
    FILE *f;

    if (argc < 2) {
        usage();
        exit (-1);
    }

    /* first check the options that should work before any other stuff is used */
    for (i = 1; i < argc; i++) {
        if (!strcmp("--verbose", argv[i])) {
            verbose = 1;
        } else if (!strcmp("--debug", argv[i])) {
            verbose = 2;
        } else if (!strcmp("-h", argv[i]) || !strcmp("--help", argv[i]))  {
            usage();
            exit (-1);
        } else {
            break;
        }
    }

    if (i == argc) {
        usage();
        exit (-1);
    }

    if (chameleon_init() < 0) {
        LOGERR("initialization failed.\n");
        exit(-1);
    }
    if((buffer = (unsigned char*)malloc(C64_RAM_SIZE)) == NULL) {
        LOGERR("could not allocate memory.\n");
        exit(shutdown(-1));
    };
 
#ifdef LINUX
    /* make sure that if the binary is setuid root, the created files will be
       owned by the user running the binary (and not root) */
    {
        int fsuid = setfsuid(getuid());
        int fsgid = setfsgid(getgid());
        LOGVER("fsuid: %d fsgid: %d\n", fsuid, fsgid);
    }
#endif

    chameleon_setlogfunc(logfunc);

    /* check the rest of the options */
    for (; i < argc; i++) {
        if (!strcmp("-x", argv[i])) {
            /* write .prg file to memory + RUN */
            i++;
            name = argv[i];
            if ((f = fopen(name, "rb")) == NULL) {
                LOGERR("error opening: '%s'\n", name);
                exit(shutdown(-1));
            }
            addr = fgetc(f);
            addr += ((int)fgetc(f) << 8);

            len = fread(buffer, 1, C64_RAM_SIZE - addr, f);
            fclose(f);
            printf("sending '%s' ($%04x bytes to $%04x.)...\n", name, len, addr);
            if (chameleon_writememory(buffer, len, addr) < 0) {
                LOGERR("error writing to chameleon memory.\n");
                exit(shutdown(-1));
            }
            execute_run();
        } else if (!strcmp("-r", argv[i])) {
            /* execute prg via RUN */
            execute_run();
        } else if (!strcmp("-e", argv[i])) {
            /* execute prg via SYS */
            i++;
            addr = strtoul(argv[i], NULL, 0);
            execute_sys(addr);
        } else if (!strcmp("-w", argv[i])) {
            /* write .prg file to memory */
            i++;
            name = argv[i];
            if ((f = fopen(name, "rb")) == NULL) {
                LOGERR("error opening: '%s'\n", name);
                exit(shutdown(-1));
            }
            addr = fgetc(f);
            addr += ((int)fgetc(f) << 8);

            len = fread(buffer, 1, C64_RAM_SIZE - addr, f);
            fclose(f);
            printf("sending '%s' ($%04x bytes to $%04x.)...\n", name, len, addr);
            if (chameleon_writememory(buffer, len, addr) < 0) {
                LOGERR("error writing to chameleon memory.\n");
                exit(shutdown(-1));
            }
        } else if (!strcmp("-wa", argv[i])) {
            /* write .prg file to specified address */
            i++;
            name = argv[i];
            if ((f = fopen(name, "rb")) == NULL) {
                LOGERR("error opening: '%s'\n", name);
                exit(shutdown(-1));
            }
            i++;
            addr = strtoul(argv[i], NULL, 0);
            fgetc(f);fgetc(f);

            len = fread(buffer, 1, C64_RAM_SIZE - addr, f);
            fclose(f);
            printf("sending '%s' ($%04x bytes to $%04x.)...\n", name, len, addr);
            if (chameleon_writememory(buffer, len, addr) < 0) {
                LOGERR("error writing to chameleon memory.\n");
                exit(shutdown(-1));
            }
        } else if (!strcmp("-wb", argv[i])) {
            /* write binary file to specified address */
            i++;
            name = argv[i];
            if ((f = fopen(name, "rb")) == NULL) {
                LOGERR("error opening: '%s'\n", name);
                exit(shutdown(-1));
            }
            i++;
            addr = strtoul(argv[i], NULL, 0);

            len = fread(buffer, 1, C64_RAM_SIZE - addr, f);
            fclose(f);
            printf("sending '%s' ($%04x bytes to $%04x.)...\n", name, len, addr);
            if (chameleon_writememory(buffer, len, addr) < 0) {
                LOGERR("error writing to chameleon memory.\n");
                exit(shutdown(-1));
            }
        } else if (!strcmp("-f", argv[i])) {
            /* fill a block of memory */
            i++;
            addr = strtoul(argv[i], NULL, 0);
            i++;
            end = strtoul(argv[i], NULL, 0);
            i++;
            val = strtoul(argv[i], NULL, 0);
            len = (end - addr) + 1;

            memset(buffer, val, C64_RAM_SIZE);

            printf("filling $%04x to $%04x ($%04x bytes)...\n", addr, end, len);
            if (chameleon_writememory(buffer, len, addr) < 0) {
                LOGERR("error writing to chameleon memory.\n");
                exit(shutdown(-1));
            }
        } else if (!strcmp("-n", argv[i])) {
            /* ignored */
            i++;
        } else if (!strcmp("-p", argv[i])) {
            /* ignored */
            i++;
        } else if (!strcmp("-T", argv[i])) {
            /* ignored */
            i++;
        } else if (!strcmp("-R", argv[i])) {
            /* ignored */
            i++;
        } else {
            usage();
            exit(shutdown(-1));
        }
    }

    return shutdown(0);
}
