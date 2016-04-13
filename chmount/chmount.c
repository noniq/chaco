

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
#include "chameleon.h"

#ifndef DEBUG
/* #define DEBUG */
#endif

#define CHMOUNT_VERSION "0.1"

#define LOGERR(...)       logfunc (LOGLVL_ERR, __VA_ARGS__ )
#define LOGVER(...)       logfunc (LOGLVL_VER, __VA_ARGS__ )
#define LOGMSG(...)       logfunc (LOGLVL_MSG, __VA_ARGS__ )
#define DBG(...)          logfunc (LOGLVL_DBG, __VA_ARGS__ )

int cleanup(int n);

int verbose = 0;
#define BUFFER_SIZE    (1*1024*1024)
unsigned char *buffer = NULL;

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

int cleanup(int n) {
    chameleon_close();
    if (buffer) {
        free(buffer);
        buffer = NULL;
    }
    return n;
}

void usage (void)
{
    printf(
    "chmount %s (compiled on %s)\n"
    "usage: chmount <options>\n"
    "\n"
    "-h --help          this help\n"
    "--verbose          enable verbose messages\n"
    "--debug            enable debug messages\n"
    "\n"
    "-d64 Filename      send/mount d64 (drive 1, slot 1)\n"
    "-g64 Filename      send/mount g64 (drive 1, slot 1)\n"
    "-crt Filename      send/mount crt (slot 4)\n"
    , CHMOUNT_VERSION
    , __DATE__
    );
}

/******************************************************************************/

#include "gcr.h"

unsigned char gcrbuffer[GCRMAXHALFTRACKS][GCRMAXTRACKLEN];
unsigned char gcrspdbuffer[GCRMAXHALFTRACKS][4];
unsigned int gcrlenbuffer[GCRMAXHALFTRACKS];

#include "d64togcr.c"
extern int loadg64(char *name);
//extern int loadd64(char *name);

extern unsigned char romimage[1024*1024];
extern int loadcrt(char *name);

int main(int argc, char *argv[])
{
    char *inname;
    int tracks = -1;
    unsigned int addr;
    int i;
    int mode = -1;


    if (argc < 3) {
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

    for (; i < argc; i++) {
        if (!strcmp(argv[i], "-d64")) {
            i++;
            inname = argv[i];
            tracks = loadd64(inname);
            printf("%d tracks loaded from %s, using disk id %02x %02x\n", tracks, inname, d64buffer[0x165a2], d64buffer[0x165a3]);
            encoded64image(tracks, d64buffer[0x165a2], d64buffer[0x165a3]);
            mode = 1;
        } else if (!strcmp(argv[i], "-g64")) {
            i++;
            inname = argv[i];
            tracks = loadg64(inname);
            printf("%d tracks loadeded from %s\n", tracks, inname);
            mode = 1;
        } else if (!strcmp(argv[i], "-crt")) {
            i++;
            inname = argv[i];
            tracks = loadcrt(inname);
            if (tracks > 0) {
                printf("%d bytes loadeded from %s\n", tracks, inname);
                mode = 2;
            }
        } else {
            usage();
            exit (-1);
        }
    }
    if (tracks > 0) {
        chameleon_setprogressfunc(1000, (void (*)(unsigned int, unsigned int))progress);
        chameleon_setlogfunc(logfunc);

        if (chameleon_init() < 0) {
            LOGERR("initialization failed.\n");
            exit(-1);
        }
        progressmsg = "Writing";

        if (mode == 1) {
            addr = RAMBASE_DISK0IMG0;
            if (chameleon_writememory(&gcrbuffer[2][0], 0x2000*84, addr) < 0) {
                LOGERR("error writing to chameleon memory.\n");
                exit(cleanup(-1));
            }
        } else if (mode == 2) {
            addr = RAMBASE_USERCRT1;
#if 0
            {
            FILE *f;
            f = fopen("bla","wb");
            fwrite(&romimage[0], tracks,1,f);
            fclose(f);
            exit(1);
            }
#endif
            if (chameleon_writememory(&romimage[0], tracks, addr) < 0) {
                LOGERR("error writing to chameleon memory.\n");
                exit(cleanup(-1));
            }
        }
    }

    return 0;
}
