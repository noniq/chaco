
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

#define TARGET_C64 1
#define TARGET_VIC20 2
#define TARGET_VIC20_8K 3

int cleanup(int n);

int verbose = 0;
#define C64_RAM_SIZE    0x10000
unsigned char *buffer = NULL;

int target = TARGET_C64;

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
    "--vic20            set target to chameleon-vic20 (unexpanded)\n"
    "--vic20-8k         set target to chameleon-vic20 (+8K)\n"
    "\n"
    "--reset            trigger target reset\n"
    "--resetwait        trigger target reset and wait until after reset\n"
    "\n"
    "-n, -p, -T, -R     ignored for compatibility\n"
    , CHCODENET_VERSION
    , __DATE__
    );
}

void setup_pointers(int addr, int len)
{
    unsigned char buf[10];
    buf[0] = (addr + len) & 0xff;
    buf[1] = ((addr + len)>>8) & 0xff;
    // Pointer: Start of BASIC Variables
    if (chameleon_writememory(buf, 2, 0x2d) < 0) {
        LOGERR("error writing to chameleon memory.\n");
        exit(cleanup(EXIT_FAILURE));
    }
    // Pointer: Start of BASIC Arrays
    if (chameleon_writememory(buf, 2, 0x2f) < 0) {
        LOGERR("error writing to chameleon memory.\n");
        exit(cleanup(EXIT_FAILURE));
    }
    // Pointer: End of BASIC Arrays + 1
    if (chameleon_writememory(buf, 2, 0x31) < 0) {
        LOGERR("error writing to chameleon memory.\n");
        exit(cleanup(EXIT_FAILURE));
    }
    // Tape End Address/End of Program
    if (chameleon_writememory(buf, 2, 0xae) < 0) {
        LOGERR("error writing to chameleon memory.\n");
        exit(cleanup(EXIT_FAILURE));
    }
}

void execute_sys(int addr)
{
    unsigned char buf[10];
    int n;
    /* put RUN into keyboard buffer */
    buf[0] = 0;
    if (chameleon_writememory(buf, 1, 198) < 0) {
        LOGERR("error writing to chameleon memory.\n");
        exit(cleanup(EXIT_FAILURE));
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
        exit(cleanup(EXIT_FAILURE));
    }
    buf[0] = n+3;
    if (chameleon_writememory(buf, 1, 198) < 0) {
        LOGERR("error writing to chameleon memory.\n");
        exit(cleanup(EXIT_FAILURE));
    }
}

void execute_run(void)
{
    unsigned char buf[4];
    /* put RUN into keyboard buffer */
    buf[0] = 0;
    if (chameleon_writememory(buf, 1, 198) < 0) {
        LOGERR("error writing to chameleon memory.\n");
        exit(cleanup(EXIT_FAILURE));
    }
    buf[0] = 82;
    buf[1] = 85;
    buf[2] = 78;
    buf[3] = 13;
    if (chameleon_writememory(buf, 4, 631) < 0) {
        LOGERR("error writing to chameleon memory.\n");
        exit(cleanup(EXIT_FAILURE));
    }
    buf[0] = 4;
    if (chameleon_writememory(buf, 1, 198) < 0) {
        LOGERR("error writing to chameleon memory.\n");
        exit(cleanup(EXIT_FAILURE));
    }
}

void execute_resetwait(void)
{
    unsigned char buf[6];
    unsigned int readyloc = 1024 + (5 * 40);
    /* overwrite the memory where we expect "ready" after reset */
    buf[0] = 32;
    buf[1] = 32;
    buf[2] = 32;
    buf[3] = 32;
    buf[4] = 32;
    buf[5] = 32;
    if (chameleon_writememory(buf, 6, readyloc) < 0) {
        LOGERR("error writing to chameleon memory.\n");
        exit(cleanup(EXIT_FAILURE));
    }
    if (chameleon_writememory(buf, 1, 0x80000000) < 0) {
        LOGERR("error writing to chameleon memory.\n");
        exit(cleanup(EXIT_FAILURE));
    }
    printf("waiting for reset to finish..."); fflush(stdout);
    while ((buf[0] != 18) && (buf[1] != 5) && (buf[2] != 1) &&
           (buf[3] != 4) && (buf[4] != 25) && (buf[5] != 46)){
        if (chameleon_readmemory(buf, 6, readyloc) < 0) {
            LOGERR("error reading chameleon memory.\n");
            exit(cleanup(EXIT_FAILURE));
        }
    }
    printf("done.\n");
}

int cleanup(int n) {
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
        exit (EXIT_FAILURE);
    }

    /* first check the options that should work before any other stuff is used */
    for (i = 1; i < argc; i++) {
        if (!strcmp("--verbose", argv[i])) {
            verbose = 1;
        } else if (!strcmp("--debug", argv[i])) {
            verbose = 2;
        } else if (!strcmp("-h", argv[i]) || !strcmp("--help", argv[i]))  {
            usage();
            exit (EXIT_FAILURE);
        } else {
            break;
        }
    }

    if (i == argc) {
        usage();
        exit (EXIT_FAILURE);
    }

    chameleon_setlogfunc(logfunc);

    if (chameleon_init() < 0) {
        LOGERR("initialization failed.\n");
        exit(EXIT_FAILURE);
    }
    if((buffer = (unsigned char*)malloc(C64_RAM_SIZE)) == NULL) {
        LOGERR("could not allocate memory.\n");
        exit(cleanup(EXIT_FAILURE));
    };

    /* check the rest of the options */
    for (; i < argc; i++) {
        if (!strcmp("-x", argv[i])) {
            /* write .prg file to memory + RUN */
            i++;
            name = argv[i];
            if ((f = fopen(name, "rb")) == NULL) {
                LOGERR("error opening: '%s'\n", name);
                exit(cleanup(EXIT_FAILURE));
            }
            addr = fgetc(f);
            addr += ((int)fgetc(f) << 8);

            len = fread(buffer, 1, C64_RAM_SIZE - addr, f);
            fclose(f);
            printf("sending '%s' ($%04x bytes to $%04x.)...\n", name, len, addr);
            if (chameleon_writememory(buffer, len, addr) < 0) {
                LOGERR("error writing to chameleon memory.\n");
                exit(cleanup(EXIT_FAILURE));
            }
            setup_pointers(addr, len);
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
                exit(cleanup(EXIT_FAILURE));
            }
            addr = fgetc(f);
            addr += ((int)fgetc(f) << 8);

            len = fread(buffer, 1, C64_RAM_SIZE - addr, f);
            fclose(f);
            printf("sending '%s' ($%04x bytes to $%04x.)...\n", name, len, addr);
            if (chameleon_writememory(buffer, len, addr) < 0) {
                LOGERR("error writing to chameleon memory.\n");
                exit(cleanup(EXIT_FAILURE));
            }
        } else if (!strcmp("-wa", argv[i])) {
            /* write .prg file to specified address */
            i++;
            name = argv[i];
            if ((f = fopen(name, "rb")) == NULL) {
                LOGERR("error opening: '%s'\n", name);
                exit(cleanup(EXIT_FAILURE));
            }
            i++;
            addr = strtoul(argv[i], NULL, 0);
            fgetc(f);fgetc(f);

            len = fread(buffer, 1, C64_RAM_SIZE - addr, f);
            fclose(f);
            printf("sending '%s' ($%04x bytes to $%04x.)...\n", name, len, addr);
            if (chameleon_writememory(buffer, len, addr) < 0) {
                LOGERR("error writing to chameleon memory.\n");
                exit(cleanup(EXIT_FAILURE));
            }
        } else if (!strcmp("-wb", argv[i])) {
            /* write binary file to specified address */
            i++;
            name = argv[i];
            if ((f = fopen(name, "rb")) == NULL) {
                LOGERR("error opening: '%s'\n", name);
                exit(cleanup(EXIT_FAILURE));
            }
            i++;
            addr = strtoul(argv[i], NULL, 0);

            len = fread(buffer, 1, C64_RAM_SIZE - addr, f);
            fclose(f);
            printf("sending '%s' ($%04x bytes to $%04x.)...\n", name, len, addr);
            if (chameleon_writememory(buffer, len, addr) < 0) {
                LOGERR("error writing to chameleon memory.\n");
                exit(cleanup(EXIT_FAILURE));
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
                exit(cleanup(EXIT_FAILURE));
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
        } else if (!strcmp("--vic20", argv[i])) {
            target = TARGET_VIC20;
        } else if (!strcmp("--vic20-8k", argv[i])) {
            target = TARGET_VIC20_8K;
        } else if (!strcmp("--reset", argv[i])) {
            if (chameleon_writememory(buffer, 1, 0x80000000) < 0) {
                LOGERR("error writing to chameleon memory.\n");
                exit(cleanup(EXIT_FAILURE));
            }
        } else if (!strcmp("--resetwait", argv[i])) {
            if (target != TARGET_C64) {
                LOGERR("--resetwait not implemented for vic20 yet\n");
                exit(cleanup(EXIT_FAILURE));
            }
            execute_resetwait();
        } else {
            usage();
            exit(cleanup(EXIT_FAILURE));
        }
    }

    return cleanup(EXIT_SUCCESS);
}
