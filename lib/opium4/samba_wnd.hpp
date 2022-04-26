#ifndef SAMBA_WND_HPP
#define SAMBA_WND_HPP

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include <string>
#include "opium_wx_interface.h"

wxDECLARE_EVENT(SET_WND_TITLE, wxCommandEvent);
wxDECLARE_EVENT(REQUEST_CLOSE, wxCommandEvent);

class SambaApp;

class SambaWnd: public wxDialog
{
public:
    SambaWnd(const wxString& title, const wxPoint& pos, const wxSize& size);

    bool isPainting();
    void RequestUpdate();
    void OnTimer(wxTimerEvent& event);
    void IgnoreNextMouseRelease();
    void SetSambaApp(SambaApp *app) {theApp_ = app;}

private:
    void OnSize(wxSizeEvent& event);
    void OnMove(wxMoveEvent& event);
    void OnPaint(wxPaintEvent& event);
    void OnMouseDown(wxMouseEvent& event);
    void OnMouseUp(wxMouseEvent& event);
    void OnFocus(wxFocusEvent& event);
    void OnKeyChar(wxKeyEvent& event);
    void OnRequestUpdate(wxCommandEvent& event);
    void OnSetWndTitle(wxCommandEvent& event);
    void OnRequestClose(wxCommandEvent& event);
    wxDECLARE_EVENT_TABLE();

    bool is_painting{false};
    wxTimer timer_;
    wxPoint mousePos_;
    bool ignoreMouseRelease_{false};
    SambaEventWx lastMouseButton_{SMBWX_MOUSE_LEFT_DOWN};
    SambaApp *theApp_;
};

#endif