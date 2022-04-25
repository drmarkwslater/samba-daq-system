
#include <samba_evt_handler.hpp>

// define global window handling events
wxDEFINE_EVENT(CREATE_WINDOW, wxCommandEvent);

wxBEGIN_EVENT_TABLE(SambaWnd, wxDialog)
    EVT_COMMAND(wxID_ANY, CREATE_WINDOW, SambaEvtHandler::OnCreateWindow)
wxEND_EVENT_TABLE()

void SambaEvtHandler::OnCreateWindow(wxCommandEvent& event)
{
    std::cout << "CREATE WINDOW" << std::endl;
}