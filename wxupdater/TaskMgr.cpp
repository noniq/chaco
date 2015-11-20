
#include <string>
#include <stdio.h>

#include "TaskMgr.h"
#include "Dbg.h"

#ifndef DEBUG
/* #define DEBUG */
#endif

#ifdef DEBUG

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

using namespace std;


tHANDLE TaskMgr::hTaskThread;
int TaskMgr::TaskLoopStatus;

tHANDLE TaskMgr::TaskMgrEvent; 

int TaskMgr::allTasksInactive = 0;

vector<Task*> TaskMgr::TaskList;


int TaskMgr::addTask(std::string name, TASKENTRY_INT tEntry)
{
    DBG("TaskMgr: added Task '%s'\n", (const char*)name.c_str());
    Task * newTask = new Task(name);
    newTask->setTaskEntry(tEntry,0);//default
    TaskList.push_back(newTask);
    return 0;
}

int TaskMgr::addTask(std::string name, TASKENTRY_VOIDPTR tEntry)
{
    DBG("TaskMgr: added Task '%s'\n", (const char*)name.c_str());
    Task * newTask = new Task(name);
    newTask->setTaskEntry(tEntry,0);//default
    TaskList.push_back(newTask);
    return 0;    
}

int TaskMgr::addTask(std::string name, TASKENTRY_VOID tEntry)
{
    DBG("TaskMgr: added Task '%s'\n", (const char*)name.c_str());
    Task * newTask = new Task(name);
    newTask->setTaskEntry(tEntry);//default
    TaskList.push_back(newTask);
    return 0;    
}

int TaskMgr::runTask(std::string name, void * tParam)
{
    for(unsigned int i = 0; i < TaskList.size(); i++)
    {
        if(TaskList[i]->getName().compare(name)==0)
        {
            if(TaskList[i]->type != TYPE_STRING)return -1;
            TaskList[i]->bActive = true;
            if(tParam != NULL)TaskList[i]->setParam(tParam);
            DBG("starting task %d: '%s'\n", i, name.c_str());
            ThreadLib::TLSignalEvent(TaskMgrEvent);
            return 0;
        }
    }
    LOGERR("TaskMgr::runTask '%s' does not exist.\n", name.c_str());
    return -1;
}

int TaskMgr::runTask(std::string name)
{
    for(unsigned int i = 0; i < TaskList.size(); i++)
    {
        if(TaskList[i]->getName().compare(name)==0)
        {
            if(TaskList[i]->type != TYPE_VOID)return -1;
            TaskList[i]->bActive = true;

            DBG("starting task %d: '%s'\n", i, name.c_str());
            ThreadLib::TLSignalEvent(TaskMgrEvent);
            return 0;
        }
    }
    return -1;
}

int TaskMgr::runTask(std::string name, int * tParam)
{
    for(unsigned int i = 0; i < TaskList.size(); i++)
    {
        if(TaskList[i]->getName().compare(name)==0)
        {
            if(TaskList[i]->type != TYPE_INT)return -1;
            TaskList[i]->bActive = true;
            if(tParam != NULL)TaskList[i]->setParam((void*)tParam);

            DBG("starting task %d: '%s'\n", i, name.c_str());

            ThreadLib::TLSignalEvent(TaskMgrEvent);
            return 0;
        }
    }
    return -1;
}


int TaskMgr::waitForTask(std::string name,int timeout)
{
    int tc = timeout;
    DBG("wait for task '%s'\n", name.c_str());
    for(unsigned int i = 0; i < TaskList.size(); i++)
    {
        if(TaskList[i]->getName().compare(name)==0)
        {
            while((TaskList[i]->bRunning == true) || (TaskList[i]->bActive == true))
            {
                if(tc == 0)return -1;
                tc-=1;
            }
            return 0;
        }
    }
    return -2;
}

int TaskMgr::startTaskLoop()
{
    TaskMgrEvent = ThreadLib::TLCreateEvent();
    TaskLoopStatus = 0x01;

    threadlib_thread_t *taskThread = new threadlib_thread_t;
    taskThread->entryPoint = TaskMgr::taskLoop;
    ThreadLib::TLCreateThread(taskThread);

    return 0;
}

void TaskMgr::taskLoop(void * args)
{
    int loopStatus = TaskMgr::TaskLoopStatus;
    int rc = 0;

    args = args; // remove warning

    DBG("TaskMgr: startet loop\n");

    while(loopStatus)
    {
        allTasksInactive = 1;
        int Tlength = TaskList.size();
        for(int i = 0; i < Tlength; i++)
        {
            if(TaskList[i]->bActive == true)
            {
                TaskList[i]->bRunning = true;
                TaskList[i]->bActive = false;
                allTasksInactive = 0;
                switch(TaskList[i]->type)
                {
                    case TYPE_INT:
                        rc = ((TASKENTRY_INT)TaskList[i]->getEntryPoint())((int*)TaskList[i]->getParam());   
                        break;
                    case TYPE_STRING:
                        rc = ((TASKENTRY_STRING)TaskList[i]->getEntryPoint())((std::string*)TaskList[i]->getParam());
                        break;
                    case TYPE_VOID:
                        rc = ((TASKENTRY_VOID)TaskList[i]->getEntryPoint())();
                        break;
                }
                TaskList[i]->bRunning = false;
                DBG("Task %d returned %d\n", i, rc);
            } 
            //Sleep(1);
        }
        if(allTasksInactive == 1)
        {
            //DBG("gone waiting... zzZZzzZZ\n");
            ThreadLib::TLWaitForEvent(TaskMgrEvent, -1);
            //DBG("woke up\n");
        }
    }
}



