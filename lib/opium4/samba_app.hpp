#ifndef SAMBA_APP_HPP
#define SAMBA_APP_HPP

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include <samba_wnd.hpp>

class SambaApp: public wxApp
{
private:
    std::vector<SambaWnd*> wndList_;

public:
    virtual bool OnInit();
    void StartRenderTimer();
    void UpdateAllWindows();

    SambaWnd *WndCreate(int x, int y, unsigned int width, unsigned int height);
};

#endif
