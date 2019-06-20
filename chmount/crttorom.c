#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "chameleon.h"
#include "eapi.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;

unsigned char romimage[1024*1024];
static unsigned char *imagebase;

static FILE *f;
static int res;

static unsigned char *offset;
static unsigned char *curroffset;
static unsigned char *endoffset;

static u8 unsupported;

static u16 crttype;
static unsigned char *crtname;
static unsigned char game, exrom;

static u8 counter;
static u16 chiplength;
static u16 chipbank;
static u16 chipload;
static u16 bytesleft;
static u8 cartnum;
static u8 cartmapping;
static u8 bigslot;

static u8 crthdr[0x40+1];
static u8 chiphdr[0x10];

static int loaded;

#define MAP_LINEAR      0x00   /* load in order from crt file (urks!) */
#define MAP_NORMAL      0x01   /* generic, load to roml/romh via loadaddr */
#define MAP_8K          0x02   /* 1*8K, roml or romh */
#define MAP_ROMLH       0x03   /* 2*8K, roml and romh */
#define MAP_LINEAR_8K   0x04   /* 8k banks, bank0, bank1, bank2 etc */
#define MAP_LINEAR_16K  0x05   /* 16k banks, bank0, bank1, bank2 etc */
#define MAP_OCEAN       0x06   /* roml[0], romh[16], roml[1], romh[17] etc */
#define MAP_FUNPLAY     0x07   /* roml[0], romh[16], roml[1], romh[17] etc */
#define MAP_EASYFLASH   0x08
#define FLG_MAPPING     0x0f

#define MAP_BIGSLOT     0x80
#define FLG_BIGSLOT     0x80

typedef struct {
    unsigned char crtid;
    unsigned char menuindex;
    unsigned char regval;
    unsigned char flags;
} CRTINFO;

/* note: the menu index counts from 0 up, with all carts >64k at the end of
 *       the list (so the index can be shared for all slots */

static CRTINFO crtinfo[] =
{
    { 1,  0, CFGCRT_RR            , MAP_LINEAR_8K },     // (AR) use RR
    { 2,  1, CFGCRT_KCS           , MAP_ROMLH },
    { 3,  2, CFGCRT_FC3           , MAP_LINEAR_16K },
    { 4,  3, CFGCRT_SIMONSBASIC   , MAP_ROMLH },
    { 5, 20, CFGCRT_OCEAN         , MAP_OCEAN | MAP_BIGSLOT  },        // (>64k) romh[0]..=roml[16]...
    { 6,  4, CFGCRT_EXPERT        , MAP_8K     },
    { 7, 21, CFGCRT_FUNPLAY       , MAP_FUNPLAY | MAP_BIGSLOT },       // (>64k) romh[0]..=roml[16]... ???
    { 8,  5, CFGCRT_SUPERGAMES    , MAP_LINEAR_16K },
#ifdef TESTING
    { 9,  6, CFGCRT_RR            , MAP_LINEAR_8K },     // (Nordic Power) use RR?
#endif
    {10,  6, CFGCRT_EPYX_FASTLOAD , MAP_LINEAR_8K },     // EPYX_FASTLOAD
    {11,  7, CFGCRT_WESTERMANN    , MAP_ROMLH },         // WESTERMANN
//    {12 },                                         // REX                  pointless (eprom card)
//    {13, 0xff },                                   // FINAL_I
//    {14, 0xff },                                   // MAGIC_FORMEL
    {15, 22, CFGCRT_GS            , MAP_LINEAR_8K | MAP_BIGSLOT },         // (>64k) romh[0]..=roml[16]... ???
    {16,  8, CFGCRT_WARPSPEED     , MAP_ROMLH },
    {17, 23, CFGCRT_DINAMIC       , MAP_LINEAR_8K | MAP_BIGSLOT },    // (>64k)
    {18,  9, CFGCRT_ZAXXON        , MAP_LINEAR },
    {19, 24, CFGCRT_MAGICDESK     , MAP_LINEAR_8K | MAP_BIGSLOT },    // (>64k)
    {20, 10, CFGCRT_SSV5          , MAP_LINEAR_16K },    // SUPER_SNAPSHOT_V5
    {21, 11, CFGCRT_COMAL80       , MAP_LINEAR_16K },
//    {22 },                                         // STRUCTURED_BASIC     pointless (no image to be found anywhere)
    {23, 12, CFGCRT_ROSS          , MAP_LINEAR_16K },    // ROSS
//    {24 },                                         // DELA_EP64            pointless (eprom card)
//    {25 },                                         // DELA_EP7x8           pointless (eprom card)
//    {26 },                                         // DELA_EP256           pointless (eprom card)
//    {27 },                                         // REX_EP256            pointless (eprom card)
    {28, 13, CFGCRT_MIKROASSEMBLER, MAP_LINEAR_8K },
//    {29, 0xff },                                   // FINAL_PLUS
//    {30, 0xff },                                   // ACTION_REPLAY4
    {31, 14, CFGCRT_STARDOS       , MAP_ROMLH },
    {32, 25, CFGCRT_EASYFLASH     , MAP_EASYFLASH | MAP_BIGSLOT},     // (>64k) EASYFLASH
//    {33 },                                         // EASYFLASH_XBANK      pointless (does not exist)
    {34, 15, CFGCRT_CAPTURE       , MAP_LINEAR_8K },
//    {35, 0xff },                                   // ACTION_REPLAY3
    {36, 16, CFGCRT_RR            , MAP_LINEAR_8K },
//    {37, 0xff, MAP_LINEAR_8K },                    // MMC64                FIXME: special case (handled seperately)
//    {38 },                                         // MMC_REPLAY           mostly pointless
//    {39 },                                         // IDE64                pointless (IDE interface)
//    {40, 0xff },                                   // SUPER_SNAPSHOT
//    {41 },                                         // IEEE488              pointless (IEEE interface)
//    {42, 0xff },                                   // GAME_KILLER
    {43, 26, CFGCRT_PROPHET64     , MAP_LINEAR | MAP_BIGSLOT },    // (>64k)
//    {44, 0xff, MAP_LINEAR_8K },                    // EXOS                   special case (external kernal)
//    {45, 0xff },                                   // FREEZE_FRAME
//    {46, 0xff },                                   // FREEZE_MACHINE
//    {47, 0xff },                                   // SNAPSHOT64
//    {48, 0xff },                                   // SUPER_EXPLODE_V5
//    {49, 0xff },                                   // MAGIC_VOICE
//    {50, 0xff },                                   // ACTION_REPLAY2
    {51, 17, CFGCRT_MACH5         , MAP_LINEAR_8K },     // MACH5
//    {52, 0xff },                                   // CARTRIDGE_DIASHOW_MAKER
    {53, 18, CFGCRT_PAGEFOX       , MAP_LINEAR },        // CARTRIDGE_PAGEFOX
    {54, 19, CFGCRT_BUSINESSBASIC , MAP_LINEAR },
    {0, 0, 0, 0}
};


static unsigned char checktype(unsigned char crtid)
{
    unsigned char i = 0;
    while (crtinfo[i].crtid != 0) {
        if (crtinfo[i].crtid == crtid) {
            return i;
        }
        ++i;
    }
    return 0xff;
}

static unsigned char readchip(void)
{
    res = fread(chiphdr, 1, 0x10, f);
    if (res != 0x10) {
        return 0;
    }

    chipbank = (((u16)chiphdr[0x0a]) << 8) + chiphdr[0x0b];      /* big endian */
    chipload = (((u16)chiphdr[0x0c]) << 8) + chiphdr[0x0d];      /* big endian */
    chiplength = (((u16)chiphdr[0x0e]) << 8) + chiphdr[0x0f];    /* big endian */

    switch (cartmapping) {
        case MAP_NORMAL:
            if (chipload <= 0x9fff) {
                offset = imagebase;
            } else {
                offset = imagebase + 0x2000L;
            }
            break;
        case MAP_EASYFLASH:
            offset = imagebase + (0x4000L * chipbank);
            if (chipload != 0x8000) {
                offset += 0x2000L;
            }
            break;
        case MAP_FUNPLAY:
            chipbank = ((chipbank >> 3) & 7) | ((chipbank & 1) << 3);
            offset = imagebase + (0x4000L * chipbank);
            break;
        case MAP_LINEAR_16K:
        case MAP_OCEAN:
            offset = imagebase + (0x4000L * chipbank);
            break;
        case MAP_LINEAR_8K:
            offset = imagebase + (0x2000L * chipbank);
            break;
        case MAP_8K:
            offset = imagebase;
            break;
        case MAP_LINEAR:
        case MAP_ROMLH:
            break;
    }

    curroffset = offset;
    bytesleft = chiplength;
    while (bytesleft) {
        if (bytesleft >= 0x2000) {
            res = fread(curroffset, 1, 0x2000, f);
            curroffset += 0x2000;
            bytesleft -= 0x2000;
            if (curroffset > endoffset) {
                endoffset = curroffset;
            }
        } else {
            res = fread(curroffset, 1, bytesleft, f);
            if ((curroffset + bytesleft) > endoffset) {
                endoffset = (curroffset + bytesleft);
            }
            bytesleft = 0;
        }
        ++counter;
    }

    /* make 8k chunk from 4k chip */
    if (chiplength == 0x1000) {
        memcpy(offset + 0x1000L, offset, 0x1000);
        chiplength = 0x2000;
    }

    switch (cartmapping) {
        case MAP_OCEAN:
        case MAP_FUNPLAY:
            // copy ROML to ROMH
            memcpy(offset + 0x2000L, offset, 0x2000);
            break;
        case MAP_ROMLH:
        case MAP_LINEAR:
            offset += chiplength;
            break;
        case MAP_EASYFLASH:
        case MAP_NORMAL:
        case MAP_8K:
        case MAP_LINEAR_8K:
        case MAP_LINEAR_16K:
            break;
    }

    return 1;
}

int loadcrt(char *name)
{
    loaded = 0;

    f = fopen(name, "rb");
    if (f) {

        /* get CRT header */
        res = fread(crthdr, 1, 0x40, f);
        if (res != 0x40) {
            fprintf(stderr, "could not read crt header\n");
            res = -1;
        } else {
            crthdr[0x40] = 0;
            crttype = (((u16)crthdr[0x16]) << 8) + crthdr[0x17];
            crtname = &crthdr[0x20];
            game = crthdr[0x18];
            exrom = crthdr[0x19];

            bigslot = unsupported = 0;
            cartmapping = MAP_NORMAL;
            if (crttype != 0) {
                cartnum = checktype(crttype);
                if (cartnum == 0xff) {
                    unsupported = 1;
                } else {
                    cartmapping = (crtinfo[cartnum].flags & FLG_MAPPING);
                    bigslot = (crtinfo[cartnum].flags & FLG_BIGSLOT);
                }
            }
            imagebase = &romimage[0];

            if (unsupported) {
                fprintf(stderr, "crt type %d is not supported\n", crttype);
                res = -1;
            } else {

                    /* clear cartridge image */
                    if (crttype == 32) {
                        /* easyflash */
                        memset(imagebase, 0xff, 0x4000L * 64);
                    }
                    /* load cartridge image */
                    offset = endoffset = imagebase;
                    counter = 0;
                    while (1) {
                        res = readchip();
//                        printf("loaded: %x %d\n",loaded,res);
                        if (res == 0) {
                            break;
                        }
//                        loaded += chiplength;
                    }
                    /* patch cartridge image */
                    if (crttype == 32) {
                        /* easyflash */
                        // check EAPI signature
                        if ((romimage[0x3800] == 0x65) &&
                            (romimage[0x3801] == 0x61) &&
                            (romimage[0x3802] == 0x70) &&
                            (romimage[0x3803] == 0x69)) {
                            // copy EAPI replacement
                            memcpy(romimage, eapidata, 0x0300);
                        }
                    }
                    res = endoffset - imagebase;
                    fclose (f);
            }
        }
    } else {
        fprintf(stderr, "error: could not open '%s'\n", name);
        exit(-1);
    }

    return res;
}


