
#ifndef CHACO_H_
#define CHACO_H_

#ifdef LINUX

#include <unistd.h>
static inline void Sleep(unsigned int num)
{
    usleep(1000 * num);
}

#else

#include <windows.h>
#include <process.h>
#include <tchar.h>

#endif

#endif
