
#include <sstream>
#include <fstream>
#include <string.h>

#include "chacolib.h"
#include "Chaco.h"

#include "Chameleon.h"
#include "Dbg.h"
#include "BasicFile.h"

#ifndef DEBUG
/* #define DEBUG */
#endif

#ifdef DEBUG

#define LOGERR(...)       dbg::dError( __VA_ARGS__ )
#define LOGVER(...)       dbg::dVerbose( __VA_ARGS__ )
#define LOGMSG(...)       dbg::dPrint( __VA_ARGS__ )
#define DBG(...)          dbg::dDebug( __VA_ARGS__ )

#else

#define LOGERR(...)       dbg::dError( __VA_ARGS__ )
#define LOGVER(...)       dbg::dVerbose( __VA_ARGS__ )
#define LOGMSG(...)       dbg::dPrint( __VA_ARGS__ )
#define DBG(...)

#endif

/******************************************************************************/

static int bUSBCAP = false;
static int bSPIACTIVE = false;
static int bBricked = false;
static int bCfgdone = false;
static int bNstatus = false;
static int bPresent = false;

bool isUsbCap()
{
    return bUSBCAP;   
}

bool isSpiActive()
{
    return bSPIACTIVE;
}

bool isBricked()
{
      return bBricked;
}

bool isCfgdone()
{
      return bCfgdone;
}

bool isNstatus()
{
      return bNstatus;
}

bool isPresent()
{
      return bPresent;
}

/******************************************************************************/

static char *ChLastError = (char*)"unknown";
static bool bError = false;

bool GetErrorActive()
{
    return bError;
}

char *getError()
{
    return ChLastError;
}

/******************************************************************************/

static bool bAccessLock = false;

bool isLocked()
{
    return bAccessLock;
}

int LockAccess()
{
    if(bAccessLock) {
        LOGERR("LockAccess failed.\n");
        return -1;
    }

    bAccessLock = true;
    return 0;
}

void UnlockAccess()
{
    bAccessLock = false;
}

/******************************************************************************/

int ChameleonInit()
{
    int rc;
    if(LockAccess() == -1)return -1;
    bPresent = false;
    bError = false;
    rc = chameleon_init();
    if (rc < 0) {
        ChLastError = (char*)"Initializing failed.";
        bError = true;
        UnlockAccess();
        return -1;
    }
    bPresent = true;
    UnlockAccess();

    int version = 0;
    int mmcStat = 0;

    rc = CheckVersion(&version, &mmcStat);
    if (rc < 0) {
        ChLastError = (char*)"CheckVersion failed.";
        bError = true;
        return -1;
    }

    LOGMSG("Chameleon up and running...\n");
    LOGMSG("Firmware Version: %02x\n" , version);
    LOGVER("sd card detected: %s\n", mmcStat ? "no" : "yes");

    updateFlashStatus();
    return rc;
}

int ChameleonClose()
{
    return chameleon_close();
}

/*
    try to find chameleon on the usb bus

    - since this function is used to periodically check for the presence of a
      chameleon when Chaco is running, no actions should be performed that may
      disturb the running core, in particular:
      - not access the FLASH (spi bus)
      - not change RAM
*/
int CheckChameleon()
{
    if(LockAccess() == -1)return -1;
    bPresent = false;
//    DBG("CheckChameleon find\n");
    if (chameleon_find() < 0) {
        UnlockAccess();
        return -1;
    }
//    DBG("CheckChameleon claim\n");
    if (chameleon_claim() < 0) {
        UnlockAccess();
        return -1;
    }
//    DBG("CheckChameleon checkconfig\n");
    if (chameleon_checkconfig() < 0) {
    UnlockAccess();
        return -1;
    }
//    DBG("CheckChameleon ok\n");
    bPresent = true;
    UnlockAccess();
    return 0;
}

int CheckVersion(int * version, int * mmcCardPresent)
{
    int rc;
    if(LockAccess() == -1) {
        return -1;
    }
    rc = chameleon_getversion(version, mmcCardPresent);
    UnlockAccess();
    return rc;
}

int updateFlashStatus(void)
{
    int rc;
    if(LockAccess() == -1) {
        return -1;
    }
    rc = chameleon_getstatus(&bSPIACTIVE,&bUSBCAP, &bBricked, &bCfgdone, &bNstatus);
    UnlockAccess();
    return rc;
}

int setJTAGSlot(int slot)
{
    int rc;
    if(LockAccess() == -1) {
        return -1;
    }
    LOGVER("set jtag slot: %d\n", slot);
    rc =  chameleon_setjtagslot(slot);
    if (rc < 0) {
        LOGERR("setJTAGSlot (%d) failed\n", slot);
    }
    UnlockAccess();
    return rc;
}

int getSlotStatus(int * slotstatus)
{
    COREINFO cinfo;
    int rc;

    if(LockAccess() == -1) {
        return -1;
    }

    for(int i = 0; i < 16; i++) {
        slotstatus[i] = 0;
        rc = chameleon_getcoreinfo(i, &cinfo);
        if (rc == 0) {
            slotstatus[i] = 1;
        }
    }

    UnlockAccess();
    return 0;
}

int startBootloader()
{
    int rc;
    if(LockAccess() == -1) {
        return -1;
    }
    rc = chameleon_startbootloader();
    UnlockAccess();
    return rc;
}

/******************************************************************************/

/* task */
int StartCH(int * flags)
{
    int rc;
    int coreNum = *flags;

    LOGMSG("starting core (%d)\n", coreNum);

    if(LockAccess() == -1)return -1;
    rc = chameleon_startcore(coreNum);
    UnlockAccess();

    if (rc < 0) {
        LOGERR("starting core (%d) failed.\n", coreNum);
        return -1;
    }

    // get status
    if (updateFlashStatus() < 0) {
        LOGERR("getting flash status failed.\n");
    } else {
        if(!isSpiActive()) {
            LOGMSG("SPI is offline again\n");
        } else {
            LOGMSG("WARNING: SPI is still online!\n");
        }

        if(isNstatus()) {
            LOGMSG("nStatus got low while writing bytes\n");
        }

        if(!isCfgdone()) {
            LOGMSG("config done is still low!\n");
        }
    }

    return 0;
}

/* task */
int readMemory(slot_memory_info_t *info)
{
    int rc;

    if(LockAccess() == -1)return -1;
    if(!info)return -1;
    if(!info->filename)return -1;

    if ((info->filename == NULL) || 
        (info->filename->c_str() == NULL) ||
        (info->filename->c_str()[0] == 0)) {
        LOGERR("Cannot read file: invalid filename or path (non ASCII?)\n");
        UnlockAccess();
        return -1;
    }
    
    
    LOGMSG("Reading memory to file: %s\n", info->filename->c_str());
    LOGMSG("Address: %08x Size: %d\n", info->address, info->length);

    unsigned char *buffer = new unsigned char[CHAMELEON_RAM_SIZE];

    rc = chameleon_readmemory(buffer, info->length, info->address);
    if (rc < 0) {
        LOGERR("reading memory failed.\n");
        delete buffer;
        UnlockAccess();
        return -1;
    }

    std::fstream readFile;

    readFile.open(info->filename->c_str(), fstream::out | fstream::binary | fstream::trunc);
    if(!readFile.is_open()) {
        LOGERR("could not open '%s'\n", info->filename->c_str());
        delete buffer;
        UnlockAccess();
        return -1;
    }
    readFile.write((char*)buffer, info->length);
    readFile.close();

    LOGMSG("done.\n");

    delete buffer;
    UnlockAccess();
    return 0;
}

/* task */
int writeMemory(slot_memory_info_t *info)
{
    int rc;

    if(LockAccess() == -1)return -1;
    if(!info)return -1;
    if(!info->filename)return -1;

    if ((info->filename == NULL) || 
        (info->filename->c_str() == NULL) ||
        (info->filename->c_str()[0] == 0)) {
        LOGERR("Cannot write file: invalid filename or path (non ASCII?)\n");
        UnlockAccess();
        return -1;
    }
    
    LOGMSG("Writing file to memory: %s\n", info->filename->c_str());
    LOGMSG("Address: %08x Size: %d\n", info->address, info->length);

    std::fstream readFile;
    const char *filename = info->filename->c_str();

    readFile.open(info->filename->c_str(), fstream::in | fstream::binary);
    if(!readFile.is_open()) {
        LOGERR("could not open '%s'\n", filename);
        UnlockAccess();
        return -1;
    }
    unsigned char *buffer = new unsigned char[info->length];
    readFile.read((char*)buffer, info->length);
    readFile.close();

    rc = chameleon_writememory(buffer, info->length, info->address);
    if (rc < 0) {
        LOGERR("writing to memory failed.\n");
    } else {
        LOGMSG("done.\n");
    }

    delete buffer;
    UnlockAccess();
    return rc;
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

static int checksdcard(void)
{
    int version, mmcCardPresent;
    if (CheckVersion(&version, &mmcCardPresent) < 0) {
        LOGERR("CheckVersion failed\n");
        return -1;
    }
    if (!mmcCardPresent) {
        LOGERR("please remove the sd card before flashing.\n");
        return -1;
    }
    return 0;
}

/* task */
int FlashCore(core_flash_info_t * cfi)
{
    if (checksdcard() < 0) {
        return -1;
    }

    if(LockAccess() == -1)return -1;
    if(!cfi)return -1;
    if(!cfi->coreName)return -1;

    unsigned char * coreData = NULL;
    unsigned char * coreFile = NULL;
    int coreLength = 0;
    COREINFO cinfo;

    unsigned char * romData = NULL;
    int romLength = 0;
    bool bFlashRom = false;

    int rc;

    LOGMSG("Flashing Core...\n");

    /* load additional ROM file */
    if(cfi->romName) {
        
        if ((cfi->romName == NULL) || 
            (cfi->romName->c_str() == NULL) ||
            (cfi->romName->c_str()[0] == 0)) {
            LOGERR("Cannot load ROM file: invalid filename or path (non ASCII?)\n");
            UnlockAccess();
            delete cfi;
            return -1;
        }

        bFlashRom = true;

        if(loadFile(&romData, &romLength, cfi->romName) < 0) {
            LOGERR("Cannot load ROM file '%s'\n",cfi->romName->c_str());
            UnlockAccess();
            delete cfi;
            return -1;
        }
    }

    if ((cfi->coreName == NULL) || 
        (cfi->coreName->c_str() == NULL) ||
        (cfi->coreName->c_str()[0] == 0)) {
        LOGERR("Cannot load Core file: invalid filename or path (non ASCII?)\n");
        UnlockAccess();
        delete cfi;
        return -1;
    }

    if(loadFile(&coreFile, &coreLength, cfi->coreName) < 0) {
        LOGERR("Cannot load Core file '%s'\n",cfi->coreName->c_str());
        UnlockAccess();
        delete cfi;
        return -1;
    }

    /* prepare proper flash image for slot */
    coreData = new unsigned char[CHAMELEON_SLOT_SIZE];
    memset(coreData, 0xff, CHAMELEON_SLOT_SIZE);

    cinfo.core_length = coreLength;
    cinfo.rom_length = romLength;
    makename(cinfo.core_name, (char*)cfi->coreName->c_str());

    coreLength = chameleon_prepareslot(coreData, coreFile, &cinfo);

    LOGMSG("Core file: %s Size: %d\n",cfi->coreName->c_str(),coreLength);
    if(bFlashRom) {
        LOGMSG("ROM file: %s Size: %d\n",cfi->romName->c_str(),romLength);
    }
    LOGMSG("Binary Length: %d Padded: %d\n",coreLength + romLength,(coreLength + romLength + 0xffff) & ~0xffff);

    if ((romLength + coreLength) > CHAMELEON_SLOT_SIZE) {
        LOGERR("length of binary data exceeds slot size.\n");
        rc = -1;
    } else {
        if (bFlashRom) {
            memcpy(&coreData[coreLength], romData, romLength);
        }

        LOGMSG("Flashing Slot: %d\n",cfi->corenum);

        rc = chameleon_writeflash(coreData, (coreLength + romLength + 0xffff) & ~0xffff, (1<<20)*cfi->corenum);
        if (rc < 0) {
            LOGERR("writing to flash failed.\n");
        } else {
            LOGMSG("done.\n");
        }
    }

    if(coreData != NULL)delete[] coreData;
    if(coreData != NULL)delete[] coreFile;
    if(romData != NULL)delete[] romData;

    UnlockAccess();
    return rc;
}

/* task */
int WriteSlot(slot_info_t * si)
{
    int rc;
    int slotnum = si->corenum;
    unsigned char * buffer;
    int size;

    if (checksdcard() < 0) {
        return -1;
    }

    if(LockAccess() == -1)return -1;

    if ((si->filename == NULL) || 
        (si->filename->c_str() == NULL) ||
        (si->filename->c_str()[0] == 0)) {
        LOGERR("Cannot write Slot-image: invalid filename or path (non ASCII?)\n");
        UnlockAccess();
        return -1;
    }
    
    std::string * filename = si->filename;   
    LOGMSG("Flashing slot %d from %s\n", slotnum, filename->c_str());

    if(loadFile(&buffer, &size, filename) < 0) {
        LOGERR("Cannot load Slot-image file '%s'",filename->c_str());
        delete[] (buffer);
        UnlockAccess();
        return -1;
    }

    LOGMSG("Size: %d\n" , size);

    rc = chameleon_writeflash(buffer, CHAMELEON_SLOT_SIZE, (slotnum<<20));
    if (rc < 0) {
        LOGERR("writing to flash failed.\n");
    } else {
        LOGMSG("done.\n");
    }

    delete[] (buffer);
    UnlockAccess();
    return rc;
}

/* task */
int ReadSlot(slot_info_t * si)
{
    int rc;
    int slotnum = si->corenum;

    if(LockAccess() == -1)return -1;

    if ((si->filename == NULL) || 
        (si->filename->c_str() == NULL) ||
        (si->filename->c_str()[0] == 0)) {
        LOGERR("Cannot save Slot-image: invalid filename or path (non ASCII?)\n");
        UnlockAccess();
        return -1;
    }
    
    std::string * filename = si->filename;   
    LOGMSG("Reading slot %d to %s\n", slotnum, filename->c_str());

    unsigned char * buffer = new unsigned char[CHAMELEON_SLOT_SIZE];

    rc = chameleon_readflash(buffer, (1<<20), (slotnum<<20));
    if (rc < 0) {
        LOGERR("reading from flash failed.\n");
    } else {
        if(saveFile(buffer,CHAMELEON_SLOT_SIZE,filename) < 0) {
            LOGERR("Cannot save Slot-image file '%s'",filename->c_str());
            delete[] (buffer);
            UnlockAccess();
            return -1;
        }
        LOGMSG("done.\n");
    }

    delete[] (buffer);
    UnlockAccess();
    return rc;
}

/* task */
int readImage(std::string * filename)
{
    int rc;

    if(LockAccess() == -1)return -1;
    
    if ((filename == NULL) || 
        (filename->c_str() == NULL) ||
        (filename->c_str()[0] == 0)) {
        LOGERR("Cannot read Flash-image: invalid filename or path (non ASCII?)\n");
        UnlockAccess();
        return -1;
    }
    
    LOGMSG("Reading image. Outputfile: %s\n",filename->c_str());

    std::fstream readBackFile;

    readBackFile.open(filename->c_str(), fstream::out | fstream::binary | fstream::trunc);
    if(!readBackFile.is_open())
    {
        UnlockAccess();
        return -1;
    }

    unsigned char *buffer = new unsigned char [CHAMELEON_FLASH_SIZE];
    rc = chameleon_readimage(buffer);
    if (rc < 0) {
        LOGERR("reading from flash failed.\n");
        readBackFile.close();
        delete buffer;
        UnlockAccess();
        return -1;
    }

    readBackFile.write((const char*)buffer,CHAMELEON_FLASH_SIZE);
    readBackFile.close();
    LOGMSG("done.\n");

    delete buffer;
    UnlockAccess();
    return 0;
}

/* task */
int writeImage(std::string * filename)
{
    int rc;

    if (checksdcard() < 0) {
        return -1;
    }

    if(LockAccess() == -1) {
        return -1;
    }
    
    if ((filename == NULL) || 
        (filename->c_str() == NULL) ||
        (filename->c_str()[0] == 0)) {
        LOGERR("Cannot write Flash-image: invalid filename or path (non ASCII?)\n");
        UnlockAccess();
        return -1;
    }
    
    LOGMSG("Writing image. Inputfile: %s\n",filename->c_str());
    std::ifstream writeFile;

    writeFile.open(filename->c_str(), fstream::in | fstream::binary);
    if(!writeFile.is_open())
    {
        UnlockAccess();
        return -1;   
    }

    unsigned char *buffer = new unsigned char [CHAMELEON_FLASH_SIZE];
    memset(buffer, 0xff, CHAMELEON_FLASH_SIZE);
    writeFile.read((char*)buffer,CHAMELEON_FLASH_SIZE);
    writeFile.close();

    rc = chameleon_writeimage(buffer);
    if (rc < 0) {
        LOGERR("writing to flash failed.\n");
    } else {
        LOGMSG("done.\n");
    }

    delete buffer;
    UnlockAccess();
    return rc;
}




