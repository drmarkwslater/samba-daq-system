#ifdef WXWIDGETS

#include <opium_wx_interface.h>
#include <samba_app.hpp>
#include <wx/display.h>
#include <memory>

#include <iostream>
#include <execinfo.h>
#include <mutex>

wxFont *theFont{nullptr};
SambaApp *theApp{nullptr};
struct Cadre *cdr_initial{nullptr};
bool samba_running{false};
bool in_paint_event{false};
int last_evt_ret_code{0};
SambaWnd *mouse_click_window{nullptr};
std::mutex paint_mtx;

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
    if (!wxThread::IsMain())
    {
        // we're not the main thread so send an event and wait for return
        return theApp->SendWndCreateEvent(x, y, width, height);
    }

    SambaWnd *w = theApp->WndCreate(x, y, width, height);

    if (mouse_click_window)
    {
        mouse_click_window->IgnoreNextMouseRelease();
    }

    return w;
}

void WndTitleWx(struct SambaWnd *w, char *title)
{
    if (!wxThread::IsMain())
    {
        wxCommandEvent event(SET_WND_TITLE);
        event.SetString(title);
        wxQueueEvent(w, event.Clone());
        return;
    }

    w->SetLabel(title);
}

void WndMoveWx(struct SambaWnd *w, int x, int y)
{
    if (!wxThread::IsMain())
    {
        return;
    }

    w->Move(x, y);
}

void WndResizeWx(struct SambaWnd *w, int h, int v)
{
    if (!wxThread::IsMain())
    {
        return;
    }

    w->SetSize(w->GetPosition().x, w->GetPosition().y, h, v);
}

void WndClearAllWx()
{
    if (samba_running) 
    {
        theApp->ExitMainLoop();
    }
}

void WndClearWx(struct SambaWnd *w)
{
    if (!wxThread::IsMain())
    {
        wxCommandEvent event(REQUEST_CLOSE);
        wxQueueEvent(w, event.Clone());
        return;
    }

    if (samba_running)
    {
        w->MenuClose();
    }
}

void WndShowTheTopWx(struct SambaWnd *w)
{
    if (!samba_running)
    {
        return;
    }

    if (!wxThread::IsMain())
    {
        return;
    }

    w->Raise();
    w->SetFocus();
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
    if (!wxThread::IsMain())
    {
        return;
    }

    std::unique_ptr<wxDC> dc = MakeDCPtr(w);
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
    if (!wxThread::IsMain())
    {
        return;
    }

    std::unique_ptr<wxDC> dc = MakeDCPtr(w);
    dc->SetBrush(wxBrush{wxColour{(unsigned char)(255 * r/65535), (unsigned char)(255 * g/65535), (unsigned char)(255 * b/65535)}});
    dc->SetPen(wxPen(wxPen{wxColour{(unsigned char)(255 * r/65535), (unsigned char)(255 * g/65535), (unsigned char)(255 * b/65535)}}));
    dc->DrawRectangle(x, y, width, height);
}

void WndDrawLineWx(struct SambaWnd *w, int x0, int y0, int x1, int y1, unsigned short r, unsigned short g, unsigned short b)
{
    if (!wxThread::IsMain())
    {
        return;
    }

    std::unique_ptr<wxDC> dc = MakeDCPtr(w);
    dc->SetBrush(wxBrush{wxColour{(unsigned char)(255 * r/65535), (unsigned char)(255 * g/65535), (unsigned char)(255 * b/65535)}});
    dc->SetPen(wxPen(wxPen{wxColour{(unsigned char)(255 * r/65535), (unsigned char)(255 * g/65535), (unsigned char)(255 * b/65535)}}));
    dc->DrawLine(x0, y0, x1, y1);
}

void WndDrawPolyWx(struct SambaWnd *w, int *x, int *y, int num, unsigned short r, unsigned short g, unsigned short b)
{
    if (!wxThread::IsMain())
    {
        return;
    }

    std::unique_ptr<wxDC> dc = MakeDCPtr(w);
    dc->SetBrush(wxBrush{wxColour{(unsigned char)(255 * r/65535), (unsigned char)(255 * g/65535), (unsigned char)(255 * b/65535)}});
    dc->SetPen(wxPen(wxPen{wxColour{(unsigned char)(255 * r/65535), (unsigned char)(255 * g/65535), (unsigned char)(255 * b/65535)}}));

    wxPoint points[1000];
    for (int i = 0; i < num; i++)
    {
        points[i].x = x[i];
        points[i].y = y[i];
    }
    dc->DrawLines(num, points);
}

void WndDrawArcWx(struct SambaWnd *w, int x, int y, int width, int height, int start, int stop, unsigned short r, unsigned short g, unsigned short b)
{
    if (!wxThread::IsMain())
    {
        return;
    }

    std::unique_ptr<wxDC> dc = MakeDCPtr(w);
    dc->SetBrush(*wxTRANSPARENT_BRUSH);
    dc->SetPen(wxPen(wxPen{wxColour{(unsigned char)(255 * r/65535), (unsigned char)(255 * g/65535), (unsigned char)(255 * b/65535)}}));

    dc->DrawEllipticArc(x, y, width, height, start, stop);
}

int OpiumExecWx(struct Cadre *cdr, SambaWnd *w)
{
    if (samba_running)
    {
        if (wxThread::IsMain())
        {
            // called for a modal dialog
            w->Show( false );
            w->ShowModal();
        }
        else
        {
            // we're not the main thread so send an event and wait for return
            w->ExecModal();
        }

        return last_evt_ret_code;
    } else {
        // called for main running
        cdr_initial = cdr;
        samba_running = true;
        wxTheApp->OnRun();
        wxTheApp->OnExit();
        wxEntryCleanup();
        samba_running = false;
    }

    return 0;
}

void OpiumCheckThreadRefreshCall()
{
    if (!wxThread::IsMain())
    {
        void* callstack[128];\
	int frame_num, frames_nb = backtrace(callstack, 128);\
	char** frame_texte = backtrace_symbols(callstack, frames_nb);\
	printf("| Suite des %d appel%s jusqu'a %s\n",frames_nb,(frames_nb > 1)?"s":"",__func__);\
	for (frame_num = 0; frame_num < frames_nb; ++frame_num) {\
		char *r,*s,c;\
		s = r = frame_texte[frame_num];\
		while((*s != '\0') && (*s != ' ')) s++; c = *s;\
		*s = '\0'; printf("| %s) ",r); if(c == '\0') continue;\
		r = s + 1; while((*r != '\0') && (*r == ' ')) r++; if(*r == '\0') continue;\
		s = r; while((*s != '\0') && (*s != ' ')) s++; c = *s; if(c == '\0') continue;\
		r = s + 1; while((*r != '\0') && (*r == ' ')) r++; if(*r == '\0') continue;\
		s = r; while((*s != '\0') && (*s != ' ')) s++; c = *s;\
		*s = '\0'; printf("@%s: ",r); if(c == '\0') continue;\
		r = s + 1; printf("%s\n",r);\
	}\
	free(frame_texte);\
        std::cout << "WARNING: OpiumRefreshIf called from secondary thread" << std::endl;
    }
}

void OpiumRefreshAllWindows()
{
    theApp->UpdateAllWindows();
}

void WndEventNewWx(SambaWnd *w, SambaEventWx type, int x, int y, int h, int v)
{
    if (type == SMBWX_MOUSE_LEFT_DOWN)
    {
        mouse_click_window = w;
    }

    if (!samba_running)
        return;

    last_evt_ret_code = OpiumManageWx(cdr_initial, w, type, x, y, h, v);
    if (type == SMBWX_MOUSE_LEFT_UP)
        theApp->UpdateAllWindows();
}

void WndAssignCursorWx(SambaWnd *w, wxCursor *c)
{
    ::wxSetCursor(*c);
}

struct wxCursor *WndCreateStdCursorWx(int cur_type)
{
    wxCursor *cur_ptr = new wxCursor;

    switch (cur_type)
    {
        case 2:
            cur_ptr = new wxCursor(wxCURSOR_ARROW);
            break;

        case 3:
            cur_ptr = new wxCursor(wxCURSOR_CROSS);
            break;

        case 4:
            cur_ptr = new wxCursor(wxCURSOR_HAND);
            break;

        default:
            *cur_ptr = *wxSTANDARD_CURSOR;
    }

    return cur_ptr;
}

void WndGetWindowSizeWx(struct SambaWnd *w, int *width, int *height)
{
    *width = w->GetClientSize().GetWidth();
    *height = w->GetClientSize().GetHeight();
}

void LockPaintEvents()
{
    paint_mtx.lock();
}

void UnlockPaintEvents()
{
    paint_mtx.unlock();
}

#endif // WXWIDGETS