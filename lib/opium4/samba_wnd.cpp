#ifdef WXWIDGETS

#include <samba_wnd.hpp>
#include <samba_app.hpp>
#include <opium_wx_interface.h>
#include <thread>
#include <chrono>

// create a custom event to request a refresh OUTSIDE the main GUI thread
wxDEFINE_EVENT(REQUEST_UPDATE, wxCommandEvent);
wxDEFINE_EVENT(SET_WND_TITLE, wxCommandEvent);
wxDEFINE_EVENT(REQUEST_CLOSE, wxCommandEvent);
wxDEFINE_EVENT(RUN_MODAL, wxCommandEvent);

wxBEGIN_EVENT_TABLE(SambaWnd, wxDialog)
    EVT_CLOSE(SambaWnd::OnClose)
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
    EVT_ENTER_WINDOW(SambaWnd::OnMouseEnter)
    EVT_COMMAND(wxID_ANY, REQUEST_UPDATE, SambaWnd::OnRequestUpdate)
    EVT_COMMAND(wxID_ANY, SET_WND_TITLE, SambaWnd::OnSetWndTitle)
    EVT_COMMAND(wxID_ANY, REQUEST_CLOSE, SambaWnd::OnRequestClose)
    EVT_COMMAND(wxID_ANY, RUN_MODAL, SambaWnd::OnRunModal)
wxEND_EVENT_TABLE()

void WndEventNewWx(struct SambaWnd *w, enum SambaEventWx type, int x, int y, int h, int v);

SambaWnd::SambaWnd(const wxString& title, const wxPoint& pos, const wxSize& size)
        : wxDialog(NULL, wxID_ANY, title, pos, size, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER), timer_(this, 1)
{
    SetBackgroundStyle(wxBG_STYLE_CUSTOM);

    // create a tiny panel to make sure Key events are captured
    // Should probably draw to this instead of the raw window
    wxPanel* mainPane = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(1, 1), wxWANTS_CHARS);

    reqCursor_ = new wxCursor(wxCURSOR_ARROW);
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

void SambaWnd::OnMouseEnter(wxMouseEvent& /*event*/)
{
    ::wxSetCursor(*reqCursor_);
}

void SambaWnd::ExecModal()
{
    // prepare and send the event
    modalDone_ = false;
    wxCommandEvent event(RUN_MODAL);
    wxQueueEvent(this, event.Clone());

    // wait for completion
    while(!modalDone_.load())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void SambaWnd::OnRunModal(wxCommandEvent& /*event*/)
{
    Show(false);
    ShowModal();

    modalDone_ = true;
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

void SambaWnd::OnSetWndTitle(wxCommandEvent& event)
{
    SetLabel(event.GetString());
}

void SambaWnd::OnRequestClose(wxCommandEvent& event)
{
    MenuClose();
}

void SambaWnd::MenuClose()
{
    menuClose_ = true;
    Close();
}

void SambaWnd::OnClose(wxCloseEvent& event)
{
    // closing from a menu or samba request so just destroy the window
    if (menuClose_)
    {
        theApp_->RemoveWindow(this);
        event.Skip();
        return;
    }

    // OS request to close so send an event instead
    // should probably *not* do this if DAQ is running....
    // Also, this doesn't seem to work with modal dialogs properly so just disable it in that instance
    if (IsModal() && event.CanVeto())
    {
        event.Veto();
        return;
    } else {
        WndEventNewWx(this, SMBWX_DELETE, 0, 0, 0, 0);
    }
}

void SambaWnd::SetSambaCursor(wxCursor *c)
{
    reqCursor_ = c;
}

#endif //WXWIDGETS