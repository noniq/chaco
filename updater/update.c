
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <dirent.h>

#ifdef POSIX
#include <unistd.h>
#include <termios.h>
#endif
#ifdef LINUX
#include <sys/fsuid.h>
#include <sys/types.h>
#endif

#include "chacolib.h"
#include "chameleon.h"

#ifndef DEBUG
/* #define DEBUG */
#endif

#define LOGERR(...)       logfunc (LOGLVL_ERR, __VA_ARGS__ )
#define LOGVER(...)       logfunc (LOGLVL_VER, __VA_ARGS__ )
#define LOGMSG(...)       logfunc (LOGLVL_MSG, __VA_ARGS__ )
#define DBG(...)          logfunc (LOGLVL_DBG, __VA_ARGS__ )

#ifdef POSIX
static int getkey(void)
{
int ch;
    struct termios oldt;
    struct termios newt;
    tcgetattr(STDIN_FILENO, &oldt); /*store old settings */
    newt = oldt; /* copy old settings to new settings */
    newt.c_lflag &= ~(ICANON | ECHO); /* make one change to old settings in new settings */
    tcsetattr(STDIN_FILENO, TCSANOW, &newt); /*apply the new settings immediatly */
    ch = getchar(); /* standard getchar call */
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt); /*reapply the old settings */
    return ch; /*return received char */
}
#endif

#ifdef WIN32
#include <conio.h>
static int getkey(void)
{
int ch;
    ch = getch();
    return ch; /*return received char */
}
#endif

int verbose = 0;

static void usage (void)
{
    printf(
    "update (beta %s) - Chameleon updater.\n"
    "usage: update <options>\n"

    "-h --help                                  this help\n"

    "--verbose                                  enable verbose messages\n"
    "--debug                                    enable debug messages\n"
    , __DATE__
    );
}

static void welcome (void)
{
    printf(
        "\nThis will update your Chameleon.\n"
        "Before proceeding, make sure to remove the sd card from the Chameleon.\n"
        "Press u to update, or any other key to abort.\n"
    );

    if (getkey() != 'u') {
        chameleon_startcore(0);
        exit (EXIT_SUCCESS);
    }
}

static void bye (int n)
{
    printf("\npress any key.\n");
    getkey();
    exit (n);
}

static int match (char *str, int version)
{
    int n;
    n = strlen(str);
    
    if (version == 0) {
        if (n > 5) {
            n-=4;
    //        printf(">%s<\n",&str[n]);
            if (!strcmp(&str[n], ".rbf")) {
                return 1;
            }
        }
    }

    if (version == 1) {
        if (n > 7) {
            n-=6;
    //        printf(">%s<\n",&str[n]);
            if (!strcmp(&str[n], "v2.rbf")) {
                return 1;
            }
        }
    }

    return 0;
}

static char *findcore(int version)
{
    DIR             *dip;
    struct dirent   *dit;
    static char fullname[0x100];

    if ((dip = opendir("./UPDATE")) == NULL)
    {
        fprintf(stderr, "error opening: '%s'\n", "./UPDATE");
        bye(-1);
    }

    while ((dit = readdir(dip)) != NULL)
    {
//                printf("\n%s", dit->d_name);
            if (match (dit->d_name, version)) {
                strcpy (fullname, "./UPDATE/");
                strcat (fullname, dit->d_name);
                return fullname;
            }
    }

    if (closedir(dip) == -1)
    {
        fprintf(stderr, "error closing: '%s'\n", "./UPDATE");
        bye(-1);;
    }
    return NULL;
}

static void progress(unsigned int percent, unsigned int value)
{
    printf("\r%s: %d.%d%% (%d bytes).", "Flashing", percent / 10, percent % 10, value);
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

static unsigned int loadfile(unsigned char *buffer, char *name, unsigned int maxlen)
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

static unsigned int loadfile2(unsigned char *buffer, char *name, unsigned int maxlen)
{
    unsigned int len;
    FILE *f;

    f = fopen(name, "rb");
    if (f == NULL) {
        printf("not found: '%s'\n", name);
        return 0;
    } else {
        len = fread(buffer, 1, maxlen, f);
        printf("found: '%s' ($%04x bytes)\n", name, len);
    }
    fclose(f);
    return len;
}

static void checksdcard(int sdinserted)
{
    while (!sdinserted) {
        int version;
        LOGERR("please remove the sd card before flashing.\n");
        printf("press any key to continue.\n");
        getkey();
        if (chameleon_getversion(&version, &sdinserted) < 0) {
            LOGERR("getversion failed.\n");
            bye(-1);
        }
    }
}

int main(int argc, char *argv[])
{
    char *core_name;
    char *rom_name;
    unsigned char *buffer = malloc(CHAMELEON_RAM_SIZE);
    unsigned char *buffer2 = malloc(CHAMELEON_RAM_SIZE);
    int addr, len, i, romlen;
    COREINFO cinfo;
    int mcversion,sdinserted,chversion;
    unsigned char flashvendor, flashid;

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
            usage();
            exit (EXIT_FAILURE);
            break;
        }
    }

    if (chameleon_init() < 0) {
        LOGERR("initialization failed.\n");
        bye(-1);
    }
    if (chameleon_getversion(&mcversion, &sdinserted) < 0) {
        LOGERR("getversion failed.\n");
        bye(-1);
    }
    LOGVER("Firmware Version: %02x\n", mcversion);
    LOGVER("sd card detected: %s\n", sdinserted ? "no" : "yes");

    if (chameleon_getflashid(&flashvendor, &flashid) < 0) {
        LOGERR("getflashid failed.\n");
        bye(-1);
    }
    /* chameleon v1: 01 20 18 03 01 */
    LOGVER("Flash Vendor: %02x\n", flashvendor);
    LOGVER("Flash ID: %02x\n", flashid);
    
    if (flashvendor == 0x01) {
        chversion = 0;
        printf("detected Chameleon v1 Hardware\n");
    } else {
        chversion = 1;
        printf("detected Chameleon v2 Hardware\n");
    }
#ifdef LINUX
    /* make sure that if the binary is setuid root, the created files will be
       owned by the user running the binary (and not root) */
    {
        int fsuid = setfsuid(getuid());
        int fsgid = setfsgid(getgid());
        LOGVER("fsuid: %d fsgid: %d\n", fsuid, fsgid);
    }
#endif

    chameleon_setprogressfunc(1000, (void (*)(unsigned int, unsigned int))progress);
    chameleon_setlogfunc(logfunc);

    welcome();

    rom_name = "./UPDATE/rom-menu.bin";
    core_name = findcore(chversion);
    if (core_name == NULL) {
        LOGERR("FPGA core not found.\n");
        bye(-1);
    }
    LOGVER("using core:    '%s'\n", core_name);
    LOGVER("using romfile: '%s'\n", rom_name);

    /* update slot 0 */
    {
        unsigned char *rom_buffer = malloc(CHAMELEON_RAM_SIZE);

        checksdcard(sdinserted);

        addr = 0;

        makename(cinfo.core_name, core_name);

        printf("using core name: '%s'\n", cinfo.core_name);
        len = loadfile(buffer, core_name, CHAMELEON_SLOT_SIZE);

        cinfo.core_length = len;
        printf("flashing core from '%s' (%d bytes to %08x.)...\n", core_name, len, addr);

        romlen = loadfile(rom_buffer, rom_name, CHAMELEON_SLOT_SIZE);
        printf("flashing roms from '%s' (%d bytes to %08x.)...\n", rom_name, romlen, addr + len);
        cinfo.rom_length = romlen;

        if ((len + romlen + 3) > CHAMELEON_SLOT_SIZE) {
            fprintf(stderr, "error: binary size exceeds slot size.\n");
            exit(EXIT_FAILURE);
        }

        /* load extra ROMs */
        loadfile2(&rom_buffer[(RAMBASE_BASIC    - RAMBASE_CFGROM)], "./UPDATE/basic.rom"  , 1024 *  8);
        loadfile2(&rom_buffer[(RAMBASE_CHARGEN  - RAMBASE_CFGROM)], "./UPDATE/chargen.rom", 1024 *  4);
        loadfile2(&rom_buffer[(RAMBASE_KERNAL   - RAMBASE_CFGROM)], "./UPDATE/kernal.rom" , 1024 *  8);
        loadfile2(&rom_buffer[(RAMBASE_MMC64    - RAMBASE_CFGROM)], "./UPDATE/mmc64.rom"  , 1024 *  8);
        loadfile2(&rom_buffer[(RAMBASE_DEFCRT0  - RAMBASE_CFGROM)], "./UPDATE/slot1.rom"  , 1024 * 64);
        loadfile2(&rom_buffer[(RAMBASE_DEFCRT1  - RAMBASE_CFGROM)], "./UPDATE/slot2.rom"  , 1024 * 64);
        loadfile2(&rom_buffer[(RAMBASE_DRIVEROM - RAMBASE_CFGROM)], "./UPDATE/drive.rom"  , 1024 * 16);

        memset (buffer2, 0xff, CHAMELEON_SLOT_SIZE);
        len = chameleon_prepareslot(buffer2, buffer, &cinfo);
        memcpy(&buffer2[len], rom_buffer, romlen);
        len += romlen;

        chameleon_writeflash(buffer2, (len + 0xffff) & ~0xffff, addr);
        free(rom_buffer);

        chameleon_startcore(0);
    }

    chameleon_close();

    free(buffer);
    free(buffer2);

    bye(0);
    return EXIT_SUCCESS;
}
