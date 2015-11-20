
#ifndef _TASKMGR_
#define _TASKMGR_

#include <vector>

#include <string>
#include "Task.h"

#include "Threading.h"

/* Include File of the taskmanager.
* It provides a basic signal and slot principle
* You can register a function as task and call it asynchronly by its name
* 
*/

class TaskMgr
{
    public:
        //Different methods for registering tasks depending on their parameters
       static int addTask(std::string name, TASKENTRY_INT tEntry);
       static int addTask(std::string name, TASKENTRY_VOIDPTR tEntry);
       static int addTask(std::string name, TASKENTRY_VOID tEntry);

       //Different methods for calling a specific task
       static int runTask(std::string name, void * tParam);
       static int runTask(std::string name, int * tParam);
       static int runTask(std::string name);

       //has to be called before any task is called.
       static int startTaskLoop();
       static int waitForTask(std::string name,int timeout);

    private:
        static tHANDLE hTaskThread;
        static int TaskLoopStatus;
        static void taskLoop(void * args);
        static tHANDLE TaskMgrEvent;

        static int allTasksInactive;

        static std::vector<Task*> TaskList;
};
#endif
