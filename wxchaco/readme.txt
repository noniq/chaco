
TODO:

- rewrite Threading.cpp in the following way:
  - get rid of timeout param of "TLWaitForEvent"
  - use wx-threading stuff (instead of windows- or posix threads)

- rewrite TaskMgr.h in the following way:
  - add a "stop task" method (for "abort" function)
  - use wx threading stuff directly and remove Threading.cpp/.h
  - use only one extra task which does all the payload

- ChaCo does not do any sanity checks on files and/or filenames. User can select an
  .rbf file as binary and a binary file as rbf file, which will fail. Sanity check should
  include size range and filename suffix for the core
