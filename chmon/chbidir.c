
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "chacolib.h"

//#define LOGUSB(_x_)      printf _x_
#define LOGUSB(_x_)

#define DATASIZE (256)
#define BUFSIZE  (DATASIZE+4)

#define C64WBUF_BASE (0x07e8-(2*BUFSIZE))
#define C64RBUF_BASE (0x07e8-BUFSIZE)

#define WDATA     (0)        /* 256 bytes */
#define WSEQ      (DATASIZE+0)
#define WBYTES    (DATASIZE+1)
#define WSTART    (DATASIZE+2)

#define RDATA     (0)        /* 256 bytes */
#define RSEQ      (DATASIZE+0)
#define RBYTES    (DATASIZE+1)
#define RSTART    (DATASIZE+2)

#define C64BUF_WDATA     (C64WBUF_BASE+WDATA)        /* 256 bytes */
#define C64BUF_WSEQ      (C64WBUF_BASE+WSEQ)
#define C64BUF_WBYTES    (C64WBUF_BASE+WBYTES)
#define C64BUF_WSTART    (C64WBUF_BASE+WSTART)

#define C64BUF_RDATA     (C64RBUF_BASE+RDATA)        /* 256 bytes */
#define C64BUF_RSEQ      (C64RBUF_BASE+RSEQ)
#define C64BUF_RBYTES    (C64RBUF_BASE+RBYTES)
#define C64BUF_RSTART    (C64RBUF_BASE+RSTART)

#define LOGERR(...)       logfunc (LOGLVL_ERR, __VA_ARGS__ )
#define LOGVER(...)       logfunc (LOGLVL_VER, __VA_ARGS__ )
#define LOGMSG(...)       logfunc (LOGLVL_MSG, __VA_ARGS__ )
#define DBG(...)          logfunc (LOGLVL_DBG, __VA_ARGS__ )

extern int cleanup(int n);

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

/*******************************************************************************
 * write to c64
 ******************************************************************************/

static unsigned char wbuf[BUFSIZE];

int usb_write_byte(unsigned char *buf)
{
    LOGUSB(("usb_write_byte\n"));

    // signal C64 that we want to write
    wbuf[WBYTES] = 1;           // bytes left
    chameleon_writememory(&wbuf[WBYTES], 1, C64BUF_WBYTES);

    LOGUSB((" usb_write_byte 1 byte: wait for c64 ready "));fflush(stdout);
    do { // wait until c64 is ready to recieve
        chameleon_readmemory(&wbuf[WSEQ], 3, C64BUF_WSEQ);
        LOGUSB(("(seq:%02x bytes:%02x start:%02x)",wbuf[WSEQ],wbuf[WBYTES],wbuf[WSTART]));fflush(stdout);
    } while (wbuf[WSTART] != 0xff);
    LOGUSB(("(ok) (last seq: %02x)\n", wbuf[WSEQ]));

    // data to c64
    wbuf[WSTART] = 0;             // start flag
    wbuf[WDATA] = *buf;         // data
    wbuf[WSEQ] = wbuf[WSEQ] + 1;  // sequence number
    LOGUSB((" usb_write_byte 1 byte: %02x (seq: %02x)  ", wbuf[WDATA], wbuf[WSEQ])); fflush(stdout);


    chameleon_writememory(&wbuf[WSTART], 1, C64BUF_WSTART);
    chameleon_writememory(&wbuf[WDATA], 1, C64BUF_WDATA);
    chameleon_writememory(&wbuf[WSEQ], 1, C64BUF_WSEQ);

    wbuf[WBYTES] = 0;           // bytes left
    chameleon_writememory(&wbuf[WBYTES], 1, C64BUF_WBYTES);
    LOGUSB(("(ok)\n"));

    return 1;
}

int usb_write_bytes(unsigned char *buf, int len)
{
    int i;
    LOGUSB(("ftdiwrite %d bytes\n", len));

    for (i=0;i<len;i++) {

        // signal C64 that we want to write
        wbuf[WBYTES] = 1;           // bytes left
        chameleon_writememory(&wbuf[WBYTES], 1, C64BUF_WBYTES);

        LOGUSB((" ftdiwrite 1 byte: wait for c64 ready "));fflush(stdout);
        do { // wait until c64 is ready to recieve
            chameleon_readmemory(&wbuf[WSEQ], 3, C64BUF_WSEQ);
            LOGUSB(("(seq:%02x bytes:%02x start:%02x)",wbuf[WSEQ],wbuf[WBYTES],wbuf[WSTART]));fflush(stdout);
        } while (wbuf[WSTART] != 0xff);
        LOGUSB(("(ok) (last seq: %02x)\n", wbuf[WSEQ]));

        // data to c64
        wbuf[WSTART] = 0;             // start flag
        wbuf[WDATA] = buf[i];         // data
        wbuf[WSEQ] = wbuf[WSEQ] + 1;  // sequence number
        LOGUSB((" ftdiwrite 1 byte: %02x (seq: %02x)  ", wbuf[WDATA], wbuf[WSEQ])); fflush(stdout);


        chameleon_writememory(&wbuf[WSTART], 1, C64BUF_WSTART);
        chameleon_writememory(&wbuf[WDATA], 1, C64BUF_WDATA);
        chameleon_writememory(&wbuf[WSEQ], 1, C64BUF_WSEQ);

        wbuf[WBYTES] = 0;           // bytes left
        chameleon_writememory(&wbuf[WBYTES], 1, C64BUF_WBYTES);
        LOGUSB(("(ok)\n"));
    }
    return len;
}

int usb_write_block(unsigned char *buf, int len)
{
    LOGUSB(("usb_write_block %d bytes\n", len));
    if (len > DATASIZE) {
        printf("internal error: usb_write_block len>%d)\n", DATASIZE);
        exit(EXIT_FAILURE);
    }

    // signal C64 that we want to write
    // HACK: somehow handle sizes>255 better
    wbuf[WBYTES] = (len > 0xff) ? 0xff : len;           // bytes left
    chameleon_writememory(&wbuf[WBYTES], 1, C64BUF_WBYTES);

    LOGUSB((" usb_write_block 1 byte: wait for c64 ready "));fflush(stdout);
    do { // wait until c64 is ready to recieve
        chameleon_readmemory(&wbuf[WSEQ], 3, C64BUF_WSEQ);
        LOGUSB(("(%02x %02x %02x)",wbuf[WSTART],wbuf[WDATA],wbuf[WSEQ]));fflush(stdout);
    } while (wbuf[WSTART] != 0xff);
    LOGUSB(("(ok) (last seq: %02x)\n", wbuf[2]));

    // data to c64
    wbuf[WSTART] = 0;           // start flag
    memcpy(&wbuf[WDATA], &buf[0], len);      // data
    wbuf[WSEQ] = wbuf[WSEQ] + 1;  // sequence number
    LOGUSB((" usb_write_block 1 byte: %02x (seq: %02x)  ", wbuf[WDATA], wbuf[WSEQ]));fflush(stdout);

    chameleon_writememory(&wbuf[WSTART], 1, C64BUF_WSTART);
    chameleon_writememory(&wbuf[WDATA], len, C64BUF_WDATA);
    chameleon_writememory(&wbuf[WSEQ], 1, C64BUF_WSEQ);

    wbuf[WBYTES] = 0;           // bytes left
    chameleon_writememory(&wbuf[WBYTES], 1, C64BUF_WBYTES);
    LOGUSB(("(ok)\n"));
    return len;
}

/*******************************************************************************
 * read from c64
 ******************************************************************************/

static unsigned char rbuf[BUFSIZE];

int usb_bytes_available(void)
{
    chameleon_readmemory(&rbuf[RSTART], 1, C64BUF_RSTART);
    return rbuf[RSTART] == 0xff;
}

int usb_read_byte(unsigned char *buf)
{
    LOGUSB((" usb_read_byte 1 byte: wait for c64 ready "));fflush(stdout);
    do { // wait until c64 wants to send
        chameleon_readmemory(&rbuf[RSEQ], 3, C64BUF_RSEQ);
        LOGUSB(("(%02x %02x %02x)",rbuf[RSTART],rbuf[RDATA],rbuf[RSEQ]));fflush(stdout);
    } while (rbuf[RSTART] != 0xff);
    LOGUSB(("(ok) (last seq: %02x)\n", rbuf[RSEQ]));

    chameleon_readmemory(&rbuf[RDATA], 1, C64BUF_RDATA);
    *buf = rbuf[RDATA];
    LOGUSB((" usb_read_byte 1 byte: %02x\n", *buf));

    // clear the start flag
    rbuf[RSTART] = 0;             // start flag
    chameleon_writememory(&rbuf[RSTART], 1, C64BUF_RSTART);

    return 1;
}

int usb_read_bytes(unsigned char * buf, int len)
{
    int i;
    LOGUSB(("ftdiread %d bytes\n", len));
    for (i=0;i<len;i++) {

        LOGUSB((" ftdiread 1 byte: wait for c64 ready "));fflush(stdout);
        do { // wait until c64 wants to send
            chameleon_readmemory(&rbuf[RSEQ], 3, C64BUF_RSEQ);
            LOGUSB(("(%02x %02x %02x)",rbuf[RSTART],rbuf[RDATA],rbuf[RSEQ]));fflush(stdout);
        } while (rbuf[RSTART] != 0xff);
        LOGUSB(("(ok) (last seq: %02x)\n", rbuf[RSEQ]));

        chameleon_readmemory(&rbuf[RDATA], 1, C64BUF_RDATA);
        buf[i] = rbuf[RDATA];
        LOGUSB((" ftdiread 1 byte: %02x\n", buf[i]));

        // clear the start flag
        rbuf[RSTART] = 0;             // start flag
        chameleon_writememory(&rbuf[RSTART], 1, C64BUF_RSTART);

    }
    return len;
}

int usb_read_block(unsigned char *buf, int len)
{
    LOGUSB(("usb_read_block %d bytes\n", len));
    if (len > DATASIZE) {
        printf("internal error: usb_read_block len>%d)\n", DATASIZE);
        exit(EXIT_FAILURE);
    }

    LOGUSB((" usb_read_block %d bytes: wait for c64 ready ", len));fflush(stdout);
    do { // wait until c64 wants to send
        chameleon_readmemory(&rbuf[RSEQ], 3, C64BUF_RSEQ);
        LOGUSB(("(%02x %02x %02x)",rbuf[RSTART],rbuf[RDATA],rbuf[RSEQ]));fflush(stdout);
    } while (rbuf[RSTART] != 0xff);
    LOGUSB(("(ok) (last seq: %02x)\n", rbuf[RSEQ]));

    chameleon_readmemory(buf, len, C64BUF_RDATA);
    LOGUSB((" usb_read_block %s bytes: %02x\n", len, buf[0]));

    // clear the start flag
    rbuf[RSTART] = 0;             // start flag
    chameleon_writememory(&rbuf[RSTART], 1, C64BUF_RSTART);

    return len;
}

/******************************************************************************/

int check_server_running(void)
{
    /* check BASIC direct- oder program-mode */
    if (chameleon_readmemory(&rbuf[0], 1, 0x9d) < 0) {
        LOGERR("error reading C64 memory.");
        exit(cleanup(EXIT_FAILURE));
    }
//    printf("%02x - ", rbuf[0]);
    if (rbuf[0] != 0x00) {
//        printf("server not running\n");
        return 0;
    }
//    printf("server running\n");
    return 1;
}

