
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
int videomode = 0;  // 0 means PAL, 1 means NTSC
int coremode = 0; // 0 means C64, 1 means vic20

#define RAMBASE_VRAM        0x00600000L
#define VRAM_SIZE           0x00100000L
#define FRAME_BUFFER_LEN    0x00050000L

#define C64_PAL_LINELEN       (512 - 92)
#define C64_PAL_LINELENBYTES  (C64_PAL_LINELEN / 2)
#define C64_PAL_XOFFS         17                /* xoffset in bytes */
#define C64_PAL_LINES         312
#define C64_PAL_YOFFS         65
#define C64_PAL_PIXPERLINE    512
#define C64_PAL_BYTESPERLINE  256

#define C64_PAL_FRAME_START     (RAMBASE_VRAM+(C64_PAL_YOFFS*C64_PAL_BYTESPERLINE))
#define C64_PAL_FRAME_SIZE      (C64_PAL_BYTESPERLINE*C64_PAL_LINES)

#define C64_NTSC_LINELEN       (512 - 92)
#define C64_NTSC_LINELENBYTES  (C64_NTSC_LINELEN / 2)
#define C64_NTSC_XOFFS         17                /* xoffset in bytes */
#define C64_NTSC_LINES         263
#define C64_NTSC_YOFFS         65
#define C64_NTSC_PIXPERLINE    512
#define C64_NTSC_BYTESPERLINE  256

#define C64_NTSC_FRAME_START     (RAMBASE_VRAM+(C64_NTSC_YOFFS*C64_NTSC_BYTESPERLINE))
#define C64_NTSC_FRAME_SIZE      (C64_NTSC_BYTESPERLINE*C64_NTSC_LINES)

#define VIC20_PAL_LINELEN       (512 - 274)
#define VIC20_PAL_LINELENBYTES  (VIC20_PAL_LINELEN / 2)
#define VIC20_PAL_XOFFS         17                /* xoffset in bytes */
#define VIC20_PAL_LINES         302
#define VIC20_PAL_YOFFS         10
#define VIC20_PAL_PIXPERLINE    512
#define VIC20_PAL_BYTESPERLINE  256

#define VIC20_PAL_FRAME_START     (RAMBASE_VRAM+(VIC20_PAL_YOFFS*VIC20_PAL_BYTESPERLINE))
#define VIC20_PAL_FRAME_SIZE      (VIC20_PAL_BYTESPERLINE*VIC20_PAL_LINES)

unsigned char *framebuffer = NULL;

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
    "chshot %s (compiled on %s)\n"
    "usage: chshot <options>\n"
    "\n"
    "-h --help          this help\n"
    "--verbose          enable verbose messages\n"
    "--debug            enable debug messages\n"
    "--ntsc             assume NTSC mode\n"
    "--vic20            assume VIC20 core\n"
    "\n"
    "-o Filename        save screenshot\n"
    , CHCODENET_VERSION
    , __DATE__
    );
}

int read_frame(int n)
{
    unsigned long addr = C64_PAL_FRAME_START + (FRAME_BUFFER_LEN * n);
    unsigned long len = C64_PAL_FRAME_SIZE;
    if ((coremode == 0) && (videomode == 0)) {
        addr = C64_PAL_FRAME_START + (FRAME_BUFFER_LEN * n);
        len = C64_PAL_FRAME_SIZE;
    } else if ((coremode == 0) && (videomode == 1)) {
        addr = C64_NTSC_FRAME_START + (FRAME_BUFFER_LEN * n);
        len = C64_NTSC_FRAME_SIZE;
    } else if ((coremode == 1) && (videomode == 0)) {
        addr = VIC20_PAL_FRAME_START + (FRAME_BUFFER_LEN * n);
        len = VIC20_PAL_FRAME_SIZE;
    }
    return chameleon_readmemory(framebuffer, len, addr);
}

unsigned char bmphdr[54] = {
0x42, 0x4D,               // 0        2        signature, must be 4D42 hex
0x8A, 0x18, 0x01, 0x00,   // 2        4        size of BMP file in bytes (unreliable)
0x00, 0x00,               // 6        2        reserved, must be zero
0x00, 0x00,               // 8        2        reserved, must be zero
0x36, 0x04, 0x00, 0x00,   // 10       4        offset to start of image data in bytes
0x28, 0x00, 0x00, 0x00,   // 14       4        size of BITMAPINFOHEADER structure, must be 40
0x00, 0x00, 0x00, 0x00,   // 18       4        image width in pixels
0x00, 0x00, 0x00, 0x00,   // 22       4        image height in pixels
0x01, 0x00,               // 26       2        number of planes in the image, must be 1
0x08, 0x00,               // 28       2        number of bits per pixel (1, 4, 8, or 24)
0x00, 0x00, 0x00, 0x00,   // 30       4        compression type (0=none, 1=RLE-8, 2=RLE-4)
0x00, 0x18, 0x03, 0x00,   // 34       4        size of image data in bytes (including padding)
0x13, 0x0B, 0x00, 0x00,   // 38       4        horizontal resolution in pixels per meter (unreliable)
0x13, 0x0B, 0x00, 0x00,   // 42       4        vertical resolution in pixels per meter (unreliable)
0x00, 0x00, 0x00, 0x00,   // 46       4        number of colors in image, or zero
0x00, 0x00, 0x00, 0x00    // 50       4        number of important colors, or zero
};

unsigned int palette_pepto_pal[16 * 4] = {
    0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0x00,
    0x2B, 0x37, 0x68, 0x00,
    0xB2, 0xA4, 0x70, 0x00,
    0x86, 0x3D, 0x6F, 0x00,
    0x43, 0x8D, 0x58, 0x00,
    0x79, 0x28, 0x35, 0x00,
    0x6F, 0xC7, 0xB8, 0x00,
    0x25, 0x4F, 0x6F, 0x00,
    0x00, 0x39, 0x43, 0x00,
    0x59, 0x67, 0x9A, 0x00,
    0x44, 0x44, 0x44, 0x00,
    0x6C, 0x6C, 0x6C, 0x00,
    0x84, 0xD2, 0x9A, 0x00,
    0xB5, 0x5E, 0x6C, 0x00,
    0x95, 0x95, 0x95, 0x00
};

unsigned char palette_pepto_ntsc_sony[4 * 16] = {
    0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0x00,
    0x2B, 0x35, 0x7C, 0x00,
    0xB1, 0xA6, 0x5A, 0x00,
    0x85, 0x41, 0x69, 0x00,
    0x43, 0x86, 0x5D, 0x00,
    0x78, 0x2E, 0x21, 0x00,
    0x6F, 0xBE, 0xCF, 0x00,
    0x26, 0x4A, 0x89, 0x00,
    0x00, 0x33, 0x5B, 0x00,
    0x59, 0x64, 0xAF, 0x00,
    0x43, 0x43, 0x43, 0x00,
    0x6B, 0x6B, 0x6B, 0x00,
    0x84, 0xCB, 0xA0, 0x00,
    0xB3, 0x65, 0x56, 0x00,
    0x95, 0x95, 0x95, 0x00
};

unsigned int palette_mike_pal[16 * 4] = {
    0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0x00,
    0x21, 0x1F, 0xB6, 0x00,
    0xFF, 0xF0, 0x4D, 0x00,
    0xFF, 0x3F, 0xB4, 0x00,
    0x37, 0xE2, 0x44, 0x00,
    0xFF, 0x34, 0x1A, 0x00,
    0x1B, 0xD7, 0xDC, 0x00,
    0x00, 0x54, 0xCA, 0x00,
    0x72, 0xB0, 0xE9, 0x00,
    0x93, 0x92, 0xE7, 0x00,
    0xFD, 0xF7, 0x9A, 0x00,
    0xFF, 0x9F, 0xE0, 0x00,
    0x93, 0xE4, 0x8F, 0x00,
    0xFF, 0x90, 0x82, 0x00,
    0x85, 0xDE, 0xE5, 0x00
};

int save_frame(FILE *f)
{
    int x,y,i;
    unsigned char hdr[54];
    unsigned char *p;
    memcpy(hdr, bmphdr, 54);
    hdr[10]=54;                          /* offset to data */
    hdr[11]=4;

    if ((coremode == 0) && (videomode == 0)) {
        hdr[22]=C64_PAL_LINES & 0xff;        /* height */
        hdr[23]=C64_PAL_LINES >> 8;
        hdr[18]=C64_PAL_LINELEN & 0xff;      /* width */
        hdr[19]=C64_PAL_LINELEN >> 8;
        fwrite(hdr, 1, 54, f);
        for (i = 0; i < 16*4; i++) {
            fputc(palette_pepto_pal[i],f);
        }
        for (i = 0; i < 240; i++) {
            fputc(i * 4,f);fputc(i* 4,f);fputc(i* 4,f);fputc(0,f);
        }
        for (y = (C64_PAL_LINES-1); y >= 0; y--) {
            p = framebuffer + C64_PAL_XOFFS + (C64_PAL_BYTESPERLINE * y);
            for (x = 0; x < C64_PAL_LINELENBYTES; x++) {
                fputc(p[x] & 0x0f, f);
                fputc(p[x] >> 4, f);
            }
        }
    } else if ((coremode == 0) && (videomode == 1)) {
        hdr[22]=C64_NTSC_LINES & 0xff;        /* height */
        hdr[23]=C64_NTSC_LINES >> 8;
        hdr[18]=C64_NTSC_LINELEN & 0xff;      /* width */
        hdr[19]=C64_NTSC_LINELEN >> 8;
        fwrite(hdr, 1, 54, f);
        for (i = 0; i < 16*4; i++) {
            fputc(palette_pepto_ntsc_sony[i],f);
        }
        for (i = 0; i < 240; i++) {
            fputc(i * 4,f);fputc(i* 4,f);fputc(i* 4,f);fputc(0,f);
        }
        for (y = (C64_NTSC_LINES-1); y >= 0; y--) {
            p = framebuffer + C64_NTSC_XOFFS + (C64_NTSC_BYTESPERLINE * y);
            for (x = 0; x < C64_NTSC_LINELENBYTES; x++) {
                fputc(p[x] & 0x0f, f);
                fputc(p[x] >> 4, f);
            }
        }
    } else if ((coremode == 1) && (videomode == 0)) {
        hdr[22]=VIC20_PAL_LINES & 0xff;        /* height */
        hdr[23]=VIC20_PAL_LINES >> 8;
        hdr[18]=(VIC20_PAL_LINELEN * 2) & 0xff;      /* width */
        hdr[19]=(VIC20_PAL_LINELEN * 2) >> 8;
        fwrite(hdr, 1, 54, f);
        for (i = 0; i < 16*4; i++) {
            fputc(palette_mike_pal[i],f);
        }
        for (i = 0; i < 240; i++) {
            fputc(i * 4,f);fputc(i* 4,f);fputc(i* 4,f);fputc(0,f);
        }
        for (y = (VIC20_PAL_LINES-1); y >= 0; y--) {
            p = framebuffer + VIC20_PAL_XOFFS + (VIC20_PAL_BYTESPERLINE * y);
            for (x = 0; x < VIC20_PAL_LINELENBYTES; x++) {
                fputc(p[x] & 0x0f, f);
                fputc(p[x] & 0x0f, f);
                fputc(p[x] >> 4, f);
                fputc(p[x] >> 4, f);
            }
        }
    }
    return 0;
}

int cleanup(int n) {
    chameleon_close();
    if (framebuffer) {
        free(framebuffer);
        framebuffer = NULL;
    }
    return n;
}

int main(int argc, char *argv[])
{
    char *name;
    int i;
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
        } else if (!strcmp("--ntsc", argv[i])) {
            videomode = 1;
        } else if (!strcmp("--vic20", argv[i])) {
            coremode = 1;
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
    if((framebuffer = (unsigned char*)malloc(VRAM_SIZE)) == NULL) {
        LOGERR("could not allocate memory.\n");
        exit(cleanup(EXIT_FAILURE));
    };

    /* check the rest of the options */
    for (; i < argc; i++) {
        if (!strcmp("-o", argv[i])) {
            /* save screenshot */
            i++;
            name = argv[i];
            if ((f = fopen(name, "wb")) == NULL) {
                LOGERR("error opening: '%s'\n", name);
                exit(cleanup(EXIT_FAILURE));
            }
            read_frame(0);
            save_frame(f);
            fclose(f);
        } else {
            usage();
            exit(cleanup(EXIT_FAILURE));
        }
    }

    return cleanup(EXIT_SUCCESS);
}
