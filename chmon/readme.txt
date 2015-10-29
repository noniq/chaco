
chmon is a port of "netmon" by hannenz to chameleon USB, with the following
differences:

- --ip option removed
- --send option added (send and start chmon.prg before running chmon)

the reu functionality is completely untested atm

have a look at the original readme for instructions on how to use the program

the original program was released here: http://people.freenet.de/hannenz/netmon.html

--------------------------------------------------------------------------------

the binaries have been renamed so they can coexist with the regular netmon

netserv.prg -> chmon.prg
netmon.exe -> chmon.exe

--------------------------------------------------------------------------------

$ chmon --help

-s --send            send server program (chmon.prg)
-c --command [cmd]   direct command, type chmon -c help for a full list of
                     available commands or chmon -c help name for usage of the
                     given command
-v --version         print version number
-h --help            this help message

for any commands to work, the server program chmon.prg must be running on the
c64. you can use the --send option to send it via USB, for this to work the c64
must be in basic mode ("reset to basic" from chameleon menu).

chmon.prg will report OK and wait for incoming packets. The server can be
quitted by hitting RUN/STOP on C64.

When chmon.prg is running, leave your C64 and start CHMON on the PC. You will be
in some kind of shell now resembling strongly to a standard machine languag
monitor, just that any commands will be sent over to the C64, executed there and
a response will be send back which will be processed and displayed in the CHMON-
Shell.

To make use of CHMON from within scripts, it is possible to avoid the shell and
give a single command as parameter. See the Reference for details about this
feature.

CHMON allows full access to all C64-RAM (including REU!!) and devices on the
serial bus (disk drives).

chmon will start in the SHELL-MODE by default. However it is possible to invoke
chmon just for one single command passed as parameter and bypass the shell-mode.
chmon will interpret any string from the command line that is not an option as a
single command line.

All numeric input and output is in HEX only, except with the '#'-command (device
numbers are decimals)!!

All parameters are seperated by spaces. To allow spaces within one (string)
parameter, set it into quotation marks. Also use quotation marks to specify
string parameters

Attention: Most likely your local shell will interpret the quotation marks if
given at the command line promt, so try masking them if giving commands as
parameter so that chmon will recognize them correctly - in Linux/UNIX bash you
would need to preceed each quotation mark by a backslash to do so. Don't ask me
how to do that under Windows, coz I don't know....


MEMORY COMMANDS


m [addr [addr]]
will display the memory content of the specified memory area as well-known hex
ASCII & PetASCII dump

i [addr [addr]]
display the specified memory area as PetASCII dump

d [addr [addr]]
print a disassembly of the specified memory area to stdout.
if no address range is given for m,i or d it will start at 0x0000 and continue
where it stopped last time

ram [value]
set the type of memory access. If no value is given it will default to #$34. A
value >$80 will set REU access. REU memory is then accessed just by specifying
24-bit-addresses (e.g. $034567 will acces address $4567 in bank 3).

rom
shortcut for ram $37

reu
shortcut for ram $ff

put filename [addr]
sends the contents of filename to c64 memory at addr. if addr is not given, the
first two bytes of the file are used as "load address", else the whole file,
including the first two bytes is sent to the given memory location.

get addr addr filename
get specified memroy area and save to filename.

poke addr "string" | val[val[val...]]]
write string or val-byte(s) to addr. (try poke 0400 "hello world!")

f from to value
fill c64 memory with value in given range specified by from and to.

h from to "string" | [val[val[val...]]]
searches for occurances of "string" or val,val,val ... in memory area specified
by from and to.

c from to with
compares the memory segment starting at from up to to with the memory segment
starting at with. prints any mismatches to stdout.


EXECUTION COMMANDS


g addr
jump (goto) to the given address

call addr [areg[xreg[yreg[sreg]]]]
call a subroutine - performs a JSR to addr with the registers set to areg,xreg,
yreg and sreg (status-register) before calling. registers not specified are set
to zero.
If the called routine returns and chmon.prg is still alive, it will report the
register values upon re-entry.

basic
exit server to BASIC.

run
perform BASIC "RUN".


DISK ACCESS


$[pattern]
show directory of current device. Use any pattern that your drive supports,
e.g. $*=SEQ

#[value]
set/ tell current device. if no value is specified, it will display the current
device, else the device is set to value.

@[string]
will send any chars following the '@' to command channel and read it afterwards.
Be aware that there is NO SPACE between @ and string and that in this
exceptional case, no '#' is needed to specify a string parameter!

read localfile [remotefile]
read a file from disk to local filesystem - reads localfile from disk and saves
it to remotefile in the local filesystem. if remotefile is not specified it will
default to the same name as localfile.

write localfile [remotefile]
write a file from the local filesystem to disk - reads localfile from local
filesystem disk and saves it to remotefile on disk. if remotefile is not
specified it will default to the same name as localfile.


  MISC COMMANDS


status
displays some info about server status.

quit, x
exit netmon.

help | ? [command]
display help; you may specify a specific command to get help only for this
command.

cmd string [parameter [parameter... ]]
send all following parameters to the local system's shell, e.g. try cmd ls -l
to send the ls command with parameter -l to your linux shell.
