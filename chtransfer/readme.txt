
This is a port of "EasyTransfer" by Skoe to the chameleon USB

- .crt transfer option removed
- "raw" send option removed
- embedded USB test program added

the binaries have been renamed so they can coexist with the actual ef3 tools:

ef3xfer -> chxfer
EasyTransfer -> ChTransfer

--------------------------------------------------------------------------------

if the cc65 tools are not in your path, use something like this to compile:

$ CC65PREFIX=~/cc65/bin/ make all

TODO:

- perhaps add disk reader
- transfer speed could be improved by sending fewer, larger blocks
