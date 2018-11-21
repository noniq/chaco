This repository contains the Chameleon Control Program (Chaco) and related
offspring:

flasher         original update program from the release packages (originally
                named "update", but renamed since that triggers a silly UAC
                warning in win7 and above)
wxflasher       alternative self contained update program

chacocmd        original commandline USB client
Chaco           original GUI USB client

chshot          make screenshot from C64 screen over USB
chcodenet       a "codenet" clone that works over USB
chtransfer      port of "EasyTransfer" by Skoe (GUI USB client)
chxfer          port of "ef3xfer" by Skoe
chmon           port of "netmon" by Hannenz
chusb           port of "ef3usb" by Tom-Cat

doc             some more documentation for developers
libs            source packages of some libraries that are required for building

--------------------------------------------------------------------------------
Quick Instructions:

- tools that want to transfer a helper program to the c64 usually require the
  c64 in basic mode for that ("reset to basic" in chameleon menu)
- tools that want to send an external helper .prg may look for it in the current
  working directory

--------------------------------------------------------------------------------

Home of this repository is: https://svn.icomp.de/svn/chaco/

For more information have a look at our WIKI: http://wiki.icomp.de/wiki/Chameleon

We accept patches in unified diff/patch format.

If you have further questions contact me at tobias@icomp.de

--------------------------------------------------------------------------------

PLEASE UNDERSTAND THAT WE CAN NOT GIVE ANY FURTHER SUPPORT ON HOW TO USE, HOW
TO COMPILE, OR HOW TO DO ANYTHING ELSE WITH THE CODE CONTAINED HERE.

Please read license.txt
