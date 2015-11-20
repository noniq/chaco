
#ifndef TASK_H_
#define TASK_H_

#include <iostream>
#include <string>

//defines Task types and utilities for TaskMgr
typedef int (*TASKENTRY_INT)(int*);
typedef int (*TASKENTRY_VOID)(void);
typedef int (*TASKENTRY_VOIDPTR)(void*);
typedef int (*TASKENTRY_STRING)(std::string*);

#define TYPE_INT 0
#define TYPE_STRING 1
#define TYPE_VOID 3

class Task
{
    private:
        std::string taskName;
        void * pParam;
      
        void * taskEntryPtr; 
        
    public:
        int type;  
        bool bActive;
        bool bRunning;
        bool bShouldHaveOwnThread;
        
        void * getEntryPoint()
        {
            return taskEntryPtr;
        }
        
        void * getParam()
        {
            return pParam;
        }
        
        Task(std::string name)
        {
            taskName = name;    
            bActive = false;
            bRunning = false;
        }
        
        ~Task(){}
        
        void setTaskEntry(TASKENTRY_INT entryPtr,int * p)
        {
            type = TYPE_INT;
            pParam = (void*)p;
            taskEntryPtr = (void*)entryPtr;
        }
        
        void setTaskEntry(TASKENTRY_VOID entryPtr)
        {
            type = TYPE_VOID;
            pParam = (void*)0;
            taskEntryPtr = (void*)entryPtr;
        }
                   
        void setTaskEntry(TASKENTRY_VOIDPTR entryPtr,void * p)
        {
            type = TYPE_STRING;
            pParam = (void*)p;
            taskEntryPtr = (void*)entryPtr;    
        }
        
        void setParam(void * p)
        {
            pParam = p;
        }
        
        std::string getName()
        {
            return taskName;
        }

};

#endif

