

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
    "-d64 Filename      send/mount d64\n"
    , CHMOUNT_VERSION
    , __DATE__
    );
}

/******************************************************************************/
#include "d64togcr.c"

int main(int argc, char *argv[])
{
    char *inname;
    int tracks;
//    unsigned char *buffer;
    unsigned int addr;
//    int t;
    int i;

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
        } else {
            usage();
            exit (-1);
        }
    }

    chameleon_setprogressfunc(1000, (void (*)(unsigned int, unsigned int))progress);
    chameleon_setlogfunc(logfunc);

    if (chameleon_init() < 0) {
        LOGERR("initialization failed.\n");
        exit(-1);
    }

    addr = RAMBASE_DISK0IMG0;
    progressmsg = "Writing";
#if 0
    printf("sending ($%04x bytes to $%04x.)...\n", 0x2000*84, addr);
    for (t = 2; t <= (42 * 2); t++) {
        buffer = &gcrbuffer[t][0];
        if (chameleon_writememory(buffer, 0x2000, addr) < 0) {
            LOGERR("error writing to chameleon memory.\n");
            exit(cleanup(-1));
        }
        addr += 0x2000;
    }
#endif
    if (chameleon_writememory(&gcrbuffer[2][0], 0x2000*84, addr) < 0) {
        LOGERR("error writing to chameleon memory.\n");
        exit(cleanup(-1));
    }

    return 0;
}
