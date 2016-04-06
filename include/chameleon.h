
#ifndef CHAMELEON_H_
#define CHAMELEON_H_

/*******************************************************************************
    Chameleon Register map
*******************************************************************************/

/*
    0xd040 - 0xd05f     VGA Controller
*/

#define VGAMODE         0xd040 /* FIXME: VGAMOD in progman */
#define VGACTRL         0xd041 /* FIXME: VGACFG in progman */

#define VGACOL          0xd042 /* FIXME: obsolet */

#define VGARQT          0xd043
#define VGAW            0xd044
#define VGAH            0xd045
#define VGAHWH          0xd046
#define VGAHZ           0xd047

#define VGAMODE_800_600_72      0
#define VGAMODE_800_600_50      1
#define VGAMODE_800_600_60      2
#define VGAMODE_800_600_75      3
#define VGAMODE_640_480_50      4
#define VGAMODE_640_480_60      5
#define VGAMODE_640_480_72      6
#define VGAMODE_640_480_75      7
#define VGAMODE_640_480_85      8
#define VGAMODE_1024_768_50     9
#define VGAMODE_1024_768_60     10
#define VGAMODE_1024_768_72     11
#define VGAMODE_1024_768_75     12
#define VGAMODE_1024_768_85     13
#define VGAMODE_1152_864_50     14
#define VGAMODE_1152_864_60     15
#define VGAMODE_1152_864_72     16
#define VGAMODE_1152_864_75     17
#define VGAMODE_1152_864_85     18
#define VGAMODE_1280_1024_50    19
#define VGAMODE_1280_1024_60    20
#define VGAMODE_1280_1024_72    21
#define VGAMODE_1280_1024_75    22
#define VGAMODE_1280_1024_85    23
#define VGAMODE_1600_1200_50    24
#define VGAMODE_1600_1200_60    25

#define VGAMODE_DEFAULT          0
#define VGAMODE_NUMMODES         0x1a /* FIXME: obsolet */

#define VGACTRL_BUFFER_SHIFT     0
#define VGACTRL_FILTER_SHIFT     2
#define VGACTRL_SCANLINES_SHIFT  5
#define VGACTRL_VICSYNC_SHIFT    7

#define VGACTRL_BUFFER_MASK      (3 << VGACTRL_BUFFER_SHIFT)
#define VGACTRL_FILTER_MASK      (7 << VGACTRL_FILTER_SHIFT)
#define VGACTRL_SCANLINES_MASK   (3 << VGACTRL_SCANLINES_SHIFT)
#define VGACTRL_VICSYNC_MASK     (1 << VGACTRL_VICSYNC_SHIFT)

/* bits 1,0 :  00 = no buffering, 01 = double buffer, 10 = triple buffer
  (last two frames averaged) */
#define VGACTRL_SINGLEBUFFER    0x00
#define VGACTRL_DOUBLEBUFFER    0x01
#define VGACTRL_TRIPLEBUFFER    0x02
/* bits 4,3,2 :  000 = nearest neighbor scaling. 001 = scale2x, 010 = alien */
#define VGACTRL_FILTERNEAREST   0x00
#define VGACTRL_FILTERSCALE2X   0x04
#define VGACTRL_FILTERALIEN     0x08
/* bit 6, 5 : scanlines have 00 = full brightness,  01 = 75% brightness
   10 = 50% brightness 11 = 25% brightness */
#define VGACTRL_SCANLINES_OFF   0x00
#define VGACTRL_SCANLINES_75P   0x20
#define VGACTRL_SCANLINES_50P   0x40
#define VGACTRL_SCANLINES_25P   0x60
/* bit 7 : 0 = async VGA,  1 = synchronized 50/60 Hz mode */
#define VGACTRL_VICSYNC         0x80

/*
    0xd0a0 - 0xd0af     MMU, Timers, extra config registers
*/

#define MMUADDR0        0xd0a0
#define MMUADDR1        0xd0a1
#define MMUADDR2        0xd0a2
#define MMUADDR3        0xd0a3

#define VERIDX          0xd0a8 /* core version info index */
#define VERDAT          0xd0a9 /* core version info data */

#define TIMER1          0xd0aa /* (1ms per tick)   delay 1ms */
#define TIMER2          0xd0ab /* (10ms per tick)  keyboard delay */
#define TIMER3          0xd0ac /* (10ms per tick)  spi timeouts, delay 10ms */
#define TIMER4          0xd0ad /* (100ms per tick) quicksearch, delay 100ms */

#define LASTBTN         0xd0ae

#define MMUSLOT         0xd0af

/*
000 Nothing or unknown
010 Left button pressed short
011 Left button pressed long
100 Middle (freezer) button pressed short
101 Middle (freezer) button pressed long
110 Right (reset) button pressed short
111 Right (reset) button pressed long
*/
#define LASTBTN_MASK            0x07
#define LASTBTN_NONE            0x00
#define LASTBTN_LEFT_SHORT      0x02
#define LASTBTN_LEFT_LONG       0x03
#define LASTBTN_MIDDLE_SHORT    0x04
#define LASTBTN_MIDDLE_LONG     0x05
#define LASTBTN_RIGHT_SHORT     0x06
#define LASTBTN_RIGHT_LONG      0x07

#define MMUSLOT_C64RAM0         0x00
#define MMUSLOT_C64RAM1         0x01
#define MMUSLOT_C64RAM2         0x02
#define MMUSLOT_C64RAM3         0x03
#define MMUSLOT_C64RAM4         0x04
#define MMUSLOT_C64RAM5         0x05
#define MMUSLOT_C64RAM6         0x06
#define MMUSLOT_C64RAM7         0x07
#define MMUSLOT_C64RAM8         0x08
#define MMUSLOT_C64RAM9         0x09
#define MMUSLOT_C64RAMA         0x0a
#define MMUSLOT_C64RAMB         0x0b
#define MMUSLOT_C64RAMC         0x0c
#define MMUSLOT_C64RAMD         0x0d
#define MMUSLOT_C64RAME         0x0e
#define MMUSLOT_C64RAMF         0x0f

#define MMUSLOT_REU             0x10
#define MMUSLOT_GEORAM          0x11

#define MMUSLOT_CARTRAM         0x12    /* cartridge RAM */
#define MMUSLOT_CARTROM         0x13    /* cartridge ROM */

#define MMUSLOT_MMC64ROM        0x14

#define MMUSLOT_RESERVED15      0x15
#define MMUSLOT_RESERVED16      0x16
#define MMUSLOT_RESERVED17      0x17

#define MMUSLOT_DRIVE0          0x18
#define MMUSLOT_DRIVE1          0x19

#define MMUSLOT_FRAMEBUFFER     0x1c

#define MMUSLOT_C64CHARGEN      0x1d
#define MMUSLOT_C64BASIC        0x1e
#define MMUSLOT_C64KERNAL       0x1f

#define MMUSLOT_MENU0           0x20
#define MMUSLOT_MENU2           0x21
#define MMUSLOT_MENU4           0x22
#define MMUSLOT_MENU6           0x23
#define MMUSLOT_MENU8           0x24
#define MMUSLOT_MENUA           0x25
#define MMUSLOT_MENUE           0x26
#define MMUSLOT_MENUD700        0x27

#define MMUSLOT_DRIVE0IMAGE0    0x28
#define MMUSLOT_DRIVE0IMAGE1    0x29
#define MMUSLOT_DRIVE0IMAGE2    0x2a
#define MMUSLOT_DRIVE0IMAGE3    0x2b

#define MMUSLOT_DRIVE1IMAGE0    0x2c
#define MMUSLOT_DRIVE1IMAGE1    0x2d
#define MMUSLOT_DRIVE1IMAGE2    0x2e
#define MMUSLOT_DRIVE1IMAGE3    0x2f


/*******************************************************************************
    0xd0f0-0xd0ff       config registers
*******************************************************************************/

#define CFGCRT  0xd0f0  /* cartridge emulation */
#define CFGSPI  0xd0f1  /* clockport and spi emulation */
#define CFGVIC  0xd0f2  /* vic-ii emulation config */
#define CFGTUR  0xd0f3  /* turbo config */
#define CFGSID  0xd0f4  /* SID config */
#define CFGREU  0xd0f5  /* REU config */
#define CFGDWR  0xd0f6  /* disk images write flags */
#define CFGDSK  0xd0f7  /* disk images */
#define CFGFD0  0xd0f8  /* drive emulation */
#define CFGFD1  0xd0f9  /* drive emulation */
#define CFGREG  0xd0fa  /* enable chameleon registers */
#define CFGBTN  0xd0fb  /* button config */
#define CFGIO   0xd0fc  /* i/o and cia emulation */
#define CFGDIS  0xd0fd  /* disable config mode */
#define CFGENA  0xd0fe  /* enable config mode */
#define CFGRTI  0xd0ff  /* disable config mode */

/* cartridge emulation */
#define CFGCRT_NONE                     0x00
#define CFGCRT_RR                       0x01 /* crt id 1: AR 9: Nordic 36: RR */
#define CFGCRT_KCS                      0x02 /* crt id 2 */
#define CFGCRT_FC3                      0x03 /* crt id 3 */
#define CFGCRT_SIMONSBASIC              0x04 /* crt id 4 */
#define CFGCRT_OCEAN                    0x05 /* crt id 5 */
#define CFGCRT_EXPERT                   0x06 /* crt id 6 */
#define CFGCRT_FUNPLAY                  0x07 /* crt id 7 */
#define CFGCRT_SUPERGAMES               0x08 /* crt id 8 */
#define CFGCRT_EPYX_FASTLOAD            0x0a /* crt id 10 */
#define CFGCRT_WESTERMANN               0x0b /* crt id 11 */
#define CFGCRT_GS                       0x0f /* crt id 15 */
#define CFGCRT_WARPSPEED                0x10 /* crt id 16 */
#define CFGCRT_DINAMIC                  0x11 /* crt id 17 */
#define CFGCRT_ZAXXON                   0x12 /* crt id 18 */
#define CFGCRT_MAGICDESK                0x13 /* crt id 19 */
#define CFGCRT_SSV5                     0x14 /* crt id 20 */
#define CFGCRT_COMAL80                  0x15 /* crt id 21 */
#define CFGCRT_ROSS                     0x17 /* crt id 23 */
#define CFGCRT_MIKROASSEMBLER           0x1c /* crt id 28 */
#define CFGCRT_STARDOS                  0x1f /* crt id 31 */
#define CFGCRT_EASYFLASH                0x20 /* crt id 32 */
#define CFGCRT_CAPTURE                  0x22 /* crt id 34 */
#define CFGCRT_PROPHET64                0x2b /* crt id 43 */
#define CFGCRT_MACH5                    0x33 /* crt id 51 */
#define CFGCRT_PAGEFOX                  0x35 /* crt id 53 */
#define CFGCRT_BUSINESSBASIC            0x36 /* crt id 54 */

#define CFGCRT_16K_GAME                 0xfc /* crt id 0 */
#define CFGCRT_16K_ULTIMAX              0xfd /* crt id 0 */
#define CFGCRT_8K_GAME                  0xfe /* crt id 0 */

/* clockport and spi emulation */
#define CFGSPI_CPNMI_SHIFT              (6)
#define CFGSPI_CP_SHIFT                 (4)
#define CFGSPI_ROM_SHIFT                (3)
#define CFGSPI_MMC64_SHIFT              (2)
#define CFGSPI_SPI_SHIFT                (0)

#define CFGSPI_CPNMI_MASK               (0x40)
#define CFGSPI_CP_MASK                  (0x30)
#define CFGSPI_ROM_MASK                 (0x08)
#define CFGSPI_MMC64_MASK               (0x04)
#define CFGSPI_SPI_MASK                 (0x03)

#define CFGSPI_CPNMI_DISABLED           (0 << CFGSPI_CPNMI_SHIFT)
#define CFGSPI_CPNMI_ENABLED            (1 << CFGSPI_CPNMI_SHIFT)
#define CFGSPI_CP_OFF                   (0 << CFGSPI_CP_SHIFT)
#define CFGSPI_CP_DE00                  (1 << CFGSPI_CP_SHIFT)
#define CFGSPI_CP_DF20                  (2 << CFGSPI_CP_SHIFT)
#define CFGSPI_ROM_MMU                  (0 << CFGSPI_ROM_SHIFT)
#define CFGSPI_ROM_EXTERN               (1 << CFGSPI_ROM_SHIFT)
#define CFGSPI_MMC64_ENABLED            (0 << CFGSPI_MMC64_SHIFT)
#define CFGSPI_MMC64_DISABLED           (1 << CFGSPI_MMC64_SHIFT)
#define CFGSPI_SPI_OFF                  (0 << CFGSPI_SPI_SHIFT)
#define CFGSPI_SPI_MMC64                (1 << CFGSPI_SPI_SHIFT)
#define CFGSPI_SPI_EXTENDED             (3 << CFGSPI_SPI_SHIFT)

/* vic-ii emulation config */
#define CFGVIC_READ_SHIFT               (7)
#define CFGVIC_FB_SHIFT                 (6)
#define CFGVIC_BORDER_SHIFT             (4)
#define CFGVIC_TYPE_SHIFT               (0)

#define CFGVIC_TYPE_MASK                (0x07)

#define CFGVIC_READ_ENABLE              (1 << CFGVIC_READ_SHIFT)
#define CFGVIC_FB_ENABLE                (1 << CFGVIC_FB_SHIFT)
#define CFGVIC_BORDER_OPEN              (1 << CFGVIC_BORDER_SHIFT)

#define CFGVIC_TYPE_PAL                 (0 << CFGVIC_TYPE_SHIFT)
#define CFGVIC_TYPE_NTSC                (2 << CFGVIC_TYPE_SHIFT)

/* turbo config */

#define CFGTUR_ENABLE_SHIFT             (7)
#define CFGTUR_SWITCH_SHIFT             (6)
#define CFGTUR_VICBIT_SHIFT             (5)
#define CFGTUR_AUTOSPD_SHIFT            (4)
#define CFGTUR_LIMIT_SHIFT              (0)

#define CFGTUR_LIMIT_MASK               (0x0f << CFGTUR_LIMIT_SHIFT)

#define CFGTUR_ENABLE                   (1 << CFGTUR_ENABLE_SHIFT)
#define CFGTUR_SWITCH                   (1 << CFGTUR_SWITCH_SHIFT)
#define CFGTUR_VICBIT                   (1 << CFGTUR_VICBIT_SHIFT)
#define CFGTUR_AUTOSPD_OFF              (1 << CFGTUR_AUTOSPD_SHIFT)
#define CFGTUR_LIMIT_NONE               (0x00 << CFGTUR_LIMIT_SHIFT)
#define CFGTUR_LIMIT_2MHZ               (0x01 << CFGTUR_LIMIT_SHIFT)
#define CFGTUR_LIMIT_3MHZ               (0x02 << CFGTUR_LIMIT_SHIFT)
#define CFGTUR_LIMIT_4MHZ               (0x03 << CFGTUR_LIMIT_SHIFT)
#define CFGTUR_LIMIT_5MHZ               (0x04 << CFGTUR_LIMIT_SHIFT)
#define CFGTUR_LIMIT_6MHZ               (0x05 << CFGTUR_LIMIT_SHIFT)
#define CFGTUR_LIMIT_1MHZ               (0x0c << CFGTUR_LIMIT_SHIFT)
#define CFGTUR_LIMIT_750KHZ             (0x0d << CFGTUR_LIMIT_SHIFT)
#define CFGTUR_LIMIT_500KHZ             (0x0e << CFGTUR_LIMIT_SHIFT)
#define CFGTUR_LIMIT_250KHZ             (0x0f << CFGTUR_LIMIT_SHIFT)

/* SID config */
#define CFGSID_TYPE1_SHIFT              (7)
#define CFGSID_TYPE0_SHIFT              (6)
#define CFGSID_REAL_SHIFT               (3)
#define CFGSID_EMU_SHIFT                (0)

#define CFGSID_TYPE1_MASK               (1 << CFGSID_TYPE1_SHIFT)
#define CFGSID_TYPE0_MASK               (1 << CFGSID_TYPE0_SHIFT)
#define CFGSID_REAL_MASK                (7 << CFGSID_REAL_SHIFT)
#define CFGSID_EMU_MASK                 (7 << CFGSID_EMU_SHIFT)

#define CFGSID_TYPE1_6581               (0 << CFGSID_TYPE1_SHIFT)
#define CFGSID_TYPE1_8580               (1 << CFGSID_TYPE1_SHIFT)
#define CFGSID_TYPE0_6581               (0 << CFGSID_TYPE0_SHIFT)
#define CFGSID_TYPE0_8580               (1 << CFGSID_TYPE0_SHIFT)
#define CFGSID_REAL_MONO                (0 << CFGSID_REAL_SHIFT)
#define CFGSID_REAL_STEREO_D4           (1 << CFGSID_REAL_SHIFT)
#define CFGSID_REAL_STEREO_D5           (4 << CFGSID_REAL_SHIFT)
#define CFGSID_REAL_STEREO_D7           (5 << CFGSID_REAL_SHIFT)
#define CFGSID_REAL_STEREO_DE           (6 << CFGSID_REAL_SHIFT)
#define CFGSID_REAL_STEREO_DF           (7 << CFGSID_REAL_SHIFT)
#define CFGSID_EMU_MONO                 (0 << CFGSID_EMU_SHIFT)
#define CFGSID_EMU_STEREO_D4            (1 << CFGSID_EMU_SHIFT) /* actually D4xx, see progman */
#define CFGSID_EMU_STEREO_D5            (4 << CFGSID_EMU_SHIFT) /* same as _D7, see progman */
#define CFGSID_EMU_STEREO_D7            (5 << CFGSID_EMU_SHIFT) /* same as _D5, see progman */
#define CFGSID_EMU_STEREO_DE            (6 << CFGSID_EMU_SHIFT)
#define CFGSID_EMU_STEREO_DF            (7 << CFGSID_EMU_SHIFT)

/* REU config */
#define CFGREU_REU_ENABLE_SHIFT         (7)
#define CFGREU_GEORAM_ENABLE_SHIFT      (6)
#define CFGREU_GEORAM_SIZE_SHIFT        (3)
#define CFGREU_REU_SIZE_SHIFT           (0)

#define CFGREU_REU_ENABLE               (1 << CFGREU_REU_ENABLE_SHIFT)
#define CFGREU_GEORAM_ENABLE            (1 << CFGREU_GEORAM_ENABLE_SHIFT)
#define CFGREU_GEORAM_64K               (0 << CFGREU_GEORAM_SIZE_SHIFT)
#define CFGREU_GEORAM_128K              (1 << CFGREU_GEORAM_SIZE_SHIFT)
#define CFGREU_GEORAM_256K              (2 << CFGREU_GEORAM_SIZE_SHIFT)
#define CFGREU_GEORAM_512K              (3 << CFGREU_GEORAM_SIZE_SHIFT)
#define CFGREU_GEORAM_1M                (4 << CFGREU_GEORAM_SIZE_SHIFT)
#define CFGREU_GEORAM_2M                (5 << CFGREU_GEORAM_SIZE_SHIFT)
#define CFGREU_GEORAM_4M                (6 << CFGREU_GEORAM_SIZE_SHIFT)
#define CFGREU_REU_128K                 (0 << CFGREU_REU_SIZE_SHIFT)
#define CFGREU_REU_256K                 (1 << CFGREU_REU_SIZE_SHIFT)
#define CFGREU_REU_512K                 (2 << CFGREU_REU_SIZE_SHIFT)
#define CFGREU_REU_1M                   (3 << CFGREU_REU_SIZE_SHIFT)
#define CFGREU_REU_2M                   (4 << CFGREU_REU_SIZE_SHIFT)
#define CFGREU_REU_4M                   (5 << CFGREU_REU_SIZE_SHIFT)
#define CFGREU_REU_8M                   (6 << CFGREU_REU_SIZE_SHIFT)
#define CFGREU_REU_16M                  (7 << CFGREU_REU_SIZE_SHIFT)

/* disk image write flags */
#define CFGDWR_FD1_IMG3_SHIFT           (7)
#define CFGDWR_FD1_IMG2_SHIFT           (6)
#define CFGDWR_FD1_IMG1_SHIFT           (5)
#define CFGDWR_FD1_IMG0_SHIFT           (4)
#define CFGDWR_FD0_IMG3_SHIFT           (3)
#define CFGDWR_FD0_IMG2_SHIFT           (2)
#define CFGDWR_FD0_IMG1_SHIFT           (1)
#define CFGDWR_FD0_IMG0_SHIFT           (0)

#define CFGDWR_FD1_IMG3                 (1 << CFGDWR_FD1_IMG3_SHIFT)
#define CFGDWR_FD1_IMG2                 (1 << CFGDWR_FD1_IMG2_SHIFT)
#define CFGDWR_FD1_IMG1                 (1 << CFGDWR_FD1_IMG1_SHIFT)
#define CFGDWR_FD1_IMG0                 (1 << CFGDWR_FD1_IMG0_SHIFT)
#define CFGDWR_FD0_IMG3                 (1 << CFGDWR_FD0_IMG3_SHIFT)
#define CFGDWR_FD0_IMG2                 (1 << CFGDWR_FD0_IMG2_SHIFT)
#define CFGDWR_FD0_IMG1                 (1 << CFGDWR_FD0_IMG1_SHIFT)
#define CFGDWR_FD0_IMG0                 (1 << CFGDWR_FD0_IMG0_SHIFT)

/* drive disk slots */
#define CFGDSK_FD1_NUMIMAGES_SHIFT      (6)
#define CFGDSK_FD1_ACTIVEIMG_SHIFT      (4)
#define CFGDSK_FD0_NUMIMAGES_SHIFT      (2)
#define CFGDSK_FD0_ACTIVEIMG_SHIFT      (0)

#define CFGDSK_FD1_NUMIMAGES_MASK       (3 << CFGDSK_FD1_NUMIMAGES_SHIFT)
#define CFGDSK_FD1_ACTIVEIMG_MASK       (3 << CFGDSK_FD1_ACTIVEIMG_SHIFT)
#define CFGDSK_FD0_NUMIMAGES_MASK       (3 << CFGDSK_FD0_NUMIMAGES_SHIFT)
#define CFGDSK_FD0_ACTIVEIMG_MASK       (3 << CFGDSK_FD0_ACTIVEIMG_SHIFT)

/* drive emulation */
#define CFGFD0_ENABLE_CPU_RUNNING_SHIFT (6)
#define CFGFD0_DRIVE_DOOR_OPEN_SHIFT    (5)
#define CFGFD0_WRITE_PROTECT_SHIFT      (4)
#define CFGFD0_MEMSIZE_SHIFT            (2)
#define CFGFD0_DRIVE_ID_SHIFT           (0)

#define CFGFD0_ENABLE_CPU_MASK          (1 << CFGFD0_ENABLE_CPU_RUNNING_SHIFT)
#define CFGFD0_DRIVE_DOOR_MASK          (1 << CFGFD0_DRIVE_DOOR_OPEN_SHIFT)
#define CFGFD0_WRITE_PROTECT_MASK       (1 << CFGFD0_WRITE_PROTECT_SHIFT)
#define CFGFD0_MEMSIZE_MASK             (1 << CFGFD0_MEMSIZE_SHIFT)
#define CFGFD0_DRIVE_ID_MASK            (3 << CFGFD0_DRIVE_ID_SHIFT)

#define CFGFD0_ENABLE_CPU_RUNNING       (1 << CFGFD0_ENABLE_CPU_RUNNING_SHIFT)
#define CFGFD0_ENABLE_CPU_STOPPED       (0 << CFGFD0_ENABLE_CPU_RUNNING_SHIFT)
#define CFGFD0_DRIVE_DOOR_OPEN          (1 << CFGFD0_DRIVE_DOOR_OPEN_SHIFT)
#define CFGFD0_DRIVE_DOOR_CLOSED        (0 << CFGFD0_DRIVE_DOOR_OPEN_SHIFT)
#define CFGFD0_WRITE_PROTECT_ON         (1 << CFGFD0_WRITE_PROTECT_SHIFT)
#define CFGFD0_WRITE_PROTECT_OFF        (0 << CFGFD0_WRITE_PROTECT_SHIFT)
#define CFGFD0_MEMSIZE_2K               (0 << CFGFD0_MEMSIZE_SHIFT)
#define CFGFD0_MEMSIZE_8K               (1 << CFGFD0_MEMSIZE_SHIFT)
#define CFGFD0_DRIVE_ID8                (0 << CFGFD0_DRIVE_ID_SHIFT)
#define CFGFD0_DRIVE_ID9                (1 << CFGFD0_DRIVE_ID_SHIFT)
#define CFGFD0_DRIVE_ID10               (2 << CFGFD0_DRIVE_ID_SHIFT)
#define CFGFD0_DRIVE_ID11               (3 << CFGFD0_DRIVE_ID_SHIFT)

#define CFGFD1_ENABLE_CPU_RUNNING_SHIFT (6)
#define CFGFD1_DRIVE_DOOR_OPEN_SHIFT    (5)
#define CFGFD1_WRITE_PROTECT_SHIFT      (4)
#define CFGFD1_MEMSIZE_SHIFT            (2)
#define CFGFD1_DRIVE_ID_SHIFT           (0)

#define CFGFD1_ENABLE_CPU_MASK          (1 << CFGFD1_ENABLE_CPU_RUNNING_SHIFT)
#define CFGFD1_DRIVE_DOOR_MASK          (1 << CFGFD1_DRIVE_DOOR_OPEN_SHIFT)
#define CFGFD1_WRITE_PROTECT_MASK       (1 << CFGFD1_WRITE_PROTECT_SHIFT)
#define CFGFD1_MEMSIZE_MASK             (1 << CFGFD1_MEMSIZE_SHIFT)
#define CFGFD1_DRIVE_ID_MASK            (3 << CFGFD1_DRIVE_ID_SHIFT)

#define CFGFD1_ENABLE_CPU_RUNNING       (1 << CFGFD1_ENABLE_CPU_RUNNING_SHIFT)
#define CFGFD1_ENABLE_CPU_STOPPED       (0 << CFGFD1_ENABLE_CPU_RUNNING_SHIFT)
#define CFGFD1_DRIVE_DOOR_OPEN          (1 << CFGFD1_DRIVE_DOOR_OPEN_SHIFT)
#define CFGFD1_DRIVE_DOOR_CLOSED        (0 << CFGFD1_DRIVE_DOOR_OPEN_SHIFT)
#define CFGFD1_WRITE_PROTECT_ON         (1 << CFGFD1_WRITE_PROTECT_SHIFT)
#define CFGFD1_WRITE_PROTECT_OFF        (0 << CFGFD1_WRITE_PROTECT_SHIFT)
#define CFGFD1_MEMSIZE_2K               (0 << CFGFD1_MEMSIZE_SHIFT)
#define CFGFD1_MEMSIZE_8K               (1 << CFGFD1_MEMSIZE_SHIFT)
#define CFGFD1_DRIVE_ID8                (0 << CFGFD1_DRIVE_ID_SHIFT)
#define CFGFD1_DRIVE_ID9                (1 << CFGFD1_DRIVE_ID_SHIFT)
#define CFGFD1_DRIVE_ID10               (2 << CFGFD1_DRIVE_ID_SHIFT)
#define CFGFD1_DRIVE_ID11               (3 << CFGFD1_DRIVE_ID_SHIFT)

/* enable chameleon registers */
#define CFGREG_IOROM_SHIFT              (5)
#define CFGREG_PALETTE_SHIFT            (3)
#define CFGREG_MMU_SHIFT                (1)
#define CFGREG_VGA_SHIFT                (0)

#define CFGREG_IOROM_ENABLE             (1 << CFGREG_IOROM_SHIFT)
#define CFGREG_PALETTE_ENABLE           (1 << CFGREG_PALETTE_SHIFT)
#define CFGREG_MMU_ENABLE               (1 << CFGREG_MMU_SHIFT)     /* also enables timer registers */
#define CFGREG_VGA_ENABLE               (1 << CFGREG_VGA_SHIFT)

/* button config */
#define CFGBTN_DEBUG_INFO_SHIFT         (6)
#define CFGBTN_BUTTONS_SHIFT            (0)

#define CFGBTN_DEBUG_INFO_MASK          (3 << CFGBTN_DEBUG_INFO_SHIFT)
#define CFGBTN_BUTTONS_MASK             (0xf << CFGBTN_BUTTONS_SHIFT)

#define CFGBTN_DEBUG_INFO_NONE          (0 << CFGBTN_DEBUG_INFO_SHIFT)
#define CFGBTN_DEBUG_INFO_CPU           (1 << CFGBTN_DEBUG_INFO_SHIFT)
#define CFGBTN_DEBUG_INFO_DRIVE1        (2 << CFGBTN_DEBUG_INFO_SHIFT)
#define CFGBTN_DEBUG_INFO_FULL          (3 << CFGBTN_DEBUG_INFO_SHIFT)

/*
0000 0 Menu
0001 1 Cartridge On/Off
0010 2 Toggle Turbo On/Off
0100 4 Disk change drive 8 (next)
0101 5 Disk change drive 9 (next)
*/
#define CFGBTN_LEFT_MENU                (0 << CFGBTN_BUTTONS_SHIFT) /* menu (same as 0.7 sec middle) */
#define CFGBTN_LEFT_CART_BUTTON         (1 << CFGBTN_BUTTONS_SHIFT) /* cartridge on/off/prg */
#define CFGBTN_LEFT_TURBO               (2 << CFGBTN_BUTTONS_SHIFT) /* turbo on/off */
#define CFGBTN_LEFT_RESERVED            (3 << CFGBTN_BUTTONS_SHIFT)
#define CFGBTN_LEFT_DISK0               (4 << CFGBTN_BUTTONS_SHIFT) /* disk change drive 1 */
#define CFGBTN_LEFT_DISK1               (5 << CFGBTN_BUTTONS_SHIFT) /* disk change drive 2 */

/* i/o and cia emulation */
#define CFGIO_IEC_PORT_SHIFT            (7)
#define CFGIO_IEC_RESET_SHIFT           (6)
#define CFGIO_MOUSE_DISABLED_SHIFT      (5)
#define CFGIO_MOUSE_PORT_SHIFT          (4)
#define CFGIO_IR_DISABLED_SHIFT         (3)
#define CFGIO_RESET_MENU_SHIFT          (2)
#define CFGIO_C64IEC_PORT_SHIFT         (0)

#define CFGIO_IEC_CONNECTED             (0 << CFGIO_IEC_PORT_SHIFT)
#define CFGIO_IEC_DISCONNECTED          (1 << CFGIO_IEC_PORT_SHIFT)
#define CFGIO_IEC_RESET                 (1 << CFGIO_IEC_RESET_SHIFT)
#define CFGIO_MOUSE_DISABLED            (1 << CFGIO_MOUSE_DISABLED_SHIFT)
#define CFGIO_MOUSE_ENABLED             (0 << CFGIO_MOUSE_DISABLED_SHIFT)
#define CFGIO_MOUSE_PORT2               (1 << CFGIO_MOUSE_PORT_SHIFT)
#define CFGIO_MOUSE_PORT1               (0 << CFGIO_MOUSE_PORT_SHIFT)
#define CFGIO_IR_DISABLED               (1 << CFGIO_IR_DISABLED_SHIFT)
#define CFGIO_IR_ENABLED                (0 << CFGIO_IR_DISABLED_SHIFT)
#define CFGIO_RESET_MENU                (1 << CFGIO_RESET_MENU_SHIFT)
#define CFGIO_C64IEC_CONNECTED          (0 << CFGIO_C64IEC_PORT_SHIFT)
#define CFGIO_C64IEC_DISCONNECTED       (1 << CFGIO_C64IEC_PORT_SHIFT)

/* config disable register */

/* reading in config mode gives these values */
#define CFGDIS_VICSTATUS_SHIFT          (7)
#define CFGDIS_CP_STATUS_SHIFT          (5) /* readback of reset line */
#define CFGDIS_SLOTSTATUS_SHIFT         (4)
#define CFGDIS_SLOT_SHIFT               (0)

#define CFGDIS_VICSTATUS_MASK           (0x01 << CFGDIS_VICSTATUS_SHIFT)
#define CFGDIS_CP_STATUS_MASK           (0x01 << CFGDIS_CP_STATUS_SHIFT)
#define CFGDIS_SLOTSTATUS_MASK          (0x01 << CFGDIS_SLOTSTATUS_SHIFT)
#define CFGDIS_SLOT_MASK                (0x0f << CFGDIS_SLOT_SHIFT)

#define CFGDIS_CP_STATUS_OK             (0x00 << CFGDIS_CP_STATUS_SHIFT)

/* writing any value leaves config mode */
#define CFGDIS_DISABLE_CFG              (0)

/* config register */

/* write these values to enable/switch config */
#define CFGENA_ENABLE_CFG               42     /* enable config mode */
#define CFGENA_DISABLE_CFG              0xff   /* disable config mode */
#define CFGENA_START_CORE               0x10   /* OR with core slot number to start core (in config/menu mode)  */
#define CFGENA_MENU_MODE                0x20   /* force menu mode (in config mode) */
#define CFGENA_MENU_MODE_DISABLE_NMI    0x21   /* force menu mode, disable NMI (in config mode) */
#define CFGENA_ENABLE_NMI               0x22   /* enable NMI (in config/menu mode) */
#define CFGENA_DISABLE_NMI              0x23   /* disable NMI (in config/menu mode) */
#define CFGENA_RESET                    0xa5   /* soft reset (in config/menu mode) */
#define CFGENA_RESET_EXIT_CFG           0xa6   /* soft reset, leave config mode (in config/menu mode) */

/* reading in config/menu mode gives these values */
#define CFGENA_CARTRIDGE                0x01
#define CFGENA_STANDALONE               0xa1
#define CFGENA_CONE                     0xc1
#define CFGENA_DOCKINGSTATION           0xd1

/* RTI register */

/* reading leaves menu mode (and enables NMI) */

/* writing any value leaves config mode */
#define CFGRTI_DISABLE_CFG              (0)

/*******************************************************************************
    0xd100 - 0xd3ff     256 entry color palette, 3 * 0x100 bytes
*******************************************************************************/

/* FIXME: to be removed */
#define PALRED  0xd100
#define PALGRN  0xd200
#define PALBLU  0xd300

/*******************************************************************************
    0xde00 - 0xdfff     Cartridge I/O
*******************************************************************************/

/* 0xde00 - 0xde01      Retro Replay */
/* 0xde00 - 0xdfff      Final Cartridge 3 */
/* 0xde00 - 0xdeff      GEORAM */
/* 0xdf00 - 0xdf0a      REU */

/*******************************************************************************
    0xdf10 - 0xdf12     SPI interface (MMC64)
*******************************************************************************/

#define SPI_DATA                0xdf10
#define SPI_CTRL                0xdf11
#define SPI_STATUS              0xdf12

#define SPI_CTRL_TRIGGER_SHIFT          (6)
#define SPI_CTRL_SELECT1_SHIFT          (4)
#define SPI_CTRL_SPEED_SHIFT            (2)
#define SPI_CTRL_SELECT0_SHIFT          (1)

#define SPI_CTRL_WRITE_TRIGGER          (0 << SPI_CTRL_TRIGGER_SHIFT)
#define SPI_CTRL_READ_TRIGGER           (1 << SPI_CTRL_TRIGGER_SHIFT)

#define SPI_CTRL_SELECT_MASK            ((1 << SPI_CTRL_SELECT1_SHIFT) | (1 << SPI_CTRL_SELECT0_SHIFT))

#define SPI_CTRL_SELECT_MMC             ((0 << SPI_CTRL_SELECT1_SHIFT) | (0 << SPI_CTRL_SELECT0_SHIFT))
#define SPI_CTRL_SELECT_NONE            ((0 << SPI_CTRL_SELECT1_SHIFT) | (1 << SPI_CTRL_SELECT0_SHIFT))
#define SPI_CTRL_SELECT_FLASH           ((1 << SPI_CTRL_SELECT1_SHIFT) | (0 << SPI_CTRL_SELECT0_SHIFT))
#define SPI_CTRL_SELECT_RTC             ((1 << SPI_CTRL_SELECT1_SHIFT) | (1 << SPI_CTRL_SELECT0_SHIFT))

#define SPI_CTRL_SPEED_250KHZ           (0 << SPI_CTRL_SPEED_SHIFT)
#define SPI_CTRL_SPEED_8MHZ             (1 << SPI_CTRL_SPEED_SHIFT)

/*
    SD Card
*/

#define SPI_STATUS_MMC_WRITE_PROTECT_SHIFT      (4)
#define SPI_STATUS_MMC_PRESENT_SHIFT            (3)
#define SPI_STATUS_BUSY_SHIFT                   (0)

#define SPI_STATUS_CARDRO       (1 << SPI_STATUS_MMC_WRITE_PROTECT_SHIFT)
#define SPI_STATUS_NOCARD       (1 << SPI_STATUS_MMC_PRESENT_SHIFT)
#define SPI_STATUS_BUSY         (1 << SPI_STATUS_BUSY_SHIFT)

/* mandatory commands */
#define SD_GO_IDLE_STATE                (0x40 | 0)    /* CMD0 */
#define SD_SET_OP_COND                  (0x40 | 1)    /* CMD1 */
#define SD_SWITCH_FUNC                  (0x40 | 6)    /* CMD6 */
#define SD_SEND_IF_COND                 (0x40 | 8)    /* CMD8 */
#define SD_SEND_CSD                     (0x40 | 9)    /* CMD9 */
#define SD_SEND_CID                     (0x40 | 10)   /* CMD10 */
#define SD_STOP_TRANSMISSION            (0x40 | 12)   /* CMD12 */
#define SD_SEND_STATUS                  (0x40 | 13)   /* CMD13 */
#define SD_SET_BLOCKLEN                 (0x40 | 16)   /* CMD16 */
#define SD_READ_SINGLE_BLOCK            (0x40 | 17)   /* CMD17 */
#define SD_READ_MULTIPLE_BLOCK          (0x40 | 18)   /* CMD18 */
#define SD_WRITE_BLOCK                  (0x40 | 24)   /* CMD24 */
#define SD_WRITE_MULTIPLE_BLOCK         (0x40 | 25)   /* CMD25 */
#define SD_PROGRAM_CSD                  (0x40 | 27)   /* CMD27 */
#define SD_ERASE_WR_BLK_START_ADDR      (0x40 | 32)   /* CMD32 */
#define SD_ERASE_WR_BLK_END_ADDR        (0x40 | 33)   /* CMD33 */
#define SD_ERASE                        (0x40 | 38)   /* CMD38 */
#define SD_APP_CMD                      (0x40 | 55)   /* CMD55 */
#define SD_GEN_CMD                      (0x40 | 56)   /* CMD56 */
#define SD_READ_OCR                     (0x40 | 58)   /* CMD58 */
#define SD_CRC_ON_OFF                   (0x40 | 59)   /* CMD59 */
#define SD_SD_STATUS                    (0x40 | 13)   /* ACMD13 */
#define SD_SEND_NUM_WR_BLOCKS           (0x40 | 22)   /* ACMD22 */
#define SD_SET_WR_BLK_ERASE_COUNT       (0x40 | 23)   /* ACMD23 */
#define SD_SEND_OP_COND                 (0x40 | 41)   /* ACMD41 */
#define SD_SET_CLR_CARD_DETECT          (0x40 | 42)   /* ACMD42 */
#define SD_SEND_SCR                     (0x40 | 51)   /* ACMD51 */

/* SD specs say "In the CRC OFF mode, the CRC bits of the command are defined as
   'don't care' for the transmitter and ignored by the receiver." - however,
   apparently using 0 as CRC is required by some cards.
*/
#define SD_NO_CRC       0x01

/*
    PCF2123 SPI RTC
*/

#define RTC_CMD_READ    (0x80 | 0x10)
#define RTC_CMD_WRITE   (0x00 | 0x10)

#define RTC_REG_CTRL1            0x00
#define RTC_REG_CTRL2            0x01
#define RTC_REG_SECOND           0x02
#define RTC_REG_MINUTE           0x03
#define RTC_REG_HOUR             0x04
#define RTC_REG_DAY              0x05
#define RTC_REG_WEEKDAY          0x06
#define RTC_REG_MONTH            0x07
#define RTC_REG_YEAR             0x08
#define RTC_REG_ALARM_MINUTE     0x09
#define RTC_REG_ALARM_HOUR       0x0a
#define RTC_REG_ALARM_DAY        0x0b
#define RTC_REG_ALARM_WEEKDAY    0x0c
#define RTC_REG_OFFSET           0x0d
#define RTC_REG_TIMEOUT          0x0e
#define RTC_REG_COUNTDOWN        0x0f

/*
    S25SL128P SPI Flash ROM
*/

#define FLASH_CMD_PAGE_PROGRAM   0x02
#define FLASH_CMD_READ_DATA      0x03
#define FLASH_CMD_READ_STATUS    0x05
#define FLASH_CMD_WRITE_ENABLE   0x06
#define FLASH_CMD_ERASE          0x20

#define FLASH_STATUS_BUSY        0x01

/*******************************************************************************
    Chameleon RAM memory map
*******************************************************************************/

#define RAMBASE_C64         0x00000000L /* 64 KByte RAM for C64 mode */

/* 0x00010000 – 0x000FFFFF 960 Kb RAM free for user programs */

/* 0x00100000 - 0x0018ffff (0x00090000) read from flash at startup (0x00084000 used) */
/* >> default roms start */
#define RAMBASE_CFGROM          0x00100000L /* 32kb Startup and initialization ROM (Bootloader) */
#define RAMBASE_MMC64           0x00108000L /* 8kb MMC64 BIOS image */
#define RAMBASE_BASIC           0x0010a000L /* 8kb BASIC V2 ROM image */
#define RAMBASE_MENU_CHARGEN    0x0010c000L /* 4kb Character ROM image */
#define RAMBASE_CHARGEN         0x0010d000L /* 4kb Character ROM image */
#define RAMBASE_KERNAL          0x0010e000L /* 8kb Kernal ROM image */

#define RAMBASE_DEFCRT1         0x00110000L /* 64k (Slot1) (default: NTSC RR) */
#define RAMBASE_DEFCRT0         0x00120000L /* 64k (Slot0) (default: PAL RR) */

#define RAMBASE_MENU            0x00130000L /* 64k Chameleon menu system */
#define RAMBASE_MENUOVL         0x00140000L /* 64k Chameleon menu system - overlays (8k each) */

#define RAMBASE_PLUGINS         0x00150000L /* 2*64k Chameleon menu system - plugins (4k each) */

/* 1*64k reserved for monitor plugins */

#define RAMBASE_DRIVEROM        0x00180000L /* 16k 1541 floppy drive ROM , copied to RAMBASE_DRIVEn */

/* << default roms end */

#define RAMBASE_DRIVE0          0x00180000L
#define RAMBASE_DRIVE1          0x00190000L

/* >> menu system buffers start */
#define RAMBASE_FREEZER         0x001a0000L /* menu system freezer buffer */

#define RAMBASE_MENUSCREENS     0x001b0000L /* menu system screens buffer */

/* 64k free space here ? */

#define RAMBASE_MENUCFGBUFFER   0x001d0000L /* 1 64k block buffered from flash */
#define RAMBASE_MENUCFGDATA     0x001dfe00L /* last two pages used for config data */

#define RAMBASE_PLUGBUF         0x001e0000L /* buffer used by plugins */

#define RAMBASE_MOUNTLIST       0x001f0000L /* disk image mount info */

#define RAMBASE_DIRBUF0         0x00200000L /* filebrowser directory entries */
#define RAMBASE_DIRBUF0PLG      0x00210000L
#define RAMBASE_DIRBUF1         0x00220000L
#define RAMBASE_DIRBUF1PLG      0x00230000L

#define RAMBASE_MENUBUFFERS     RAMBASE_FREEZER
#define RAMBASE_MENUBUFFERS_LEN ((RAMBASE_DIRBUF1PLG + 0x10000L) - RAMBASE_FREEZER)
/* << menu system buffers end */

#define RAMBASE_DEFCRT0RAM  0x00240000L /* 64k Cartridge RAM (Slot0) */
#define RAMBASE_DEFCRT1RAM  0x00250000L /* 64k Cartridge RAM (Slot1) */
#define RAMBASE_USERCRT0RAM 0x00260000L /* 64k Cartridge RAM (Slot2) */
#define RAMBASE_USERCRT1RAM 0x00270000L /* 64k Cartridge RAM (Slot3) */

/* D64 cache, needs max 206114 bytes = 0x00032522 for the image, 
   plus 0x00000400 bytes for status bits = 0x00032922 */
#define RAMBASE_DISK0CACHE       0x00280000L
#define RAMBASE_DISK0ERRCACHE   (0x00280000L+205312)
#define RAMBASE_DISK1CACHE       0x002c0000L
#define RAMBASE_DISK1ERRCACHE   (0x002c0000L+205312)

/*
    3 MByte for disk images

    - 672k needed for each image / slot (8k per track, 42*2 tracks = 0x000a8000)
    - 4 slots per drive
    = 2688k per drive (= 0x002a0000)
*/
#define RAMBASE_DISK0       0x00300000L /* 3mb drive 0 images */

#define RAMBASE_DISK0IMG0   (RAMBASE_DISK0 + (0x00000000L))
#define RAMBASE_DISK0IMG1   (RAMBASE_DISK0 + (0x000a8000L))
#define RAMBASE_DISK0IMG2   (RAMBASE_DISK0 + (0x00150000L))
#define RAMBASE_DISK0IMG3   (RAMBASE_DISK0 + (0x001f8000L))

/* some free space here */

/* 1 Mbyte video RAM
   - 3 buffers
*/
#define RAMBASE_VRAM        0x00600000L

#define RAMBASE_DISK1       0x00700000L /* 3mb drive 1 images */

#define RAMBASE_DISK1IMG0   (RAMBASE_DISK1 + (0x00000000L))
#define RAMBASE_DISK1IMG1   (RAMBASE_DISK1 + (0x000a8000L))
#define RAMBASE_DISK1IMG2   (RAMBASE_DISK1 + (0x00150000L))
#define RAMBASE_DISK1IMG3   (RAMBASE_DISK1 + (0x001f8000L))

/* some free space here */

/* 2 * 1Mb max for CRT file (cartridge slots 2 and 3) */
#define RAMBASE_USERCRT0    0x00a00000L
#define RAMBASE_USERCRT1    0x00b00000L

#define RAMBASE_GEORAM      0x00c00000L /* 4Mb geoRAM memory */
#define RAMBASE_REU         0x01000000L /* 16Mb REU */


#endif /* CHAMELEON_H_ */