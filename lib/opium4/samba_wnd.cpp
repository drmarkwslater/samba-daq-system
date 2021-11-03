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
    EVT_SET_FOCUS(SambaWnd::OnFocus)
    EVT_COMMAND(wxID_ANY, REQUEST_UPDATE, SambaWnd::OnRequestUpdate)
wxEND_EVENT_TABLE()

void WndEventNewWx(struct SambaWnd *w, enum SambaEventWx type, int x, int y, int h, int v);

SambaWnd::SambaWnd(const wxString& title, const wxPoint& pos, const wxSize& size)
        : wxDialog(NULL, wxID_ANY, title, pos, size)
{
    SetBackgroundStyle(wxBG_STYLE_CUSTOM);
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
    is_painting = true;
    WndEventNewWx(this, SMBWX_PAINT, 0, 0, 0, 0);
    is_painting = false;
}

void SambaWnd::OnMouseDown(wxMouseEvent& event)
{
    WndEventNewWx(this, SMBWX_MOUSE_LEFT_DOWN, event.GetX(), event.GetY(), 0, 0);
}

void SambaWnd::OnMouseUp(wxMouseEvent& event)
{
    WndEventNewWx(this, SMBWX_MOUSE_LEFT_UP, event.GetX(), event.GetY(), 0, 0);
}

void SambaWnd::OnFocus(wxFocusEvent& /*event*/)
{
    WndEventNewWx(this, SMBWX_FOCUS, 0, 0, 0, 0);
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