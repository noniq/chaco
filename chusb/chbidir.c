
//#define LOGUSB(_x_)      printf _x_
#define LOGUSB(_x_)


#define C64WBUF_BASE 0x3800
#define C64RBUF_BASE 0x3a00

#define WDATA     (0)        /* 256 bytes */
#define WSEQ      (256+0)
#define WBYTES    (256+1)
#define WSTART    (256+2)

#define RDATA     (0)        /* 256 bytes */
#define RSEQ      (256+0)
#define RBYTES    (256+1)
#define RSTART    (256+2)

#define C64BUF_WDATA     (C64WBUF_BASE+WDATA)        /* 256 bytes */
#define C64BUF_WSEQ      (C64WBUF_BASE+WSEQ)
#define C64BUF_WBYTES    (C64WBUF_BASE+WBYTES)
#define C64BUF_WSTART    (C64WBUF_BASE+WSTART)

#define C64BUF_RDATA     (C64RBUF_BASE+RDATA)        /* 256 bytes */
#define C64BUF_RSEQ      (C64RBUF_BASE+RSEQ)
#define C64BUF_RBYTES    (C64RBUF_BASE+RBYTES)
#define C64BUF_RSTART    (C64RBUF_BASE+RSTART)


/*******************************************************************************
 * write to c64
 ******************************************************************************/

static unsigned char wbuf[256+4];

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
    if (len > 256) {
        printf("internal error: usb_write_block len>256)\n");
        exit(EXIT_FAILURE);
    }

    // signal C64 that we want to write
    wbuf[WBYTES] = len;           // bytes left
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

static unsigned char rbuf[256+4];

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
    if (len > 256) {
        printf("internal error: usb_read_block len>256)\n");
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

