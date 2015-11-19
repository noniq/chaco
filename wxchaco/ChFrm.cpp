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

//Do not add custom headers between
//Header Include Start and Header Include End
//wxDev-C++ designer will remove them
////Header Include Start
////Header Include End

int flash_additional_roms = 0;

//----------------------------------------------------------------------------
// ChFrm
//----------------------------------------------------------------------------
//Add Custom Events only in the appropriate block.
//Code added in other places will be removed by wxDev-C++
////Event Table Start
BEGIN_EVENT_TABLE(ChFrm,wxFrame)
	////Manual Code Start
        EVT_SPINCTRL(ID_WXSPINCTRL1,ChFrm::WxSpinCtrl1Click)
        EVT_SPIN_UP(ID_WXSPINCTRL1,ChFrm::WxSpinCtrl1Click)
        EVT_SPIN_DOWN(ID_WXSPINCTRL1,ChFrm::WxSpinCtrl1Click)
	////Manual Code End
	
	EVT_CLOSE(ChFrm::OnClose)
	EVT_ACTIVATE(ChFrm::ChFrmActivate)
        EVT_BUTTON(ID_STARTBOOTLOADER,ChFrm::StartBootloaderClick)
	EVT_TIMER(ID_WXTIMER1,ChFrm::GUIUpdate)
	EVT_BUTTON(ID_READSLOT,ChFrm::readSlotClick)
	EVT_BUTTON(ID_WRITESLOT,ChFrm::writeSlotClick)
	EVT_BUTTON(ID_WRITEIMGBUTTON,ChFrm::writeImgButtonClick)
	EVT_BUTTON(ID_READIMGBUTTON,ChFrm::readImgButtonClick)
	
        EVT_SPINCTRL(ID_WXSPINCTRL2,ChFrm::setjtagslotClick)
        EVT_SPIN_UP(ID_WXSPINCTRL2,ChFrm::setjtagslotClick)
	EVT_SPIN_DOWN(ID_WXSPINCTRL2,ChFrm::setjtagslotClick)
	EVT_BUTTON(ID_WXBUTTON7,ChFrm::WxButton7Click)
	EVT_BUTTON(ID_MEMREADER,ChFrm::memreaderClick)
	
	EVT_TEXT(ID_WXEDIT1,ChFrm::WxEdit1Updated)
//	EVT_BUTTON(ID_WXBUTTON5,ChFrm::WxButton5Click)
	EVT_BUTTON(ID_WXBUTTON2,ChFrm::WxButton2Click)
	EVT_BUTTON(ID_WXBUTTON1,ChFrm::WxButton1Click)

        EVT_CHECKBOX(ID_WXCHECKBOX1,ChFrm::WxCheckBox1Click)
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
    sprintf(name, "Chaco (beta %s)", __DATE__);
    SetTitle(wxString::FromAscii(name));
    SetIcon(wxNullIcon);
#ifdef LINUX
    SetSize(8,8,WINDOW_W,501);
#else
    SetSize(8,8,WINDOW_W + 5,501 + 25);
#endif
    Center();

    //Do not add custom code between
    //GUI Items Creation Start and GUI Items Creation End
    //wxDev-C++ designer will remove them.
    //Add the custom code before or after the blocks
    ////GUI Items Creation Start
    WxStaticText4 = new wxStaticText(this, ID_WXSTATICTEXT4, wxT("Slot Management"), wxPoint(16, 8), wxDefaultSize, 0, wxT("WxStaticText4"));
    WxStaticText4->SetFont(wxFont(14, wxSWISS, wxNORMAL, wxNORMAL, false));

    WxSpinCtrl1   = new wxSpinCtrl(this, ID_WXSPINCTRL1,     wxT("0"),      wxPoint(15, 55), wxSize(45, 25),wxSP_ARROW_KEYS | wxSP_WRAP, 0, 15, 0);
    WxStaticText2 = new wxStaticText(this, ID_WXSTATICTEXT2, wxT("Selected slot"), wxPoint(70, 55+2), wxDefaultSize, 0, wxT("WxStaticText2"));

    WxButton2 = new wxButton(this, ID_WXBUTTON2, wxT("Flash .rbf/ROM"), wxPoint(15, 85),  wxSize(105, 35), 0, wxDefaultValidator, wxT("WxButton2"));
    writeSlot = new wxButton(this, ID_WRITESLOT, wxT("Write Slot"),     wxPoint(15, 125), wxSize(105, 35), 0, wxDefaultValidator, wxT("writeSlot"));
    readSlot  = new wxButton(this, ID_READSLOT,  wxT("Read Slot"),      wxPoint(15, 165), wxSize(105, 35), 0, wxDefaultValidator, wxT("readSlot"));

    WxCheckBox1    = new wxCheckBox(this, ID_WXCHECKBOX1,  wxT("Flash additional ROM"), wxPoint(135 - 5, 85), wxSize(165, 35), wxCHK_2STATE, wxDefaultValidator, wxT("WxCheckBox1"));
    writeImgButton = new wxButton(this, ID_WRITEIMGBUTTON, wxT("Write image"),          wxPoint(135, 125),    wxSize(105, 35), 0, wxDefaultValidator, wxT("writeImgButton"));
    readImgButton  = new wxButton(this, ID_READIMGBUTTON,  wxT("Read image"),           wxPoint(135, 165),    wxSize(105, 35), 0, wxDefaultValidator, wxT("readImgButton"));

    WxButton1 = new wxButton(this, ID_WXBUTTON1, wxT("Start core"),     wxPoint(15, 205), wxSize(105, 35), 0, wxDefaultValidator, wxT("WxButton1"));


    WxStaticText5 = new wxStaticText(this, ID_WXSTATICTEXT5, wxT("Memory Access"), wxPoint(COL_MEMORY+1, 8), wxDefaultSize, 0, wxT("WxStaticText5"));
    WxStaticText5->SetFont(wxFont(14, wxSWISS, wxNORMAL, wxNORMAL, false));

    WxStaticText1 = new wxStaticText(this, ID_WXSTATICTEXT1, wxT("Address:"),   wxPoint(COL_MEMORY+5, 55), wxDefaultSize, 0, wxT("WxStaticText1"));
    WxEdit1       = new wxTextCtrl(this, ID_WXEDIT1,         wxT("0x00000000"), wxPoint(COL_MEMORY+65, 55), wxSize(105, 25), 0, wxDefaultValidator, wxT("WxEdit1"));

    WxStaticText7 = new wxStaticText(this, ID_WXSTATICTEXT7, wxT("Size:"),      wxPoint(COL_MEMORY+5, 85), wxDefaultSize, 0, wxT("WxStaticText1"));
    WxEdit2       = new wxTextCtrl(this, ID_WXEDIT2,         wxT("0x00000000"), wxPoint(COL_MEMORY+65, 85), wxSize(105, 25), 0, wxDefaultValidator, wxT("WxEdit2"));

    memreader = new wxButton(this, ID_MEMREADER, wxT("Read memory"), wxPoint(COL_MEMORY+5, 125), wxSize(105, 35), 0, wxDefaultValidator, wxT("memreader"));

    WxButton7 = new wxButton(this, ID_WXBUTTON7, wxT("Write memory"),wxPoint(COL_MEMORY+5, 165), wxSize(105, 35), 0, wxDefaultValidator, wxT("WxButton7"));


    WxStaticText6 = new wxStaticText(this, ID_WXSTATICTEXT6, wxT("Debugging"), wxPoint(COL_DEBUG+1, 9), wxDefaultSize, 0, wxT("WxStaticText6"));
    WxStaticText6->SetFont(wxFont(14, wxSWISS, wxNORMAL, wxNORMAL, false));

    StartBootloader = new wxButton(this, ID_STARTBOOTLOADER, wxT("Start Bootloader"), wxPoint(COL_DEBUG+5, 55), wxSize(125, 35), 0, wxDefaultValidator, wxT("StartBootloader"));

//    WxButton5 = new wxButton(this, ID_WXBUTTON5, wxT("Search Chameleon"), wxPoint(620, 55), wxSize(120, 40), 0, wxDefaultValidator, wxT("WxButton5"));

    WxEdit3 = new wxTextCtrl(this, ID_WXEDIT3, wxT(""), wxPoint(COL_DEBUG+5, 102), wxSize(121, 19), wxTE_READONLY, wxDefaultValidator, wxT("WxEdit3"));
    WxEdit4 = new wxTextCtrl(this, ID_WXEDIT4, wxT(""), wxPoint(COL_DEBUG+5, 128), wxSize(121, 19), wxTE_READONLY, wxDefaultValidator, wxT("WxEdit4"));
    WxEdit5 = new wxTextCtrl(this, ID_WXEDIT5, wxT(""), wxPoint(COL_DEBUG+5, 154), wxSize(121, 19), wxTE_READONLY, wxDefaultValidator, wxT("WxEdit5"));

    WxSpinCtrl2 = new wxSpinCtrl(this, ID_WXSPINCTRL2, wxT("0"), wxPoint(COL_DEBUG+5, 185), wxSize(45, 25), wxSP_ARROW_KEYS, 0, 15, 0);
    WxStaticText3 = new wxStaticText(this, ID_WXSTATICTEXT3, wxT("Select JTAG slot"), wxPoint(COL_DEBUG+55, 185+2), wxDefaultSize, 0, wxT("WxStaticText3"));

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

    WxCheckBox1->SetValue(1);
}

void ChFrm::setButtonStates(bool state)
{
    readSlot->Enable(state);
    writeSlot->Enable(state);
    writeImgButton->Enable(state);
    readImgButton->Enable(state);
    WxButton1->Enable(state);
    WxButton2->Enable(state);

    if (isUsbCap()) {
        memreader->Enable(state);
        WxButton7->Enable(state);
        WxEdit1->Enable(state);
        WxEdit2->Enable(state);
    } else {
        memreader->Enable(false);
        WxButton7->Enable(false);
        WxEdit1->Enable(false);
        WxEdit2->Enable(false);
    }

    WxSpinCtrl1->Enable(state);
    WxSpinCtrl2->Enable(state);

    WxCheckBox1->Enable(state);

}

void ChFrm::OnClose(wxCloseEvent& event)
{
    this->Destroy();
}

/*
 * ChFrmActivate
 */
void ChFrm::ChFrmActivate(wxActivateEvent& event)
{
    // insert your code here
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

#if 0
/*
*   Search Chameleon and init libusb
*/
void ChFrm::WxButton5Click(wxCommandEvent& event)
{
    int rc;
    UnlockAccess(); /* HACK! */
    rc = ChameleonInit();
    if(rc == 0)setButtonStates(true);
    else setButtonStates(false);
}
#endif

/*
 * WxButton1Click
 */
void ChFrm::WxButton1Click(wxCommandEvent& event)
{
    coreNumber = WxSpinCtrl1->GetValue(); //Low byte flags , highbyte the corenumber to flash
    TaskMgr::runTask("StartCH",&coreNumber);
}

void ChFrm::WxSpinCtrl1Click(wxSpinEvent& event)
{
    int corenum = WxSpinCtrl1->GetValue();
    DBG("set flash slot: %d\n", corenum);
    if (corenum == 0) {
        WxCheckBox1->SetValue(1);
    } else {
        WxCheckBox1->SetValue(flash_additional_roms);
    }
}

void ChFrm::WxCheckBox1Click(wxCommandEvent& event)
{
    int flashrom = WxCheckBox1->GetValue();
    int corenum = WxSpinCtrl1->GetValue();
    DBG("flash additional ROM: %d\n", flashrom);
    if (corenum != 0) {
        flash_additional_roms = flashrom;
    }
}

/*
 * WxEdit1Updated
 */
void ChFrm::WxEdit1Updated(wxCommandEvent& event)
{
    // insert your code here
}

/*
 * setjtagslotClick
 */
void ChFrm::setjtagslotClick(wxSpinEvent& event)
{
    //int slot = WxSpinCtrl2->GetValue();
    int slot = event.GetPosition();
    if(slot < 0 || slot >= 16)return;

    if(setJTAGSlot(slot) == -1) {
        /* */
    }
}

/*
 * Writes a rbf file and an optianal core file to the flash
 */
void ChFrm::WxButton2Click(wxCommandEvent& event)
{
    int rc;
    wxFileDialog *OpenRomFileDialog = NULL;
    wxFileDialog *OpenCoreFileDialog = NULL;

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

    bool flashRom = WxCheckBox1->GetValue();
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

    cfi->corenum = WxSpinCtrl1->GetValue();

    TaskMgr::runTask("FlashCore",(cfi));

    delete OpenCoreFileDialog;
    if (OpenRomFileDialog) delete OpenRomFileDialog;
}

/*
 * Reads Chameleon memory
 */
void ChFrm::memreaderClick(wxCommandEvent& event)
{
    long address;
    long size;

    wxFileDialog *OpenFileDialog =  new wxFileDialog(this, wxT("target filename"), wxT(""), wxT(""), wxT("Binary files (*.bin)|*.bin|all files (*.*)|*.*"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    wxString * targetFileName;
    int rc = OpenFileDialog->ShowModal();

    if(rc == wxID_OK)
    {
        targetFileName = new wxString(OpenFileDialog->GetPath()); /* FIXME: memory leak! */
    } else {
        delete OpenFileDialog;
        return;
    }

    std::string * target = new std::string(); /* FIXME: memory leak! */
    *target = toStdString(*targetFileName);

    wxString adrStr = WxEdit1->GetValue();
    adrStr.ToLong(&address,0);
    wxString sizeStr = WxEdit2->GetValue();
    sizeStr.ToLong(&size,0);

    slot_memory_info_t *meminfo = new slot_memory_info_t; /* FIXME: memory leak! */
    meminfo->address = address;
    meminfo->length = size;
    meminfo->filename = target;

    TaskMgr::runTask("ReadMemory",(meminfo));

    delete OpenFileDialog;
}

/*
 * Write a binary file to the Chameleon memory
 */
void ChFrm::WxButton7Click(wxCommandEvent& event)
{
    long address;
    int size = 0;

    wxFileDialog *OpenFileDialog =  new wxFileDialog(this, wxT("source filename"), wxT(""), wxT(""), wxT("Binary files (*.bin)|*.bin|all files (*.*)|*.*"), wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    wxString * targetFileName;
    int rc = OpenFileDialog->ShowModal();

    if(rc == wxID_OK)
    {
        targetFileName = new wxString(OpenFileDialog->GetPath()); /* FIXME: memory leak! */
    } else {
        delete OpenFileDialog;
        return;
    }

    std::string * target = new std::string(); /* FIXME: memory leak! */
    *target = toStdString(*targetFileName);

    wxString adrStr = WxEdit1->GetValue();
    adrStr.ToLong(&address,0);

    size = FileLength((char*)targetFileName->char_str());

    slot_memory_info_t *meminfo = new slot_memory_info_t; /* FIXME: memory leak! */
    meminfo->address = address;
    meminfo->length = size;
    meminfo->filename = target;

    TaskMgr::runTask("WriteMemory",(meminfo));

    delete OpenFileDialog;
}

/*
 * write file to flash (16MB)
 */
void ChFrm::writeImgButtonClick(wxCommandEvent& event)
{
    wxFileDialog *OpenFileDialog =  new wxFileDialog(this, wxT("select flash image"), wxT(""), wxT(""), wxT("Binary files (*.bin)|*.bin|all files (*.*)|*.*"), wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    int rc = OpenFileDialog->ShowModal();
    if(rc == wxID_OK)
    {
        std::string * fn = new std::string(toStdString(OpenFileDialog->GetPath()));  /* FIXME: memory leak! */
        TaskMgr::runTask("WriteImage",(void*)fn);
    }
    delete OpenFileDialog;
}

/*
 * read image (16MB) from flash to file
 */
void ChFrm::readImgButtonClick(wxCommandEvent& event)
{
    wxFileDialog *OpenFileDialog =  new wxFileDialog(this, wxT("select flash image"), wxT(""), wxT(""), wxT("Binary files (*.bin)|*.bin|all files (*.*)|*.*"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    int rc = OpenFileDialog->ShowModal();
    if(rc == wxID_OK)
    {
        std::string * fn = new std::string(toStdString(OpenFileDialog->GetPath())); /* FIXME: memory leak! */
        TaskMgr::runTask("ReadImage",(void*)fn);
    }
    delete OpenFileDialog;
}

/*
 * write file to flash slot (1MB)
 */
void ChFrm::writeSlotClick(wxCommandEvent& event)
{
    wxFileDialog *OpenFileDialog =  new wxFileDialog(this, wxT("select flash slot image"), wxT(""), wxT(""), wxT("Binary files (*.bin)|*.bin|all files (*.*)|*.*"), wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    int rc = OpenFileDialog->ShowModal();
    if(rc == wxID_OK)
    {
        std::string * fn = new std::string(toStdString(OpenFileDialog->GetPath()));   /* FIXME: memory leak! */
        slot_info_t * sli = new slot_info_t();  /* FIXME: memory leak! */
        sli->filename = fn;
        sli->corenum = WxSpinCtrl1->GetValue();
        TaskMgr::runTask("WriteSlot",(void*)sli);
    }
    delete OpenFileDialog;
}

/*
 * read slot (1MB) from flash to file
 */
void ChFrm::readSlotClick(wxCommandEvent& event)
{
    wxFileDialog *OpenFileDialog =  new wxFileDialog(this, wxT("select flash slot image"), wxT(""), wxT(""), wxT("Binary files (*.bin)|*.bin|all files (*.*)|*.*"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    int rc = OpenFileDialog->ShowModal();
    if(rc == wxID_OK)
    {
        std::string * fn = new std::string(toStdString(OpenFileDialog->GetPath())); /* FIXME: memory leak! */
        slot_info_t * sli = new slot_info_t(); /* FIXME: memory leak! */
        sli->filename = fn;
        sli->corenum = WxSpinCtrl1->GetValue();
        TaskMgr::runTask("ReadSlot",(void*)sli);
    }
    delete OpenFileDialog;
}

/*
 * StartBootloaderClick
 */
void ChFrm::StartBootloaderClick(wxCommandEvent& event)
{
    startBootloader();
}

