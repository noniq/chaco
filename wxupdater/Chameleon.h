
#ifndef CHAMELEON_H_
#define CHAMELEON_H_

#include <iostream>
#include <string>

#include "Dbg.h"

using namespace std;


        typedef struct
        {
            string * romName;
            string * coreName;
            bool bFlashRom;
            int corenum;
        } core_flash_info_t;

        typedef struct
        {
            std::string * filename;
            int corenum;
        } slot_info_t;

        typedef struct
        {
            std::string * filename;
            int address;
            int length;
        } slot_memory_info_t;

        //Status requests
        bool isSpiActive();
        bool isUsbCap();
        bool isBricked();
        bool isCfgdone();
        bool isNstatus();

        bool isPresent();

        //Initializing 
        int ChameleonInit();
        int ChameleonClose();

        //Error Handling
        string getError();
        bool GetErrorActive();

        int setJTAGSlot(int slot);
        int CheckVersion(int * version, int * mmcCardPresent);
        int CheckChameleon();
        int updateFlashStatus(void);

        int startBootloader();

        int StartCH(int * flags); /* task */
        int FlashCore(core_flash_info_t * cfi); /* task */

        int LockAccess();
        void UnlockAccess();
        bool isLocked();
#endif

