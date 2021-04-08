#ifdef WXWIDGETS

#include <samba_app.hpp>
#include <samba_wnd.hpp>

bool SambaApp::OnInit()
{
    return true;
}

SambaWnd *SambaApp::WndCreate(int x, int y, unsigned int width, unsigned int height)
{
    // wnd => frame
    SambaWnd *wnd = new SambaWnd( "Hello World", wxPoint(x, y), wxSize(width, height) );
    wnd->SetClientSize(wxSize(width, height));
    wnd->Show( true );
    return wnd;
}


#endif