#ifndef SAMBA_APP_HPP
#define SAMBA_APP_HPP

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include <samba_wnd.hpp>
#include <vector>
#include <atomic>
#include <samba_evt_handler.hpp>

class SambaApp: public wxApp
{
private:
    std::vector<SambaWnd*> wndList_;

public:
    virtual bool OnInit();
    void StartRenderTimer();
    void UpdateAllWindows();

    SambaWnd *WndCreate(int x, int y, unsigned int width, unsigned int height);
    SambaWnd *SendWndCreateEvent(int x, int y, unsigned int width, unsigned int height);
    void ManageWndCreateEvent(int x, int y, unsigned int width, unsigned int height);

    SambaEvtHandler evtHandler_;
    std::atomic_bool creationDone_{false};
    SambaWnd *lastWindowCreated_{nullptr};
    void RemoveWindow(SambaWnd *w);
};

#endif
