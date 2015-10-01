
#ifndef CHACOLIB_H_
#define CHACOLIB_H_

#ifdef __CPLUSPLUS
extern "C" {
#endif

#define CHAMELEON_NUM_SLOTS     (16)
#define CHAMELEON_SLOT_SIZE     (1024 *1024)
#define CHAMELEON_FLASH_SIZE    (16 * 1024 *1024)
#define CHAMELEON_RAM_SIZE      (32 * 1024 *1024)

#define CHACO_OK                 0
#define CHACO_ERROR             -1

extern int chameleon_init(void);
extern int chameleon_close(void);
extern int chameleon_find(void);
extern int chameleon_claim(void);

extern void chameleon_setprogressfunc(unsigned int len, void (*func)(unsigned int,unsigned int));
extern void chameleon_progresshelper(unsigned int percent,unsigned int value);
extern int chameleon_getprogresspercent(void);
extern int chameleon_getprogressvalue(void);

typedef int (*chamlog_t) (int lvl, const char * format, ...);
extern void chameleon_setlogfunc(chamlog_t func);
#define LOGLVL_ERR      0
#define LOGLVL_VER      1
#define LOGLVL_MSG      2
#define LOGLVL_DBG      3

extern int chameleon_checkconfig(void);
extern int chameleon_getversion(int * version, int * mmcCardPresent);
extern int chameleon_getstatus(int *spiactive, int *usbcap, int *bricked, int *cfgdone, int *nstatus);

extern int chameleon_setjtagslot(int slot);

extern int chameleon_startcore(int core);
extern int chameleon_startbootloader(void);

extern int chameleon_readmemory(unsigned char * buffer, int length, int address);
extern int chameleon_writememory(unsigned char * buffer, int length, int address);

extern int chameleon_readflash(unsigned char *buffer, unsigned int len, unsigned int addr);
extern int chameleon_writeflash(unsigned char *buffer, unsigned int len, unsigned int addr);
extern int chameleon_eraseflash(unsigned int len, unsigned int addr);

/* helper function: read/write full 16MB flash image */
extern int chameleon_readimage(unsigned char *buffer);
extern int chameleon_writeimage(unsigned char *buffer);

#define COREINFO_MAGIC_LEN      4
#define COREINFO_VERSION_LEN    4
#define COREINFO_CORELENGTH_LEN 4
#define COREINFO_COREOFFSET_LEN 4
#define COREINFO_ROMLENGTH_LEN  4
#define COREINFO_ROMOFFSET_LEN  4
#define COREINFO_CORENAME_LEN   0x40
#define COREINFO_INFOLENGTH_LEN 4
#define COREINFO_INFOOFFSET_LEN 4

#define COREINFO_LEN (COREINFO_MAGIC_LEN + COREINFO_VERSION_LEN + COREINFO_CORELENGTH_LEN + COREINFO_COREOFFSET_LEN + COREINFO_ROMLENGTH_LEN + COREINFO_ROMOFFSET_LEN + COREINFO_CORENAME_LEN + COREINFO_INFOLENGTH_LEN + COREINFO_INFOOFFSET_LEN)

#define COREINFO_MAGIC          "ch64"
#define COREINFO_VERSION        0x00000001

#define CORENAME_MAXLEN (COREINFO_CORENAME_LEN - 1)

/* helper function: get core info */
typedef struct {
    unsigned char magic[COREINFO_MAGIC_LEN];
    unsigned long version;
    unsigned long core_length;
    unsigned long core_offset;
    unsigned long rom_length;
    unsigned long rom_offset;
    char core_name[COREINFO_CORENAME_LEN];
    /* offset and length of this info block, these two come always last so they
       can be found be seeking backwards from the rom offset */
    unsigned long info_length;
    unsigned long info_offset;
} COREINFO;

extern int chameleon_getcoreinfo(int corenum, COREINFO *cinfo);

/* helper function:  convert raw .rbf to proper slot data */
// FIXME: length of destbuffer is defined as?
extern int chameleon_prepareslot(unsigned char *destbuffer, unsigned char *corebuffer, COREINFO *cinfo);

#ifdef __CPLUSPLUS
}
#endif /* __cplusplus */

#endif /* CHACOLIB_H_ */
