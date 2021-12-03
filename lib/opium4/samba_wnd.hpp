#ifndef SAMBA_WND_HPP
#define SAMBA_WND_HPP

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include <string>

class SambaWnd: public wxDialog
{
public:
    SambaWnd(const wxString& title, const wxPoint& pos, const wxSize& size);

    bool isPainting();
    void RequestUpdate();
    void OnTimer(wxTimerEvent& event);

private:
    void OnSize(wxSizeEvent& event);
    void OnMove(wxMoveEvent& event);
    void OnPaint(wxPaintEvent& event);
    void OnMouseDown(wxMouseEvent& event);
    void OnMouseUp(wxMouseEvent& event);
    void OnFocus(wxFocusEvent& event);
    void OnKeyChar(wxKeyEvent& event);
    void OnRequestUpdate(wxCommandEvent& event);
    wxDECLARE_EVENT_TABLE();

    bool is_painting{false};
    wxTimer timer_;
    wxPoint mousePos_;
};

#endif