
TODO:

- rewrite Threading.cpp in the following way:
  - get rid of timeout param of "TLWaitForEvent"
  - use wx-threading stuff (instead of windows- or posix threads)

- rewrite TaskMgr.h in the following way:
  - add a "stop task" method (for "abort" function)
  - use wx threading stuff directly and remove Threading.cpp/.h
  - use only one extra task which does all the payload
