///-----------------------------------------------------------------
///
/// @file      ChFrm.cpp
/// @author    stefan
/// Created:   26.07.2010 09:12:50
/// @section   DESCRIPTION
///            ChFrm class implementation
///
///------------------------------------------------------------------

#include "chacolib.h"
#include "Chaco.h"

#include "ChFrm.h"
#include "Chameleon.h"
#include "BasicFile.h"

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

#define GUI_TIMER_BASE  100
#define GUI_TICKS_PER_SEC (1000 / GUI_TIMER_BASE)
#define GUI_SECTICKS(x) ((1000 * (x)) / GUI_TIMER_BASE)

//----------------------------------------------------------------------------
// ChFrm
//----------------------------------------------------------------------------
//Add Custom Events only in the appropriate block.
//Code added in other places will be removed by wxDev-C++
////Event Table Start
BEGIN_EVENT_TABLE(ChFrm,wxFrame)
	
	EVT_CLOSE(ChFrm::OnClose)
	EVT_TIMER(ID_WXTIMER1,ChFrm::GUIUpdate)
	
	EVT_BUTTON(ID_WXBUTTON2,ChFrm::WxButton2Click)
	EVT_BUTTON(ID_WXBUTTON1,ChFrm::WxButton1Click)

END_EVENT_TABLE()
////Event Table End

ChFrm::ChFrm(wxWindow *parent, wxWindowID id, const wxString &title, const wxPoint &position, const wxSize& size, long style)
: wxFrame(parent, id, title, position, size, style)
{
    CreateGUIControls();
    SetBackgroundColour( wxColour( 250, 250, 0 ) );
}

ChFrm::~ChFrm()
{
}

#define WINDOW_W     691

#define COL_MEMORY   310
#define COL_DEBUG    510

void ChFrm::CreateGUIControls()
{
    char name[0x40];
    sprintf(name, "Chamelon Updater (built %s)", __DATE__);
    SetTitle(wxString::FromAscii(name));
    SetIcon(wxNullIcon);
#ifdef LINUX
    SetSize(8,8,WINDOW_W,501);
#else
    SetSize(8,8,WINDOW_W + 5,501 + 25);
#endif
    Center();

    WxButton2 = new wxButton(this, ID_WXBUTTON2, wxT("Flash .rbf/ROM"), wxPoint(15, 85),  wxSize(105, 35), 0, wxDefaultValidator, wxT("WxButton2"));
    WxButton1 = new wxButton(this, ID_WXBUTTON1, wxT("Abort"),     wxPoint(15, 205), wxSize(105, 35), 0, wxDefaultValidator, wxT("WxButton1"));

    WxEdit3 = new wxTextCtrl(this, ID_WXEDIT3, wxT(""), wxPoint(COL_DEBUG+5, 102), wxSize(121, 19), wxTE_READONLY, wxDefaultValidator, wxT("WxEdit3"));
    WxEdit4 = new wxTextCtrl(this, ID_WXEDIT4, wxT(""), wxPoint(COL_DEBUG+5, 128), wxSize(121, 19), wxTE_READONLY, wxDefaultValidator, wxT("WxEdit4"));
    WxEdit5 = new wxTextCtrl(this, ID_WXEDIT5, wxT(""), wxPoint(COL_DEBUG+5, 154), wxSize(121, 19), wxTE_READONLY, wxDefaultValidator, wxT("WxEdit5"));

    ////GUI Items Creation End

    LogWindow = new wxTextCtrl(this,wxID_ANY,wxT(""), wxPoint(5, 245),wxSize(WINDOW_W - 11,230),wxTE_RICH2 | wxTE_MULTILINE | wxTE_READONLY );
    wxFont *font = new wxFont(wxSystemSettings::GetFont(wxSYS_ANSI_FIXED_FONT));
    font->SetPointSize(11);
#ifdef LINUX
    font->SetFamily(wxFONTFAMILY_MODERN);
    font->SetFaceName(wxT("fixed"));
#endif
    LogWindow->SetDefaultStyle(wxTextAttr(*wxBLACK, *wxWHITE, *font));

    WxGauge1 = new wxGauge(this, ID_WXGAUGE1, 100, wxPoint(5, 480), wxSize(WINDOW_W - 11, 18), wxGA_HORIZONTAL | wxGA_SMOOTH, wxDefaultValidator, wxT("WxGauge1"));
    WxGauge1->SetRange(1000);
    WxGauge1->SetValue(0);

    dbgOut::dout = new std::stringstream(stringstream::out);

    WxTimer1 = new wxTimer();
    WxTimer1->SetOwner(this, ID_WXTIMER1);
    WxTimer1->Start(GUI_TIMER_BASE, wxTIMER_CONTINUOUS);

}

void ChFrm::setButtonStates(bool state)
{
    WxButton1->Enable(state);
    WxButton2->Enable(state);
}

void ChFrm::OnClose(wxCloseEvent& event)
{
    this->Destroy();
}

/*
    Appends text to the log window
*/

void ChFrm::setText(wxDateTime now, std::string text)
{
    static int pos = 0;

    LogWindow->Freeze();
    if (pos == 0) {
//        LogWindow->AppendText(now.FormatTime());
        LogWindow->AppendText(now.Format(wxT("%X:%l")));
        LogWindow->AppendText(wxT(" "));
        pos++;
    }
    LogWindow->AppendText(wxString::FromAscii(text.c_str()));
    if(text.c_str()[strlen(text.c_str()) - 1] == '\n') {
        pos = 0;
        LogWindow->ScrollLines(1);
    }
    LogWindow->ShowPosition( LogWindow->GetLastPosition() );
    LogWindow->Refresh(true, NULL);
    LogWindow->Thaw();
}

/*
    Appends text to the log window
    - since this function may be called by different tasks, some care must be
      taken. only the GUI (main) task should update the log window, so here the
      strings are buffered in a fifo, and the timer callback updates the log.
*/

#define MAXMSG  32

int textoverflow = 0;
int textreadptr = 0;
int textwriteptr = 0;
int textlines = 0;
std::string *textbuffer[MAXMSG] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
wxDateTime timebuffer[MAXMSG];
bool timerrunning = false;

void ChFrm::outPrint(std::string outString)
{
    if (timerrunning) {

        if (textlines == MAXMSG) {
            /* wait */
            if (textoverflow == 0) {
                Sleep(400 * 2);
            }
            if (textlines == MAXMSG) {
#ifdef LINUX
                fprintf(stderr, "ERROR: log text overflow\n");
#endif
                textoverflow = 1;
                return;
            }
        }
        textbuffer[textwriteptr] = new string (outString);
//        timebuffer[textwriteptr] = wxDateTime::Now();
        timebuffer[textwriteptr] = wxDateTime::UNow();
        textwriteptr = (textwriteptr + 1) % MAXMSG;
        textlines++;
    } else {
        setText(wxDateTime::UNow(), outString);
    }
}

void ChFrm::setGauge(int pos)
{
    if ((pos < 0) || (pos > 1000)) {
        pos = 0;
    }
    WxGauge1->SetValue(pos);
}

wxString toWxString(std::string string)
{
    wxString myString = wxString::FromAscii(string.c_str());
    return myString;
}

std::string toStdString(wxString string)
{
    return std::string(string.mb_str());     
}

/*
 * WxTimer
 */

void ChFrm::GUIUpdate(wxTimerEvent& event)
{
    static int found = 0, busy = 0;
    static int count;

    timerrunning = true;

    /* update the progress bar */
    chameleon_setprogressfunc(1000, chameleon_progresshelper);
    setGauge(chameleon_getprogresspercent());

    /* update the log window */
    if (textlines) {
        if (textoverflow) {
            setText(wxDateTime::UNow(), "ERROR: log overflow\n");
        }
        while (textlines) {
            if (textbuffer[textreadptr]) {
                setText(timebuffer[textreadptr],*textbuffer[textreadptr]);
                delete textbuffer[textreadptr];
                textbuffer[textreadptr] = NULL;
                textreadptr = (textreadptr + 1) % MAXMSG;
                textlines--;
            } else {
                setText(wxDateTime::UNow(), "ERROR: log not in sync\n");
                break;
            }
        }
        textlines = 0;
        textoverflow = 0;
    }

    if(isLocked()) {
//        DBG("BUSY (%d)\n", isPresent());
        /* Chaco is busy */
        if (isPresent()) {
            found = 1; busy = 1;
        } else {
            LOGERR("ChFrm::GUIUpdate BUG - should not reach here! (USB locked but no chameleon detected)\n");
            /* UnlockAccess(); */ /* HACK! */
        }
    }
    else
    {
        busy = 0;
        /*
            if chaco is idle, check chameleon every 2 seconds
        */
        count++; if (count > GUI_SECTICKS(2)) {
            count=0;
    //        DBG("IDLE (%d)\n", isPresent());
            if (isPresent()) {
                if(CheckChameleon() == 0) {
                    found = 1;
                } else {
                    found = 0;
                }
            } else {
                /*
                    chameleon was previously disconnected, try to reinit
                */
                if(ChameleonInit() == 0) {
                    found = 1;
                } else {
                    found = 0;
                }
            }
        }
    }

    if (found) {
        WxEdit3->SetValue(isBricked() ? wxT("Core is invalid") : wxT("Core is valid"));
        WxEdit4->SetValue(isUsbCap() ? wxT("Usb capable") : wxT("Not Usb capable"));
        WxEdit5->SetValue(isSpiActive() ? wxT("Spi active") : wxT("Spi inactive"));
        if (busy) {
            setButtonStates(false);
        } else {
            setGauge(0);
            setButtonStates(true);
        }
    } else {
        WxEdit3->SetValue(wxT("No"));
        WxEdit4->SetValue(wxT("Chameleon"));
        WxEdit5->SetValue(wxT("present"));
        setButtonStates(false);
    }
}


void ChFrm::WxButton1Click(wxCommandEvent& event)
{
    exit(EXIT_SUCCESS);
}

/*
 * Writes a rbf file and an optianal core file to the flash
 */
void ChFrm::WxButton2Click(wxCommandEvent& event)
{
    int rc;
    wxFileDialog *OpenRomFileDialog = NULL;
    wxFileDialog *OpenCoreFileDialog = NULL;
    bool flashRom = true;
    int corenum = 0;

    setGauge(0);
    core_flash_info_t * cfi = new core_flash_info_t(); // FIXME: memory leak!

    OpenCoreFileDialog =  new wxFileDialog(this, wxT("Choose core"), wxT(""), wxT(""), wxT("Core files (*.rbf)|*.rbf|all files (*.*)|*.*"), wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    rc = OpenCoreFileDialog->ShowModal();
    if(rc == wxID_CANCEL) {
        delete OpenCoreFileDialog;
        return;
    }

    cfi->coreName = new std::string(); // FIXME: memory leak!
    (*cfi->coreName) =  toStdString(OpenCoreFileDialog->GetPath());

    if(flashRom)
    {
        OpenRomFileDialog =  new wxFileDialog(this, wxT("Choose ROM file"), wxT(""), wxT(""), wxT("Binary files (*.bin)|*.bin|all files (*.*)|*.*"), wxFD_OPEN | wxFD_FILE_MUST_EXIST);
        rc = OpenRomFileDialog->ShowModal();
        if(rc == wxID_CANCEL) {
            delete OpenRomFileDialog;
            return;
        }
        cfi->romName = new std::string(); // FIXME: memory leak!
        (*cfi->romName) =  toStdString(OpenRomFileDialog->GetPath());

    }

    cfi->corenum = 0;

    TaskMgr::runTask("FlashCore",(cfi));

    delete OpenCoreFileDialog;
    if (OpenRomFileDialog) delete OpenRomFileDialog;
}


