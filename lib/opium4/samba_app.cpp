#ifdef WXWIDGETS

#include <samba_app.hpp>
#include <samba_wnd.hpp>
#include <thread>
#include <chrono>

bool SambaApp::OnInit()
{
    evtHandler_.SetAppPointer(this);
    return true;
}

SambaWnd *SambaApp::WndCreate(int x, int y, unsigned int width, unsigned int height)
{
    // wnd => frame
    SambaWnd *wnd = new SambaWnd( "Hello World", wxPoint(x, y), wxSize(width, height) );
    wnd->Show( true );
    wnd->SetClientSize(wxSize(width, height));
    wnd->SetSambaApp(this);

    wndList_.push_back(wnd);
    return wnd;
}

void SambaApp::RemoveWindow(SambaWnd *w)
{
    wndList_.erase( std::remove(wndList_.begin(), wndList_.end(), w), wndList_.end() );
}

void SambaApp::ManageWndCreateEvent(int x, int y, unsigned int width, unsigned int height)
{
    SambaWnd *wnd = WndCreate(x, y, width, height);
    lastWindowCreated_ = wnd;
    creationDone_ = true;
}

SambaWnd *SambaApp::SendWndCreateEvent(int x, int y, unsigned int width, unsigned int height)
{
    // prepare and send the event
    creationDone_ = false;
    wxCommandEvent event(CREATE_WINDOW);
    NewWindowConfig *cfg = new NewWindowConfig{x, y, width, height};
    event.SetClientData(static_cast<void*>(cfg));
    wxQueueEvent(&(evtHandler_), event.Clone());

    // wait for completion
    while(!creationDone_.load())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    return lastWindowCreated_;
}

void SambaApp::UpdateAllWindows()
{
    for (const auto &ptr : wndList_)
    {
        ptr->RequestUpdate();
    }
}


#endif