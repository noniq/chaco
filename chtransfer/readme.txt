
This is a port of "EasyTransfer" by Skoe to the Chameleon USB

- .crt transfer option removed
- "raw" send option removed
- embedded USB test program added

the binaries have been renamed so they can coexist with the actual ef3 tools:

ef3xfer -> chxfer
EasyTransfer -> ChTransfer

--------------------------------------------------------------------------------

Both tools will send a helper program to the C64 before doing their work, so
the c64 has to be in basic mode for this to work ("reset to basic" in chameleon
menu).

$ chxfer
chxfer (151028-0220) - Transfer data to a Chameleon over USB.

Usage: chxfer <action> [options]
Actions:
  -h       --help           Print this help and exit
  -x FILE  --exec FILE      Send a PRG file and execute it
  -w FILE  --write FILE     Write a disk image (d64)
           --format         format disk before writing
           --verify         verify disk after writing
           --usbtest        Perform USB connection test

Options to be used with --write-disk:
  -d NUM   --drive          Drive number to be used (8)

--------------------------------------------------------------------------------

if the cc65 tools are not in your path, use something like this to compile:

$ CC65PREFIX=~/cc65/bin/ make all

TODO:

- perhaps add disk reader
- transfer speed could be improved by sending fewer, larger blocks
