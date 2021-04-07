#ifdef WXWIDGETS

#include <opium_wx_interface.h>
#include <samba_app.hpp>
#include <wx/display.h>
#include <memory>

#include <iostream>

wxFont *theFont{nullptr};
SambaApp *theApp{nullptr};
struct Cadre *cdr_initial{nullptr};
bool samba_running{false};
bool in_paint_event{false};

void InitWxWidgetsApp(int *scr_width, int *scr_height)
{
    theApp = new SambaApp;
    wxApp::SetInstance( theApp );
    int argc{0};
    char **argv{nullptr};
    wxEntryStart(argc, argv);
    wxTheApp->CallOnInit();
    theApp->OnInit();

    wxDisplay display;
    wxRect screen = display.GetClientArea();
    *scr_width = screen.width;
    *scr_height = screen.height;
}

void GetFontInfo(short *width)
{
    theFont = new wxFont(wxFontInfo(14).Family(wxFONTFAMILY_MODERN));
    wxMemoryDC temp_dc;
    temp_dc.SetFont(*theFont);
    wxFontMetrics fm{temp_dc.GetFontMetrics()};
    *width = fm.averageWidth;
}

struct SambaWnd *WndCreateWx(int x, int y, unsigned int width, unsigned int height)
{
    return theApp->WndCreate(x, y, width, height);
}

void WndTitleWx(struct SambaWnd *w, char *title)
{
    w->SetLabel(title);
}

std::unique_ptr<wxDC> MakeDCPtr(SambaWnd *w)
{
    // don't know if we're coming at this from a Paint event or not
    if (w->isPainting())
        return std::make_unique<wxPaintDC>(w);
    else
        return std::make_unique<wxClientDC>(w);
}

void WndDrawStringWx(struct SambaWnd *w, int x, int y, char *text, short fr, short fg, short fb, short br, short bg, short bb )
{
    std::unique_ptr<wxDC> dc = MakeDCPtr(w);

    if (fr > -1) dc->SetTextForeground(wxColour{(unsigned char)fr, (unsigned char)fg, (unsigned char)fb});
    if (br > -1) dc->SetTextBackground(wxColour{(unsigned char)br, (unsigned char)bg, (unsigned char)bb});
    dc->SetBackgroundMode(wxSOLID);
    dc->SetFont(*theFont);
    dc->DrawText(text, x, y);
}

void WndDrawRectWx(struct SambaWnd *w, int x, int y, int width, int height, short r, short g, short b)
{
    std::unique_ptr<wxDC> dc = MakeDCPtr(w);
    if (r < 0)
    {
        dc->SetBrush(dc->GetBackground());
        dc->SetPen(wxPen(dc->GetBackground().GetColour()));
    }
    else
    {
        dc->SetBrush(wxBrush{wxColour{100, (unsigned char)g, (unsigned char)b}});
        dc->SetPen(wxPen(wxPen{wxColour{(unsigned char)r, (unsigned char)g, (unsigned char)b}}));
    }

    dc->DrawRectangle(x, y, width, height);
}

void WndDrawLineWx(struct SambaWnd *w, int x0, int y0, int x1, int y1, short r, short g, short b)
{
    std::unique_ptr<wxDC> dc = MakeDCPtr(w);
    dc->SetBrush(wxBrush{wxColour{(unsigned char)r, (unsigned char)g, (unsigned char)b}});
    dc->SetPen(wxPen(wxPen{wxColour{(unsigned char)r, (unsigned char)g, (unsigned char)b}}));

    dc->DrawLine(x0, y0, x1, y1);
}

void WndMoveWx(struct SambaWnd *w, int x, int y)
{
    w->Move(x, y);
}

void WndClearWx(struct SambaWnd *w)
{
    w->Close();
}

void WndShowTheTopWx(struct SambaWnd *w)
{
    w->Raise();
    w->SetFocus();
}
void OpiumExecWx(struct Cadre *cdr)
{
    cdr_initial = cdr;
    samba_running = true;
    wxTheApp->OnRun();
    wxTheApp->OnExit();
    wxEntryCleanup();
}

void WndEventNewWx(SambaWnd *w, SambaEventWx type, int x, int y, int v, int h)
{
    if (!samba_running)
        return;

    //OpiumManageWx(cdr_initial, w, type, x, y, v, h);
}

struct wxCursor *WndCreateStdCursorWx()
{
    wxCursor *cur_ptr = new wxCursor;
    *cur_ptr = *wxSTANDARD_CURSOR;
    return cur_ptr;
}

#endif // WXWIDGETS