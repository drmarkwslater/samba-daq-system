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

void GetFontInfo(short *width, short *ascent, short *descent, short *leading)
{
    theFont = new wxFont(wxFontInfo(14).Family(wxFONTFAMILY_MODERN));
    wxMemoryDC temp_dc;
    temp_dc.SetFont(*theFont);
    wxFontMetrics fm{temp_dc.GetFontMetrics()};
    *width = fm.averageWidth;
    *ascent = fm.ascent;
    *descent = fm.descent;
    *leading = fm.internalLeading;
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

void WndDrawStringWx(struct SambaWnd *w, int x, int y, char *text, unsigned short fr, unsigned short fg, unsigned short fb, 
                    unsigned short br, unsigned short bg, unsigned short bb, char draw_bg )
{
    std::unique_ptr<wxDC> dc = MakeDCPtr(w);
    //std::cout << text << "  "  << fr << " " << fg << " "  << fg << std::endl;
    dc->SetTextForeground(wxColour{(unsigned char)(255 * fr/65535), (unsigned char)(255 * fg/65535), (unsigned char)(255 * fb/65535)});
    dc->SetTextBackground(wxColour{(unsigned char)(255 * br/65535), (unsigned char)(255 * bg/65535), (unsigned char)(255 * bb/65535)});
    if (draw_bg)
        dc->SetBackgroundMode(wxSOLID);
    else
        dc->SetBackgroundMode(wxTRANSPARENT);
    dc->SetFont(*theFont);
    dc->DrawText(text, x, y);
}

void WndDrawRectWx(struct SambaWnd *w, int x, int y, int width, int height, unsigned short r, unsigned short g, unsigned short b)
{
    std::unique_ptr<wxDC> dc = MakeDCPtr(w);
    dc->SetBrush(wxBrush{wxColour{(unsigned char)(255 * r/65535), (unsigned char)(255 * g/65535), (unsigned char)(255 * b/65535)}});
    dc->SetPen(wxPen(wxPen{wxColour{(unsigned char)(255 * r/65535), (unsigned char)(255 * g/65535), (unsigned char)(255 * b/65535)}}));
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

void WndEventNewWx(SambaWnd *w, SambaEventWx type, int x, int y, int h, int v)
{
    if (!samba_running)
        return;
    OpiumManageWx(cdr_initial, w, type, x, y, h, v);
}

struct wxCursor *WndCreateStdCursorWx()
{
    wxCursor *cur_ptr = new wxCursor;
    *cur_ptr = *wxSTANDARD_CURSOR;
    return cur_ptr;
}

void WndGetWindowSizeWx(struct SambaWnd *w, int *width, int *height)
{
    *width = w->GetClientSize().GetWidth();
    *height = w->GetClientSize().GetHeight();
}

#endif // WXWIDGETS