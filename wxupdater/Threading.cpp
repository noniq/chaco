
#include "Chaco.h"

#include "Threading.h"
#include "Dbg.h"

#ifndef DEBUG
/* #define DEBUG */
#endif

#ifdef DEBUG

#include <stdio.h>
#define LOGERR(...)       dbg::dError( __VA_ARGS__ )
#define LOGVER(...)       dbg::dVerbose( __VA_ARGS__ )
#define LOGMSG(...)       dbg::dPrint( __VA_ARGS__ )
#define DBG(...)          dbg::dDebug( __VA_ARGS__ )

#else

#define LOGERR(...)       dbg::dError( __VA_ARGS__ )
#define LOGVER(...)       dbg::dVerbose( __VA_ARGS__ )
#define LOGMSG(...)       dbg::dPrint( __VA_ARGS__ )
#define DBG(...)

#endif


#ifndef LINUX

#include <windows.h>

DWORD WINAPI threadclasshelper(void * args)
{
    Runnable * TClass = (Runnable*)args;
    TClass->run();   
    return 0;
}

int ThreadLib::TLCreateThread(Runnable * rClass)
{
    if(!rClass)return -1;
    DWORD dummy;
    CreateThread(NULL,0,threadclasshelper,(void*)rClass,0,&dummy);
    return 0;
}

DWORD WINAPI threadhelper(void * args)
{
    threadlib_thread_t * T = (threadlib_thread_t *)args;
    (T->entryPoint)(T->params);
    return 0;
}

int ThreadLib::TLCreateThread(threadlib_thread_t * thread)
{
    if(!thread)return -1;
    thread->tHandle = CreateThread(NULL,0,threadhelper,(void*)thread,0,(DWORD*)&thread->threadId);
    if (thread->tHandle) {
        return 0;
    }
    return -1;
}

tHANDLE ThreadLib::TLCreateEvent()
{
    return CreateEvent(NULL,false,false,NULL);  
}

int ThreadLib::TLWaitForEvent(tHANDLE event, long timeout)
{
    DWORD rc = WaitForSingleObject(event,timeout);
    if(!rc)return 0;
    else return -1; 
}

int ThreadLib::TLSignalEvent(tHANDLE event)
{
    if(SetEvent(event))return 0;
    else return -1;
}

tHANDLE ThreadLib::TLCreateMutex()
{
    return CreateMutex(NULL,false,NULL);   
}

bool ThreadLib::TLReleaseMutex(tHANDLE mtx)
{
    return ReleaseMutex(mtx);  
}

#else

#include <pthread.h>
#include <semaphore.h>
#include <strings.h>

void *threadclasshelper(void * args)
{
    Runnable * TClass = (Runnable*)args;
    TClass->run();
    return NULL;
}

int ThreadLib::TLCreateThread(Runnable * rClass)
{
    int ret;
    pthread_t dummy;

    if (!rClass) {
        return -1;
    }
    ret = pthread_create( &dummy, NULL, threadclasshelper, (void*) rClass);
    if (ret != 0) {
        return -1;
    }
    return 0;
}

void * threadhelper(void * args)
{
    threadlib_thread_t *thread = (threadlib_thread_t *)args;
    (thread->entryPoint)(thread->params);
    return NULL;
}

int ThreadLib::TLCreateThread(threadlib_thread_t * thread)
{
    int ret;
    if(!thread) {
        return -1;
    }
    ret = pthread_create( &thread->threadId, NULL, threadhelper, (void*) thread);
    if (ret != 0) {
        return -1;
    }
    return 0;
}

tHANDLE ThreadLib::TLCreateEvent()
{
    sem_t *sem = new sem_t; /* FIXME: memory leak ! */
    sem_init(sem, 0, 0);
    return sem;
}

int ThreadLib::TLWaitForEvent(tHANDLE event, long timeout)
{
    int rc;
    sem_t *sem = (sem_t *)event;
    if (timeout != -1) {
        LOGERR("TLWaitForEvent - timeout not supported.\n");
    }
    DBG("TLWaitForEvent\n");
    rc = sem_wait(sem);
    DBG("TLWaitForEvent ret (%d)\n", rc);
    return rc;
}

int ThreadLib::TLSignalEvent(tHANDLE event)
{
    int rc;
    sem_t *s = (sem_t *)event;

    DBG("TLSignalEvent\n");
    rc = sem_post(s);
    DBG("TLSignalEvent ret (%d)\n", rc);
    return rc;
}

tHANDLE ThreadLib::TLCreateMutex()
{
    sem_t *sem = new sem_t; /* FIXME: memory leak ! */
    sem_init(sem, 0, 1);
    return sem;
}

bool ThreadLib::TLReleaseMutex(tHANDLE mtx)
{
    sem_t *s = (sem_t *)mtx;
    sem_post(s);
    return 0;
}

#endif


