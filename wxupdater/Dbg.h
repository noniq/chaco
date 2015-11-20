
#ifndef DBG_H_
#define DBG_H_

#include <iostream>
#include <string>
#include <sstream>
#include <stdlib.h>
#include <stdarg.h>

using namespace std;

class dbgOut
{
    public:
        virtual void outPrint(std::string) = 0;
        static stringstream * dout;

    private:
        static stringbuf * dbuf;
};

namespace dbg
{
    void setDebugOutClass(dbgOut * dClass);

    void dPrint(const char * fmt,...);
    void dError(const char * fmt,...);
    void dVerbose(const char * fmt,...);
    void dDebug(const char * fmt,...);

    void printDebug(std::string outString);

    void setVerbose(int verbose);
}

#endif
