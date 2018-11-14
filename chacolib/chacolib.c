
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#if defined (LIBUSB)
#include <libusb-1.0/libusb.h>
#elif defined (HIDAPI)
#include <hidapi.h>
#else
#error "either LIBUSB or HIDAPI must be defined"
#endif

#ifndef DEBUG
/* #define DEBUG */
#endif

#include "chacolib.h"

/******************************************************************************/

#define CHAMELEON_VID 0x18D8
#define CHAMELEON_PID 0x201D

/*
    Caution: for the (SPI)flash-related commands to work, the FPGA must be reset
             first, since the FPGA also wants to be SPI-Master

    Dest  Ctrl  Data           Reply

    0x00  -     -              [0-2] Status           get status
    0xf0  -     -              [0-1] Version          get version

    0xf1  -     -              -                      start boot loader

    0x01  -     -              [0-31] Data            read flash
    0xb0  -     [0-3] Addr     -                      set read pointer

    0x02  0     [0-31] Data    [0-31] Flash Status    write flash
    0xb1  -     [0-3] Addr     -                      set write pointer
    0x03  -     [0] Sector     [0-31] Flash Status    sector erase

    0x04  -     -              -                      reset read+write pointer

    0x06  core  -              [0-2] core size        start core
    0x07  -     -              -                      reset fpga
    0x08  -     [0] Slot       -                      set jtag slot

    0x90  -     [0-3] Addr     [0-31] Data            ram read

    0x92  -     [0-3] Addr     [0-31] Dummy           ram write
    0x93  size  [0-31] Data    [0-31] Dummy           ram write data
*   0x9f  -     -              [0-31] Dummy ?         ram write stop

    0x13  -     -              [0-4] flash id         get flash id

obsolete (remove from firmware):

*   0x9e  -     -              [0-31] Data            ?
*   0xa1                                              uart transmission ?

    0x05  -     -              -                      send flash write enable command
    0x23  -     -                                     read flash status register

*/

#define WRITE_FLASH         0x02
#define READ_FLASH          0x01
#define SECTOR_ERASE        0x03
#define START_CH            0x06
#define RESET_POINTER       0x04
#define SET_READPOINTER     0xB0
#define SET_WRITEPOINTER    0xB1
#define GETSTATUS           0x00
#define GETVERSION          0xF0

#define CMD_GET_FLASH_ID    0x13

#define CMD_RESET_FPGA          0x07
#define CMD_SET_JTAGSLOT        0x08

#define NUM_DATA_BYTES 32

#define SIZEOF_USBHIDDATAFRAME (NUM_DATA_BYTES + 2)

/* grr.. this doesnt work correctly with neither mingw32 nor linux native */
#if 0
#define APACKED  __attribute__((__packed__))
#else
#define APACKED
#endif
//#pragma GCC diagnostic push
//#pragma GCC diagnostic ignored "-Wattributes"
typedef struct
{
    unsigned char Destination;
    unsigned char Control;
    unsigned char Data[NUM_DATA_BYTES];
} USBHIDDataFrame APACKED;
//#pragma GCC diagnostic warning "-Wattributes"
//#pragma GCC diagnostic pop

#ifdef POSIX
#include <unistd.h>
static inline void Sleep(unsigned int num) {
    usleep(1000*num);
}
#else
#include <windows.h>
#endif

/*******************************************************************************
 * Logging helper functions
 *******************************************************************************/

static chamlog_t fprintf_ptr = NULL;

#define LOGERR(...)       if (fprintf_ptr) (*fprintf_ptr)(LOGLVL_ERR, __VA_ARGS__ )
#define LOGVER(...)       if (fprintf_ptr) (*fprintf_ptr)(LOGLVL_VER, __VA_ARGS__ )
#define LOGMSG(...)       if (fprintf_ptr) (*fprintf_ptr)(LOGLVL_MSG, __VA_ARGS__ )
#define DBG(...)          if (fprintf_ptr) (*fprintf_ptr)(LOGLVL_DBG, __VA_ARGS__ )

void chameleon_setlogfunc(chamlog_t func)
{
    fprintf_ptr = func;
}

/*******************************************************************************
 * include wrapper for USB functions
 *******************************************************************************/

#if defined (LIBUSB)
#include "chacolib-libusb.c"
#elif defined (HIDAPI)
#include "chacolib-hidapi.c"
#endif

/*******************************************************************************
 * Progress helper functions
 *******************************************************************************/

void (*progressfunc)(unsigned int,unsigned int) = NULL;
static unsigned int progresslen = 100;

static unsigned int progresspercent = 0;
static unsigned int progressvalue = 0;

void chameleon_setprogressfunc(unsigned int len, void (*func)(unsigned int,unsigned int))
{
    progressfunc = func;
    progresslen = len;
}

void chameleon_progresshelper(unsigned int percent, unsigned int value)
{
    progresspercent = percent;
    progressvalue = value;
    DBG("progress: %08x (%d.%d%%)\n", value, percent / 10, percent % 10);
}

int chameleon_getprogresspercent(void)
{
    return progresspercent;
}

int chameleon_getprogressvalue(void)
{
    return progressvalue;
}

static void progress(unsigned int curr, unsigned int max)
{
    static unsigned int oldPercent;
    unsigned int percent;

    if ((max == 0)&&(curr == 0)) {
        oldPercent = 0;
        progresspercent = 0;
        progressvalue = 0;
        if (progressfunc) {
            progressfunc(0, 0);
        }
    } else {
        if (max > 0x003fffff) {
            percent = ((progresslen * (curr >> 10)) / (max >> 10));
        } else {
            percent = ((progresslen * curr) / max);
        }
        if(percent > oldPercent)
        {
            oldPercent = percent;

            if (progressfunc) {
                progressfunc(percent, curr);
            }
//            DBG("progress: %d %%\n", percent);
        }
    }
}

/*****************************************************************************/

#define CMDSET(x,y) usbData.Destination = (x); usbData.Control = (y)
#define ADDRSET(x) \
usbData.Data[0] = ((x) >> 0) & 0xFF; \
usbData.Data[1] = ((x) >> 8) & 0xFF; \
usbData.Data[2] = ((x) >> 16) & 0xFF; \
usbData.Data[3] = ((x) >> 24) & 0xFF;

static int fpgarunning = -1;

static int cmd_reset_pointer(void)
{
    USBHIDDataFrame usbData;
    memset(&usbData, 0, sizeof(USBHIDDataFrame));

    CMDSET(RESET_POINTER, 0);
    chameleon_writedata(&usbData);

    return CHACO_OK;
}

static int cmd_get_status(unsigned char *buffer)
{
    int rc;
    USBHIDDataFrame usbData;
    memset(&usbData, 0, sizeof(USBHIDDataFrame));

    CMDSET(GETSTATUS, 0);
    chameleon_writedata(&usbData);
    rc = chameleon_readdata(&usbData,100);
    if (rc < 0) {
        return -1;
    }
    memcpy(buffer, &usbData.Data[0], NUM_DATA_BYTES);
    return CHACO_OK;
}

static int cmd_get_version(unsigned char *buffer)
{
    int rc;
    USBHIDDataFrame usbData;
    memset(&usbData, 0, sizeof(USBHIDDataFrame));

    CMDSET(GETVERSION, 0);
    chameleon_writedata(&usbData);
    Sleep (10);
    rc = chameleon_readdata(&usbData,2000);
    if (rc < 0) {
        return -1;
    }
    memcpy(buffer, &usbData.Data[0], NUM_DATA_BYTES);
    return CHACO_OK;
}

static int cmd_get_flash_id(unsigned char *buffer)
{
    int rc;
    USBHIDDataFrame usbData;
    memset(&usbData, 0, sizeof(USBHIDDataFrame));

    CMDSET(CMD_GET_FLASH_ID, 0);
    chameleon_writedata(&usbData);
    Sleep (10);
    rc = chameleon_readdata(&usbData,2000);
    if (rc < 0) {
        return -1;
    }
    memcpy(buffer, &usbData.Data[0], NUM_DATA_BYTES);
    return CHACO_OK;
}

static int cmd_fpga_reset(void)
{
    USBHIDDataFrame usbData;

    if (fpgarunning != 0) {
        DBG("cmd_fpga_reset\n");
        memset(&usbData, 0, sizeof(USBHIDDataFrame));

        CMDSET(CMD_RESET_FPGA, 0);
        chameleon_writedata(&usbData);
        fpgarunning = 0;
        Sleep(400);
    }
    
    return CHACO_OK;
}

static int cmd_fpga_start(int core)
{
    USBHIDDataFrame usbData;

    fpgarunning = 0;

    memset(&usbData, 0, sizeof(USBHIDDataFrame));

    CMDSET(START_CH, (unsigned char)core);
    usbData.Data[0] = 0;
    chameleon_writedata(&usbData);
    fpgarunning = 1;
    Sleep(400);
    if(chameleon_readdata(&usbData,8000)==-1) {
        return -1;
    }

    return (((int)usbData.Data[0]) * 65536) + (((int)usbData.Data[1]) * 256) + ((int)usbData.Data[2]);
}

int cmd_set_jtagslot(int slot)
{
    USBHIDDataFrame usbData;
    int rc;

    memset(&usbData, 0, sizeof(USBHIDDataFrame));

    CMDSET(CMD_SET_JTAGSLOT, 0);
    usbData.Data[0] = slot;

    rc = chameleon_writedata(&usbData);

    /* FIXME: this does not seem to work */
    rc = chameleon_readdata(&usbData, 400);
    if (rc != CHACO_OK) {
        LOGVER("cmd_set_jtagslot read timeout.\n");
        return -1;
    }
    if(slot != usbData.Data[1]) {
        LOGVER("cmd_set_jtagslot reply error.\n");
        return -1;
    }

    return CHACO_OK;
}

static int cmd_start_bootloader(void)
{
    USBHIDDataFrame usbData;
    int rc;

    memset(&usbData, 0, sizeof(USBHIDDataFrame));

    CMDSET(0xf1, 0);

    rc = chameleon_writedata(&usbData);
    if (rc != CHACO_OK) {
        LOGVER("cmd_start_bootloader failed.\n");
        return -1;
    }
    return CHACO_OK;
}

static int cmd_set_readptr(unsigned int addr)
{
    USBHIDDataFrame usbData;
    memset(&usbData, 0, sizeof(USBHIDDataFrame));

    CMDSET(SET_READPOINTER, 0);
    ADDRSET(addr);
    chameleon_writedata(&usbData);

    return CHACO_OK;
}

static int cmd_set_writeptr(unsigned int addr)
{
    USBHIDDataFrame usbData;
    memset(&usbData, 0, sizeof(USBHIDDataFrame));

    CMDSET(SET_WRITEPOINTER,0);
    ADDRSET(addr);
    chameleon_writedata(&usbData);

    return CHACO_OK;
}

static int cmd_sector_erase(unsigned int sector)
{
    USBHIDDataFrame usbData;
    memset(&usbData, 0, sizeof(USBHIDDataFrame));

    CMDSET(SECTOR_ERASE,0);
    usbData.Data[0] = sector;
    chameleon_writedata(&usbData);
    Sleep(1);
    if(chameleon_readdata(&usbData,2200)==-1) {
//            ChLastError = "Receive timeout";
        return -1;
    }
    if(usbData.Data[0] == 0xFF) {
//            ChLastError = "Erase timeout";
        return -1;
    }

    return CHACO_OK;
}

static int cmd_flash_read(unsigned char *buffer)
{
    USBHIDDataFrame usbData;
    memset(&usbData, 0, sizeof(USBHIDDataFrame));

    CMDSET(READ_FLASH,0);
    chameleon_writedata(&usbData);

    if(chameleon_readdata(&usbData,500)==-1)
    {
        return -1;
    }

    memcpy(buffer, &usbData.Data[0], NUM_DATA_BYTES);
    return CHACO_OK;
}

static int cmd_flash_write(unsigned char *buffer)
{
    USBHIDDataFrame usbData;
    memset(&usbData, 0, sizeof(USBHIDDataFrame));

    memcpy(&usbData.Data[0], buffer, NUM_DATA_BYTES);

    CMDSET(WRITE_FLASH,0);
    chameleon_writedata(&usbData);

    if(chameleon_readdata(&usbData,500) == -1)
    {
        LOGERR("Usb ERROR\n");
        return -1;
    }
    else
    {
        //if(rc == 2)dbg::dPrint("d[0] = %d , d[1] = %d", usbData.Data[0], usbData.Data[1]);
    }
    return CHACO_OK;
}

#define READMEMORY_TIMEOUT      (4000)

static int cmd_read_mem(unsigned char *buffer, unsigned int addr, int len)
{
    int rc;
    USBHIDDataFrame usbData;

    if ((len < 1) || (len > NUM_DATA_BYTES)) {
        return -1;
    }

    CMDSET(0x90, 0);
    ADDRSET(addr);
    chameleon_writedata(&usbData);
    rc = chameleon_readdata(&usbData,READMEMORY_TIMEOUT);
    if(rc < 0)
    {
        LOGERR("readMemory timeout 1!\n");
        return -1;
    }
    memcpy(buffer, &usbData.Data[0], len);
    return CHACO_OK;
}

static int cmd_write_mem_start(unsigned int addr)
{
    USBHIDDataFrame usbData;

    CMDSET(0x92, 0);
    ADDRSET(addr);
    chameleon_writedata(&usbData);
    if(chameleon_readdata(&usbData,2000)==-1)
    {
        LOGERR("Receive timeout\n");
        return -1;
    }
    return CHACO_OK;
}

static int cmd_write_mem(unsigned char *buffer, int len)
{
    USBHIDDataFrame usbData;
    memset(&usbData, 0, sizeof(USBHIDDataFrame));

    memcpy(&usbData.Data[0], buffer, len);

    CMDSET(0x93, len);
    chameleon_writedata(&usbData);

    if(chameleon_readdata(&usbData,2000)==-1)
    {
        LOGERR("Receive timeout\n");
        return -1;
    }
    return CHACO_OK;
}

static int cmd_write_mem_stop(void)
{
    USBHIDDataFrame usbData;

    CMDSET(0x9f, 0);
    chameleon_writedata(&usbData);
    if(chameleon_readdata(&usbData,2000)==-1)
    {
        return -1;
    }
    return CHACO_OK;
}

#undef CMDSET
#undef ADDRSET

/*****************************************************************************/

static unsigned char reverseBits(unsigned char byte)
{
    unsigned char tmp;
    unsigned result = 0x00;    
    int i;

    for(i = 0; i < 8; i++)
    {
        tmp = byte;
        result |= ((tmp & (0x01<<i))>>i)<<(7-i) ;
    }
    return result;
}

/*****************************************************************************/

static unsigned char *localData; /* pointer to unencoded data */
static unsigned int sizeData; /* size of unencoded data */
static unsigned int blockLength; /* NUM_DATA_BYTES */
static unsigned int readIndex;
static unsigned int blockCounter;

static void rle_init(unsigned char *data, unsigned int size, unsigned int blocklen)
{
    localData = data;
    sizeData = size;
    blockLength = blocklen;
    readIndex = 0;
    blockCounter = 0;
}

static int rle_test(unsigned int index)
{
    unsigned int cnt = 0;
    unsigned char firstByte = localData[index];

    while(cnt < 512)
//    while(cnt < 0x10000)
    {
        if(firstByte == localData[cnt + index]) {
            cnt++;
        }
        else { 
            break;
        }

        if((index + cnt) >= sizeData) {
            break;
        }
    }
    return cnt;
}

/* 
    blockBuffer - (out) pointer to output buffer (32 bytes max)
    usedBytes - (out) number of used bytes in output buffer
    Returns < 0 if failed
            = 0 if literal data
            > 0 number of 0xff bytes that are skipped
*/
static int rle_nextblock(unsigned char *blockBuffer, unsigned int * usedBytes)
{
    unsigned int maxBlockLen;
    unsigned int longestRLength;
    unsigned char sample;

    if(!localData) {
        return -1;
    }

    if(readIndex >= sizeData) {
        return -2;
    }

    /* pad length of last block */
    if(readIndex > (sizeData - blockLength)) {
        maxBlockLen = blockLength - (sizeData - readIndex);
    } else {
        maxBlockLen = blockLength;
    }

    longestRLength = rle_test(readIndex);
    sample = localData[readIndex];

    *usedBytes = 0;
#if 1
    /* we can skip 0xff bytes until the next 32-byte aligned block */
    if((longestRLength > 32) && (sample == 0xFF))
    {
        longestRLength &= ~(32 - 1);
        readIndex += longestRLength;
//        LOGVER("RLE IDX %08x LEN %08x...\n", readIndex, longestRLength);
        return longestRLength;
    }
#endif
    /* literal data */
    memcpy(blockBuffer, &localData[readIndex], maxBlockLen);
    *usedBytes = maxBlockLen;
    readIndex += maxBlockLen;
    return 0;
}

/*****************************************************************************/

static void putoffset3(unsigned char *destbuffer, unsigned int offset)
{
    destbuffer[0] = (offset >> 16) & 0xFF;
    destbuffer[1] = (offset >> 8) & 0xFF;
    destbuffer[2] = offset & 0xFF;
}

static void putoffset4(unsigned char *destbuffer, unsigned int offset)
{
    destbuffer[0] = (offset >> 24) & 0xFF;
    destbuffer[1] = (offset >> 16) & 0xFF;
    destbuffer[2] = (offset >> 8) & 0xFF;
    destbuffer[3] = offset & 0xFF;
}

/*
    cinfo->core_length
    cinfo->rom_length
*/
int chameleon_prepareslot(unsigned char *destbuffer, unsigned char *corebuffer, COREINFO *cinfo)
{
    unsigned char *ptr;
    unsigned int i,n;

    /* core binary block */
    i = 3;
    cinfo->core_offset = i;
    ptr = destbuffer + 3;
    for(n=0; n < cinfo->core_length; n++) {
        *ptr++ = reverseBits(corebuffer[n]);
        ++i;
    }
    /* core info block */
    cinfo->info_offset = i;
    cinfo->info_length = COREINFO_LEN;
    cinfo->rom_offset = i + COREINFO_LEN;
#if 1
    memcpy(cinfo->magic, COREINFO_MAGIC, COREINFO_MAGIC_LEN);
    memcpy(ptr, cinfo->magic, COREINFO_MAGIC_LEN); ptr+=COREINFO_MAGIC_LEN;
    cinfo->version = COREINFO_VERSION;
    putoffset4(ptr, cinfo->version); ptr+=COREINFO_VERSION_LEN;
    putoffset4(ptr, cinfo->core_length); ptr+=COREINFO_CORELENGTH_LEN;
    putoffset4(ptr, cinfo->core_offset); ptr+=COREINFO_COREOFFSET_LEN;
    putoffset4(ptr, cinfo->rom_length); ptr+=COREINFO_ROMLENGTH_LEN;
    putoffset4(ptr, cinfo->rom_offset); ptr+=COREINFO_ROMOFFSET_LEN;
    memcpy(ptr, cinfo->core_name, COREINFO_CORENAME_LEN); ptr+=COREINFO_CORENAME_LEN;
    putoffset4(ptr, cinfo->info_length); ptr+=COREINFO_INFOLENGTH_LEN;
    putoffset4(ptr, cinfo->info_offset); ptr+=COREINFO_INFOOFFSET_LEN;
#endif

    /* offset to first byte after core (ROM payload) */
    putoffset3(destbuffer, cinfo->rom_offset);
    return cinfo->rom_offset;
}

/*****************************************************************************/

int chameleon_readmemory(unsigned char * buffer, int length, int address)
{
    unsigned int localAddress = address;
    unsigned int remainingSize = length;
    unsigned int fSize;
    int rc;

    LOGVER("reading %08x bytes from %08x...\n", length, address);
    progress(0,0);

    while(remainingSize > 0)
    {
        fSize = NUM_DATA_BYTES;
        if(remainingSize < NUM_DATA_BYTES)fSize = remainingSize;

        rc = cmd_read_mem(buffer, localAddress, fSize);
        if(rc < 0)
        {
            LOGERR("readMemory timeout 1!\n");
            return -1;
        }
        buffer += fSize;
        remainingSize -= fSize;
        localAddress += fSize;

        progress(length - remainingSize, length);
    }

//    return receivedUsbData.Control; /* ??? */
    return CHACO_OK;
}

int chameleon_writememory(unsigned char * buffer, int length, int address)
{
    int remainingBytes = length;
    int bufferIndex = 0;
    int rc;

    LOGVER("writing %08x bytes to %08x...\n", length, address);
    progress(0,0);

    rc = cmd_write_mem_start(address);
    if(rc < 0)
    {
        LOGERR("Receive timeout\n");
        return -1;
    }

    while(remainingBytes)
    {
        int currentFrameSize = NUM_DATA_BYTES;

        if(remainingBytes < NUM_DATA_BYTES) {
            currentFrameSize = remainingBytes;
        }
        if (cmd_write_mem(&buffer[bufferIndex], currentFrameSize) < 0) {
            return -1;
        }
        bufferIndex += currentFrameSize;
        remainingBytes -= currentFrameSize;

        progress(length - remainingBytes, length);
    }
    return cmd_write_mem_stop();
}

/* FIXME: length not divideable by 32 does not work */
int chameleon_readflash(unsigned char *buffer, unsigned int len, unsigned int addr)
{
    unsigned int bytesRead = 0;
    unsigned int bytesToRead = len;

    progress(0,0);
    if (cmd_fpga_reset() != CHACO_OK) {
        return -1;
    }
    if (cmd_set_readptr(addr) != CHACO_OK) {
        return -1;
    }

    while(bytesRead < bytesToRead)
    {
        if (cmd_flash_read(&buffer[bytesRead]) != CHACO_OK) {
            return -1;
        }
        bytesRead += 32;

        progress(bytesRead, bytesToRead);
    }
    return CHACO_OK;   
}

/*
    erase flash

    - addr must be aligned to a 64kb boundary
    - len must be a multiple of 64kb
*/
int chameleon_eraseflash(unsigned int len, unsigned int addr)
{
    int rc;
    unsigned int startSector, numSectors;
    unsigned int i;

    if ((addr & 0xffff) != 0) {
        LOGERR("chameleon_eraseflash: bad address\n");
        return -1;
    }
    if (((len & 0xffff) != 0) || ( len < 0x10000))  {
        LOGERR("chameleon_eraseflash: bad length\n");
        return -1;
    }

    startSector = addr >> 16;
    numSectors = len >> 16;

    rc = cmd_fpga_reset();
    rc = cmd_set_writeptr(addr);

    if (numSectors > 1) {
        LOGVER("Erasing %d Sectors 0x%04x-0x%04x...\n", numSectors, startSector, startSector + numSectors - 1);
    } else {
        LOGVER("Erasing %d Sector 0x%04x...\n", numSectors, startSector);
    }

    progress(0,0);
    for(i = 0; i < numSectors; i++) {
#ifndef FLASHDRY
        rc = cmd_sector_erase(startSector + i);
#endif
        if (rc < 0) {
            return -1;
        }
        progress(i, numSectors);
    }

    LOGVER("Erasing complete\n");
    return 0;

}

/*
    write to flash

    - addr must be aligned to a 64kb boundary
    - len must be a multiple of 64kb
*/
//#define FLASHDRY
int chameleon_writeflash(unsigned char *buffer, unsigned int len, unsigned int addr)
{
    int rc;
    unsigned int bytesWritten = 0;
    unsigned int maxSize = len;
    unsigned int startAddress = addr;
    unsigned int startSector, numSectors;
    unsigned int i;
    unsigned int bytesPerSlotUsed = NUM_DATA_BYTES;
    unsigned char writebuffer[NUM_DATA_BYTES];

    if ((addr & 0xffff) != 0) {
        LOGERR("chameleon_writeflash: bad address\n");
        return -1;
    }
    if (((len & 0xffff) != 0) || ( len < 0x10000))  {
        LOGERR("chameleon_writeflash: bad length\n");
        return -1;
    }

    startSector = addr >> 16;
    numSectors = len >> 16;

    rle_init(buffer, len, NUM_DATA_BYTES);

    rc = cmd_fpga_reset();
    rc = cmd_set_writeptr(addr);

    if (numSectors > 1) {
        LOGVER("Erasing %d Sectors 0x%04x-0x%04x...\n", numSectors, startSector, startSector + numSectors - 1);
    } else {
        LOGVER("Erasing %d Sector 0x%04x...\n", numSectors, startSector);
    }

    progress(0,0);
    for(i = 0; i < numSectors; i++) {
#ifndef FLASHDRY
        rc = cmd_sector_erase(startSector + i);
#endif
        if (rc < 0) {
            return -1;
        }
        progress(i, numSectors);
    }

    LOGVER("Erasing complete\n");

    LOGVER("Flashing data (addr:%08x size:%08x)...\n", startAddress, maxSize);
    progress(0,0);

    while(bytesWritten < maxSize)
    {
//        DBG("bytesWritten: %x\n" , bytesWritten);
        rc = rle_nextblock(writebuffer, &bytesPerSlotUsed);
        if(rc < 0)
        {
            LOGERR("Runlength Encoder ERROR (%d)", rc);
            break;
        }

        if (rc > 0)
        {
#ifdef FLASHDRY
            LOGVER("%08x: skip %02x bytes (0xff)\n", startAddress, rc);
#endif
            startAddress += rc;
#ifndef FLASHDRY
            cmd_set_writeptr(startAddress);
#endif
            bytesWritten += rc;
        }
        else
        {
#ifndef FLASHDRY
            if (cmd_flash_write(writebuffer) != CHACO_OK) {
                return -1;
            }
#else
            if (startAddress > 0x0e0000) {
                int i;
                LOGVER("%08x: ", startAddress);
                for (i=0;i<NUM_DATA_BYTES;i++){
                    LOGVER("%02x ", writebuffer[i]);
                }
                LOGVER("\n");
            }
#endif
            startAddress += bytesPerSlotUsed;
            bytesWritten += bytesPerSlotUsed;
        }
        progress(bytesWritten, len);
    }
    LOGVER("Flashing complete\n");

    return 0;

}

/*
    utility functions - read/write a full slot (1Mb)
*/
int chameleon_readslot(unsigned char *buffer, int slotnum)
{
    return chameleon_readflash(buffer, CHAMELEON_SLOT_SIZE, slotnum << 20);
}

int chameleon_writeslot(unsigned char *buffer, int slotnum)
{
    return chameleon_writeflash(buffer, CHAMELEON_SLOT_SIZE, slotnum << 20);
}

/*
    utility functions - read/write full flash (16Mb)
*/
int chameleon_readimage(unsigned char *buffer)
{
    return chameleon_readflash(buffer, CHAMELEON_FLASH_SIZE, 0);
}

int chameleon_writeimage(unsigned char *buffer)
{
    return chameleon_writeflash(buffer, CHAMELEON_FLASH_SIZE, 0);
}

int chameleon_getstatus(int *spiactive, int *usbcap, int *bricked, int *cfgdone, int *nstatus)
{
    unsigned char buffer[NUM_DATA_BYTES];

    if (spiactive)*spiactive = 0;
    if (usbcap)*usbcap = 0;
    if (bricked)*bricked = 0;
    if (cfgdone)*cfgdone = 0;
    if (nstatus)*nstatus = 0;

    if (cmd_reset_pointer() != CHACO_OK) {
        return -1;
    }

    if (cmd_get_status(buffer) != CHACO_OK)
    {
        LOGERR("Read from flash timeout\n");
        return -1;
    }

    if(spiactive)if(buffer[0])*spiactive = 1;
    if(usbcap)if(buffer[1]&0x01)*usbcap = 1;
    if(bricked)if(buffer[2] & 0xF0)*bricked = 1;
    if(nstatus)*nstatus = buffer[8];
    if(cfgdone)*cfgdone = buffer[9];
    return CHACO_OK;
}

static unsigned long getoffset3(unsigned char *buffer)
{
    return buffer[0] * 0x10000 +  buffer[1] * 0x100 + buffer[2];
}

static unsigned long getoffset4(unsigned char *buffer)
{
    return buffer[0] * 0x1000000 + buffer[1] * 0x10000 + buffer[2] * 0x100 + buffer[3];
}

int chameleon_getcoreinfo(int corenum, COREINFO *cinfo)
{
    unsigned char buffer[0x100];
    int rc, i;

    cmd_fpga_reset();
    rc = cmd_set_readptr(corenum << 20);
    rc = cmd_flash_read(buffer);

//    DBG("%02x %02x %02x %02x \n", buffer[0],buffer[1],buffer[2],buffer[3]);

    if(rc==-1)
    {
        LOGERR("Read from flash timeout\n");
        return -1;
    }

    cinfo->rom_offset = getoffset3(buffer);
    if ((cinfo->rom_offset >= CHAMELEON_SLOT_SIZE) || (cinfo->rom_offset < 42)) {
        return -1;
    }

    rc = cmd_set_readptr((corenum << 20) + cinfo->rom_offset - (COREINFO_INFOLENGTH_LEN + COREINFO_INFOOFFSET_LEN));
    rc = cmd_flash_read(buffer);

    i = 0;
    cinfo->info_length = getoffset4(&buffer[i]); i+= COREINFO_INFOLENGTH_LEN;
    cinfo->info_offset = getoffset4(&buffer[i]);
    if ((cinfo->info_offset >= CHAMELEON_SLOT_SIZE) || (cinfo->info_offset < 42)) {
        return -1;
    }
    if ((cinfo->info_length >= CHAMELEON_SLOT_SIZE) || (cinfo->info_length < (COREINFO_MAGIC_LEN + COREINFO_VERSION_LEN + COREINFO_INFOLENGTH_LEN + COREINFO_INFOOFFSET_LEN))) {
        return -1;
    }

    rc = cmd_set_readptr((corenum << 20) + cinfo->info_offset);
    rc = cmd_flash_read(buffer);
    rc = cmd_flash_read(&buffer[32]);

    i = 0;
    memcpy(cinfo->magic, &buffer[i], COREINFO_MAGIC_LEN); i += COREINFO_MAGIC_LEN;
    cinfo->version = getoffset4(&buffer[i]); i += COREINFO_VERSION_LEN;
    cinfo->core_length = getoffset4(&buffer[i]); i += COREINFO_CORELENGTH_LEN;
    cinfo->core_offset = getoffset4(&buffer[i]); i += COREINFO_COREOFFSET_LEN;
    cinfo->rom_length = getoffset4(&buffer[i]); i += COREINFO_ROMLENGTH_LEN;
    cinfo->rom_offset = getoffset4(&buffer[i]); i += COREINFO_ROMOFFSET_LEN;
    memcpy(cinfo->core_name, &buffer[i], COREINFO_CORENAME_LEN); i += COREINFO_CORENAME_LEN;

    if (memcmp(cinfo->magic, COREINFO_MAGIC, COREINFO_MAGIC_LEN) != 0) {
        return -1;
    }
    if ((cinfo->version == 0) || (cinfo->version > COREINFO_VERSION)) {
        return -1;
    }
    if ((cinfo->core_length >= CHAMELEON_SLOT_SIZE) || (cinfo->core_length < 42)) {
        return -1;
    }
    if ((cinfo->core_offset >= CHAMELEON_SLOT_SIZE) || (cinfo->core_offset < 3)) {
        return -1;
    }
    if (cinfo->rom_length >= CHAMELEON_SLOT_SIZE) {
        return -1;
    }
    if ((cinfo->rom_offset >= CHAMELEON_SLOT_SIZE) || (cinfo->rom_offset < 42)) {
        return -1;
    }

    return 0;
}

int chameleon_startcore(int core)
{
    int csize;

    DBG("chameleon_startcore: %d\n",core);
    csize = cmd_fpga_start(core);
    if (csize == CHACO_ERROR) {
        LOGERR("Receive timeout (chameleon_startcore)\n");
        return -1;
    }
    LOGMSG("Started Core in Slot %d (Size: %d)\n", core, csize);

    return CHACO_OK;
}

int chameleon_startbootloader(void)
{
    return cmd_start_bootloader();
}

int chameleon_setjtagslot(int slot)
{
    int rc;
    rc = cmd_set_jtagslot(slot);
    if(rc != CHACO_OK) {
        LOGERR("setjtagslot COMM ERROR %d\n", rc);
        return -1;
    }
    return CHACO_OK;
}

int chameleon_getversion(int * version, int * mmcCardPresent)
{
    unsigned char buffer[NUM_DATA_BYTES];
    int rc;

    if(version)*version = 0;
    if(mmcCardPresent)*mmcCardPresent = 0;

    rc = cmd_get_version(buffer);
    if(rc != CHACO_OK)
    {
        LOGERR("chameleon_getversion timeout\n");
        return -1;
    }
    else
    {
        if(version)*version = buffer[1];
        if(mmcCardPresent)*mmcCardPresent = buffer[0];
    }

    return CHACO_OK;
}

int chameleon_getflashid(unsigned char *manufacturer, unsigned char *flash)
{
    unsigned char buffer[NUM_DATA_BYTES];
    int rc;

    rc = cmd_fpga_reset();
    rc = cmd_get_flash_id(buffer);
    if(rc != CHACO_OK)
    {
        LOGERR("chameleon_getflashid timeout\n");
        return -1;
    }
#if 0
    printf("chameleon_getflashid got: %02x %02x %02x %02x %02x %02x %02x %02x\n",
           buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7]);
#endif
    if(manufacturer)*manufacturer = buffer[0];
    if(flash)*flash = buffer[1];
    return CHACO_OK;
}

/*
    - init libusb
    - find chameleon device on usb bus
    - claim interface
*/
int chameleon_init(void)
{
    int rc;

    // some sanity checks
    if ((SIZEOF_USBHIDDATAFRAME != sizeof(USBHIDDataFrame))) {
        LOGERR("sizeof(USBHIDDataFrame) mismatch, expected %d is %d\n", (int)SIZEOF_USBHIDDATAFRAME, (int)sizeof(USBHIDDataFrame));
    }

    rc = chameleon_usb_init();
    if (rc < 0) {
        LOGERR("USB Init failed\n");
        return -1;
    }

    rc = chameleon_find();
    if (rc < 0) {
        LOGERR("No Chameleon found on Bus\n");
        return -1;
    }

    rc = chameleon_claim();
    if (rc < 0) {
        LOGERR("could not claim interface.\n");
        return -1;
    }

    return CHACO_OK;
}

/*
    close chameleon device, release interface

    - this MUST be called before application exits!
*/
int chameleon_close(void)
{
    return chameleon_usb_close();   
}
