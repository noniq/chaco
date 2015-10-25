
#include <string.h>

#define GCR_BPS 325 /* currently buffer must be large enough for this */

#define DATASIZE (GCR_BPS)
#define BUFSIZE  (DATASIZE+4)

#define C64RBUF_BASE (0x07e8-(2*BUFSIZE))
#define C64WBUF_BASE (0x07e8-BUFSIZE)

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

extern void get_block(void);

int usb_get_block(unsigned char *ptr, unsigned short len)
{
    get_block();
    memcpy(ptr, (unsigned char*)C64BUF_RDATA, len);
    return len;
}
