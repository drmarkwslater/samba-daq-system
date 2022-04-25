#ifndef SAMBA_EVT_HANDLER_HPP
#define SAMBA_EVT_HANDLER_HPP

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

wxDECLARE_EVENT(CREATE_WINDOW, wxCommandEvent);

class SambaApp;


struct NewWindowConfig
{
    int x;
    int y;
    unsigned int width;
    unsigned int height;
};

class SambaEvtHandler: public wxEvtHandler
{
private:
    SambaApp *theApp_{nullptr};

public:
    void OnCreateWindow(wxCommandEvent& event);
    void SetAppPointer(SambaApp *ptr) {theApp_ = ptr;}

    wxDECLARE_EVENT_TABLE();
};

#endif