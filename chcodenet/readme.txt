
This little program accepts the same options that grahams "codenet" client
supports, but instead works over the USB connection of the chameleon.

to get a list of options just run it without arguments:
$ ./chcodenet


-h --help          this help
--verbose          enable verbose messages
--debug            enable debug messages

-w Filename        Sends a PRG file to the C64.
-wa Filename Addr  Sends a PRG file to the C64 to a specified address.
-wb Filename Addr  Sends a binary file to the C64 to a specified address.
-x Filename        Sends a PRG file to the C64 and executes it
-f Start End Fill  Fills a block of C64 memory.
-e Addr            Jumps to an address in memory.
-r                 Executes a program via "RUN".
-n, -p, -T, -R     ignored for compatibility


The c64 must be running basic for some of the commands to work (since they
inject keypresses into the keyboard buffer). I recommend to just press reset
before using them :)

for the original codenet client look here: http://www.oxyron.de/html/codenet.html
