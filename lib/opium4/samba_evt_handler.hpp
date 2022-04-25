#ifndef SAMBA_EVT_HANDLER_HPP
#define SAMBA_EVT_HANDLER_HPP

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

class SambaEvtHandler: public wxEvtHandler
{
public:
    void OnCreateWindow(wxCommandEvent& event);

};

#endif