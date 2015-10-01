#include <stdio.h>
#include "Dbg.h"

#ifndef DEBUG
/* #define DEBUG */
#endif

dbgOut * debugOutClass;
stringstream * dbgOut::dout;
stringbuf * dbgOut::dbuf;

static char * buffer;
static int verbose = 0;

void dbg::setDebugOutClass(dbgOut * dClass)
{
    buffer = new char[256];
    debugOutClass = dClass;
}

void dbg::printDebug(std::string outString)
{
#ifdef LINUX
    const char *b = outString.c_str();
#endif
    debugOutClass->outPrint(outString);
#ifdef LINUX
    printf("%s", b); fflush(stdout);
#endif
}

void dbg::dPrint(const char * fmt,...)
{
    va_list vl;
    va_start(vl,fmt);
    vsprintf(buffer,fmt,vl);
    dbg::printDebug(buffer);
}

void dbg::dError(const char * fmt,...)
{
    va_list vl;
    va_start(vl,fmt);
    vsprintf(buffer,fmt,vl);
    dbg::printDebug("ERROR: ");
    dbg::printDebug(buffer);
}

void dbg::dVerbose(const char * fmt,...)
{
    if (verbose >= 1) {
        va_list vl;
        va_start(vl,fmt);
        vsprintf(buffer,fmt,vl);
#ifdef DEBUG
        dbg::printDebug("V: ");
#endif
        dbg::printDebug(buffer);
    }
}

void dbg::dDebug(const char * fmt,...)
{
    if (verbose >= 2) {
        va_list vl;
        va_start(vl,fmt);
        vsprintf(buffer,fmt,vl);
#ifdef DEBUG
        dbg::printDebug("D: ");
#endif
        dbg::printDebug(buffer);
    }
}

void dbg::setVerbose(int verb)
{
    verbose = verb;
}
