
#ifdef LINUX
#include <unistd.h>
#include <sys/fsuid.h>
#endif

#include "chacolib.h"
#include "Chaco.h"

#include "Dbg.h"
#include "ChFrm.h"
#include "Chameleon.h"
#include "TaskMgr.h"

#ifndef DEBUG
/* #define DEBUG */
#endif

#define LOGERR(...)       logfunc (LOGLVL_ERR, __VA_ARGS__ )
#define LOGVER(...)       logfunc (LOGLVL_VER, __VA_ARGS__ )
#define LOGMSG(...)       logfunc (LOGLVL_MSG, __VA_ARGS__ )
#define DBG(...)          logfunc (LOGLVL_DBG, __VA_ARGS__ )

static int chversion = 0;

int GetChVersion()
{
    return chversion;
}
void SetChVersion(int n)
{
    chversion = n;
}

int logfunc (int lvl, const char * format, ...)
{
    int printed = 0;
    char b[0x1000];

    va_list ap;
    va_start (ap, format);

    b[0]=0;
    vsprintf (b, format, ap);
    va_end (ap);

    switch (lvl) {
        case LOGLVL_ERR:
            dbg::dError (b);
            break;
        case LOGLVL_VER:
            dbg::dVerbose (b);
            break;
        case LOGLVL_MSG:
            dbg::dPrint (b);
            break;
        case LOGLVL_DBG:
            dbg::dDebug (b);
            break;
    }

    return printed;
}

// Define a new application type, each program should derive a class from wxApp
class MyApp : public wxApp
{
public:
    // override base class virtuals
    // ----------------------------

    // this one is called on application startup and is a good place for the app
    // initialization (doing it here and not in the ctor allows to have an error
    // return: if OnInit() returns false, the application terminates)
    virtual bool OnInit();
};

// Create a new application object: this macro will allow wxWidgets to create
// the application object during program execution (it's better than using a
// static object for many reasons) and also declares the accessor function
// wxGetApp() which will return the reference of the right type (i.e. MyApp and
// not wxApp)
IMPLEMENT_APP(MyApp)

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// the application class
// ----------------------------------------------------------------------------

// 'Main program' equivalent: the program execution "starts" here
bool MyApp::OnInit()
{
    int verbose = 0;
    int rc = 0;

    if ( argc == 2 && !wxStricmp(argv[1],  _T("--quiet")) )
    {
        verbose = -1;
    }
    if ( argc == 2 && !wxStricmp(argv[1],  _T("--verbose")) )
    {
        verbose = 1;
    }
    if ( argc == 2 && !wxStricmp(argv[1],  _T("--debug")) )
    {
        verbose = 2;
    }

    chameleon_setlogfunc(logfunc);

    dbg::setVerbose(verbose);

    // create the main application window
    ChFrm * ChacoWin = new ChFrm(NULL,1,wxT("Chaco"),wxDefaultPosition,wxDefaultSize,ChFrm_STYLE);
    dbg::setDebugOutClass(ChacoWin);
    DBG("GUI ready and rest of interface too :-) \n");

    TaskMgr::startTaskLoop(); // Start Taskmanager

    // and show it (the frames, unlike simple controls, are not shown when
    // created initially)
    //ChacoWin->SetWindowStyle(wxBORDER_THEME);
    ChacoWin->Refresh();
    ChacoWin->Show();

#ifdef LINUX
    /* make sure that if the binary is setuid root, the created files will be
       owned by the user running the binary (and not root) */
    {
        int fsuid = setfsuid(getuid());
        int fsgid = setfsgid(getgid());
        LOGVER("fsuid: %d fsgid: %d\n", fsuid, fsgid);
    }
#endif

    rc = ChameleonInit();
    if(rc == 0)
    {
        LOGMSG("Chameleon Ready\n");
        SetChVersion(GetFlashID());
        if(GetChVersion() == -1) {
            LOGERR("GetFlashID failed\n");
        }
    }
    else
    {
        if (GetErrorActive()) {
            LOGERR("%s\n", getError());
        }
        // If no Chameleon is attached -> disable all buttons
        ChacoWin->setButtonStates(false);
    }
    
    DBG("adding tasks\n");

    //Register Tasks
    TaskMgr::addTask("StartCH",(TASKENTRY_INT)StartCH);

    TaskMgr::addTask("FlashCore",(TASKENTRY_VOIDPTR)FlashCore);

    DBG("OnInit done.\n");

    // success: wxApp::OnRun() will be called which will enter the main message
    // loop and the application will run. If we returned false here, the
    // application would exit immediately.
    return true;
}



