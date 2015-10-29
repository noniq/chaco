Chameleon USB Utilities V1.8
----------------------------

This directory contains a port of "Easyflash 3 USB Utilities v1.8" by Tom-Cat -
original release archive can be found here: http://csdb.dk/release/?id=136141

- the e[xecute] and s[end] commands are equivalent, and directly write to C64
  memory instead of using a helper program on the c64 side (which is no more
  necessary)
- the .tap writing feature has been removed

- file copy commands currently use a slow per-byte transfer protocoll wrapper

--------------------------------------------------------------------------------

Usage: chusb [command] [file] [options]
 e[xecute]  file.prg|p00                   - execute prg on c64
 c[opy]     file.prg|p00|d64|d81|d71       - copy files to c64
 x[fer]     [p00]                          - copy files from c64
 w[rite]    file.d64|d81 [verify] [kernal] - write image on c64
 r[ead]     file.d64|d81 [40] [kernal]     - read image from c64
 d[ir]      file.d64|d81|d71               - display dir of file and check it
 f[ormat]   [40]                           - turbo format 1541 floppy
 s[end]     [file.prg]                     - send file.prg to C64
                                             if no file then send chusb.prg
 0[test]                                   - test the usb connection

--------------------------------------------------------------------------------

e[xecute]  file.prg|p00
-----------------------
- execute prg on c64
uses direct DMA (fast). c64 must be in direct (basic) mode

c[opy]     file.prg|p00|d64|d81|d71
-----------------------------------
- copy files to c64
uses per-byte transfers (very slow)

x[fer]     [p00]
----------------
- copy files from c64
uses per-byte transfers (very slow)

w[rite]    file.d64|d81 [verify] [kernal]
-----------------------------------------
- write image on c64
uses block based transfer

r[ead]     file.d64|d81 [40] [kernal]
-------------------------------------
- read image from c64
uses block based transfer

d[ir]      file.d64|d81|d71
---------------------------
- display dir of file and check it

f[ormat]   [40]
---------------
- turbo format 1541 floppy (starts embedded turbo formatter program)

s[end]     [file.prg]
---------------------
- send file.prg to C64, if no file then send chusb.prg
uses direct DMA (fast). c64 must be either in direct (basic) mode or chusb.prg
server must be running.

0[test]
-------
- test the usb connection

================================================================================
following is a slightly edited copy of the original readme:
================================================================================

     ********************************************
           Chameleon USB Transfer Utilities
     ********************************************
            tomcat@sgn.net, tobias@icomp.de
                        v1.8C

   by Tom-Cat , S!R (dir browse), Krill (turbo sector)
          TLR (format), Tobias (port to chameleon)

This is a all-in-1 utility for the Chameleon cartridge
and its USB connection to the PC.

Prerequisites:
- PC and Chameleon are connected using the USB cable

First send the Server to the C64. The C64 must be in BASIC mode
("exit to basic" from chameleon menu). When you are in BASIC mode
then send the chusb.prg server to the c64 using this command line :

$ chusb s

This will send and autostart the server on the c64.

When the Server is running you can still do some actions on
the C64 side like change the active Drive number, execute
a custom drive command or change the directory (only for the
mass storage devices).

When you start the client program with a chosen command then
the c64 counter-part is automatically started, so be carefull
what you do on the PC part. (you may have to reset to BASIC
before a command works)

The "chusb.exe" utility on the PC side is a command line
utility with the following usage:

Usage: CHUSB.exe command file [options]
        commands: e[xecute]  file.prg|p00
                  c[opy]     file.prg|p00|d64|d81|d71
                  x[fer]     [p00]
                  w[rite]    file.d64|d81 [verify] [kernal]
                  r[ead]     file.d64|d81 [kernal]
                  d[ir]      file.d64|d81|d71
                  s[end]     [file.prg]
                  0[test]

The commands are:

execute
=======
This will send any .PRG or .P00 file from the PC to the C64
and execute it there.
This program is able to run any file that is upto 250 blocks
long - so more than the disk drive can.

copy
====
This will copy FILES from a .D64, .D81 or .D71 image over
to the C64 drive of your choice. You can also send over
single .PRG or .P00 files.
This program will copy only PRG and SEQ files in a disk
image over. Any other files will be skipped.
Original filenames will be preserved - so your drive must
support writing of filenames from the full c64 petascii set
(if you have a SD2IEC or uIEC drive then turn on P00
filename generation).

The program uses kernal routines to write the files so
it is advisable that you use Jiffy DOS or similar to
speed things up (a full .D64 image will transfer to
IEC2ATA device using Jiffy DOS in about 1 minute).

Please do not try to cancel the transfer on the pc side
or the files on the c64 side will be damaged!

You can press the RUN/STOP key on C64 - this will stop the
transfer on the NEXT file. The current file will still be
fully transferred - so there is no corruption.

xfer
====
This command will transfer FILES from C64 drive to PC.
If you dont supply the "p00" argument then the files will
be written as .PRG files otherwise .P00 files will be
written which also include original C64 filename!

When you start this command the C64 will go into the
Directory Browser mode. From here you can selec the file
you want to copy to the PC by pressing RETURN on the file.
When the file is transferred the C64 will go back to the
directory browser so you can copy over more files.
If you press RUN/STOP key then the copy session will be
finished and the PC program will exit.

NOTE: You can also copy over the SEQ files but you must
      rename them to .SEQ on the PC side!

read
====
This will read all the data from 1541 or 1581 disk drive
connected to the C64 and transfer it over USB to the PC
which will then be written to a .D64 or .D81 image.

If you provided a .d64 image as the filename then it will
use a fast sector reader on the 1541. You can use the
kernal option if you would prefere to use the slower
kernal routines for transfer. If you provided the .d81
image type then the kernal routines are always used.

You can provide the "40" option after the filename to
read 40 tracks from the disk !

You can abort the transfer on the C64 side by pressing
RUN/STOP key.

write
=====
This will write all the data from an .D64 or .D81 image
on your PC over the USB connection to a floppy in your
1541 or 1581 disk drive connected to the C64.

NOTE: The image type is determined by the .D64 or .D81
image that you load on the C64 side. So please make sure
that the image you are trying to write to the drive is
compatible with the drive (.D64 images for 1541 disk drive
and .D81 images for the 1581 drive!).

If you provided a .d64 image as the filename then it will
use a fast sector writer on the 1541. You can use the
kernal option if you would prefere to use the slower
kernal routines for transfer. If you provided the .d81
image type then the kernal routines are always used.

Also select if you want to have verify turned ON or OFF
for image writing by suppling the verify option.
If it is turned ON then it will retry to write the sector
upto 3 times before giving up.
Verify slows down the writing considerably though.

If the D64 image has 40 tracks then all of them will be
written (turbo version only!).

You can abort the transfer on the C64 side by pressing
RUN/STOP key. This will unfortunetly leave the data on
the floppy unusable.

format
======
Will format the selected drive - the drive must be either
1541 or 1571. You can use the "40" option to format a 40
tracks disk in the drive. The format is very fast!
To format a 1581 drive please use the "Custom Drive Command"
option on the c64 side and enter the "N0:NAME,ID" command.
Thanx to TLR for the format code.

dir
===
This will display the directory of provided .D64/.D81/.D71
image. The image will also be checked for corruption.

tapwrite
========
This will write a .TAP file to tape on the c64 side.

test
====
This will test the USB connection between C64 and the PC.

After the program has finished working (it takes a few
minutes) it will report success. If any errors are detected
it will finish with an error message.

After the test is run you need to restart your c64!

send
====
[THIS COMMAND WORKS ONLY WHEN C64 IS IN BASIC MODE !]

If no argument is present it will send chusb.prg to the
C64 (reset to BASIC before) so it will autorun it.
If you supply a PRG file then it will send that file
instead.


Huge thanx to:
- skoe : for making EF3 and all this possible
- krill : for sector read/write code
- tlr : for format II code
- S!R : for the directory browse code
- enthusi : for the TAP writer code
- lemming : for providing me with an EF3 and testing !
- jmpfce : for testing
- Tom-Cat : for making the original ef3usb tool
- #easyflash and N0S members: for all the help!

History:
--------
v1.0    : - Initial release
v1.1    : - ef3copy.prg - c64 side has proper Device number
            written on startup. It was #08 before even though
            the current device was something else.
          - ef3copy.prg - Added "Custom Drive Command" menu
            option. You can now send any command to the drive
            you want. After it is done it prints the error
            channel. Also added "Exit to Basic" option.
          - ef3copy.exe - More robust transfer of data to the
            c64. No more problems with slower drives like
            1571. Progress is being displayed for transfer.
          - ef3copy(both sides) - Added some error checking.
            Also added "Run/Stop" key checking after each file
            has copied to stop the transfer.
          - added "ef3imagereader" and "ef3imagewriter" tools
            which add .D64 and .D81 image reading/writing
            to the 1541 and 1581 drives. For now they use the
            kernal routines to transfer sectors.
v1.2    : - ef3imagereader & writer: The 40 track .D64 images
            are actually not supported by the 1541 DOS :( So
            these were removed from the supported images.
            Unfortunetly you cannot write or read 40 track
            1541 images using kernal calls. This will have to
            wait until I have the custom fast read/write
            implemented for 1541 drive.
          - ef3imagewrite: Verify is now supported when
            writing the image to the disk. You can select it
            in the menu using the F5 key. It will retry to
            write the image 3 times before giving up.
          - ef3copy.exe: Now you can also Copy over .PRG and
            .P00 single files. No need to first put them into
            a .D64 image and transfer them over.
v1.3    : - added "turbo" version of D64 image reader and
            writer utilities. Thanx to Krill for the routines
            of his plushdos.
v1.4    : - created a single "ef3usb.prg" and "ef3usb.exe"
            utility which includes all previous utilities
            in one single package.
v1.5    : - speeded up the Turbo Reader a bit with reverse
            sector loading.
          - added 40 track support to D64 read and write
            (turbo version only!)
          - added turbo format command - 35 and 40 sectors.
            curtesy of tlr's "Format II" utility.
          - "dir" and "copy" commands now also display the
            disk name and ID.
v1.6    : - "execute" command now also accepts .P00 files.
v1.7    : - "xfer" command added. This command will transfer
            (copy) files from the C64 to the PC! You can
            write these files in .PRG or .P00 format.
v1.8    : - Added TAP writing (by enthusi)
          - Added possibility to send a PRG file to the EF3
            Menu running on c64 and autorun it. If no file
            is present it sends ef3usb.prg file and runs it.
          - minor bug in format handling fixed.
          - bug in "copy" PRG files which use path - the
            filename on the c64 would be wrong.
v1.8C   : - port to chameleon USB
-------------
  Tomaz Kac
  Tobias
