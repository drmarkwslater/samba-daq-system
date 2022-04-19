#ifdef WXWIDGETS

#include <samba_wnd.hpp>
#include <samba_app.hpp>
#include <opium_wx_interface.h>

// create a custom event to request a refresh OUTSIDE the main GUI thread
wxDEFINE_EVENT(REQUEST_UPDATE, wxCommandEvent);

wxBEGIN_EVENT_TABLE(SambaWnd, wxDialog)
    EVT_SIZE(SambaWnd::OnSize)
    EVT_MOVE(SambaWnd::OnMove)
    EVT_PAINT(SambaWnd::OnPaint)
    EVT_LEFT_DOWN(SambaWnd::OnMouseDown)
    EVT_LEFT_UP(SambaWnd::OnMouseUp)
    EVT_RIGHT_DOWN(SambaWnd::OnMouseDown)
    EVT_RIGHT_UP(SambaWnd::OnMouseUp)
    EVT_SET_FOCUS(SambaWnd::OnFocus)
    EVT_CHAR_HOOK(SambaWnd::OnKeyChar)
    EVT_TIMER(1, SambaWnd::OnTimer)
    EVT_COMMAND(wxID_ANY, REQUEST_UPDATE, SambaWnd::OnRequestUpdate)
wxEND_EVENT_TABLE()

void WndEventNewWx(struct SambaWnd *w, enum SambaEventWx type, int x, int y, int h, int v);

SambaWnd::SambaWnd(const wxString& title, const wxPoint& pos, const wxSize& size)
        : wxDialog(NULL, wxID_ANY, title, pos, size), timer_(this, 1)
{
    SetBackgroundStyle(wxBG_STYLE_CUSTOM);

    // create a tiny panel to make sure Key events are captured
    // Should probably draw to this instead of the raw window
    wxPanel* mainPane = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(1, 1), wxWANTS_CHARS);
}

void SambaWnd::OnTimer(wxTimerEvent& /*event*/)
{
    WndEventNewWx(this, lastMouseButton_, mousePos_.x, mousePos_.y, 0, 0);
}

void SambaWnd::OnSize(wxSizeEvent& /*event*/)
{
    const wxPoint ps = GetScreenPosition();
    const wxSize sz = GetClientSize();
    WndEventNewWx(this, SMBWX_CONFIG, ps.x, ps.y, sz.GetWidth(), sz.GetHeight());
}

void SambaWnd::OnMove(wxMoveEvent& /*event*/)
{
    const wxPoint ps = GetScreenPosition();
    const wxSize sz = GetClientSize();
    WndEventNewWx(this, SMBWX_CONFIG, ps.x, ps.y, sz.GetWidth(), sz.GetHeight());
}

void SambaWnd::OnPaint(wxPaintEvent& /*event*/)
{
    LockPaintEvents();
    is_painting = true;
    WndEventNewWx(this, SMBWX_PAINT, 0, 0, 0, 0);
    is_painting = false;
    UnlockPaintEvents();
}

void SambaWnd::IgnoreNextMouseRelease()
{
    ignoreMouseRelease_ = true;
}

void SambaWnd::OnMouseDown(wxMouseEvent& event)
{
    timer_.StartOnce(300);
    mousePos_.x = event.GetX();
    mousePos_.y = event.GetY();

    ignoreMouseRelease_ = false;
    if (event.LeftDown())
        lastMouseButton_ = SMBWX_MOUSE_LEFT_DOWN;
    else if (event.RightDown())
        lastMouseButton_ = SMBWX_MOUSE_RIGHT_DOWN;
}

void SambaWnd::OnMouseUp(wxMouseEvent& event)
{
    if (timer_.IsRunning())
    {
        timer_.Stop();
    }

    if (ignoreMouseRelease_)
    {
        ignoreMouseRelease_ = false;
        return;
    }

    if (event.LeftUp())
        WndEventNewWx(this, SMBWX_MOUSE_LEFT_UP, event.GetX(), event.GetY(), 0, 0);
    else if (event.RightUp())
        WndEventNewWx(this, SMBWX_MOUSE_RIGHT_UP, event.GetX(), event.GetY(), 0, 0);
}

void SambaWnd::OnFocus(wxFocusEvent& /*event*/)
{
    WndEventNewWx(this, SMBWX_FOCUS, 0, 0, 0, 0);
}

void SambaWnd::OnKeyChar(wxKeyEvent& event)
{
    WndEventNewWx(this, SMBWX_KEY, event.GetKeyCode(), 0, 0, 0);
}

bool SambaWnd::isPainting()
{
    return is_painting;
}

void SambaWnd::RequestUpdate()
{
    wxCommandEvent event(REQUEST_UPDATE);
    wxPostEvent(this, event);
}

void SambaWnd::OnRequestUpdate(wxCommandEvent& event)
{
    Refresh();
}

#endif //WXWIDGETS