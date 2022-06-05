#include <samba_evt_handler.hpp>
#include <samba_app.hpp>
#include <opium_wx_interface.h>

// define global window handling events
wxDEFINE_EVENT(CREATE_WINDOW, wxCommandEvent);

wxBEGIN_EVENT_TABLE(SambaEvtHandler, wxEvtHandler)
    EVT_COMMAND(wxID_ANY, CREATE_WINDOW, SambaEvtHandler::OnCreateWindow)
wxEND_EVENT_TABLE()

void SambaEvtHandler::OnCreateWindow(wxCommandEvent& event)
{
    NewWindowConfig *cfg{static_cast<NewWindowConfig*>(event.GetClientData())};
    theApp_->ManageWndCreateEvent(cfg->x, cfg->y, cfg->width, cfg->height);
}