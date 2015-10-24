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
