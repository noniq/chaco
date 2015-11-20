#ifndef _THREADING_H_
#define _THREADING_H_

/*
*   Include file for basic threading capabilities
*/

#include <string>

typedef void * tHANDLE;
typedef void (*THREADENTRYFUNC)(void *);

class Runnable
{
    public:
        virtual void run() = 0;
};

typedef struct
{
#ifndef LINUX
    tHANDLE tHandle;
#endif
    unsigned long threadId;
    THREADENTRYFUNC entryPoint;
    void * params;
    
}threadlib_thread_t;

namespace ThreadLib
{
    int TLCreateThread(Runnable * rClass);
    int TLCreateThread(threadlib_thread_t * thread);
    tHANDLE TLCreateEvent();
    int TLWaitForEvent(tHANDLE event, long timeout);
    int TLSignalEvent(tHANDLE event);
    tHANDLE TLCreateMutex();
    bool TLReleaseMutex(tHANDLE mtx);
}

#endif
