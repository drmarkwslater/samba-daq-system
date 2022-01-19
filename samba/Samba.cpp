#include <vector>
#include <string>
#include <iostream>

#include "plplot_test.hpp"


#include <wx/time.h>
#include <boost/program_options.hpp>
#include <boost/log/trivial.hpp>

#include "SambaConfig.hpp"

//namespace po = boost::program_options;

extern "C" int main_prev(int argc, char *argv[]);

/* ========================================================================== */
int main(int argc, char *argv[]) {
    /**
     * \brief Main entry point for the Samba DAQ Software
     * 
     * \param argc number of arguments
     * \param argv argument list
     * 
     * \todo improve error code return
     * 
     * \return 0 for success, 1 if errors are encountered
     * */

    // Create the config object
    SambaConfig samba_config;

    BOOST_LOG_TRIVIAL(trace) << "A trace severity message";

    // Set up command line option parsing
    boost::program_options::options_description general_options("General Options");
    general_options.add_options()
        ("help,h", "Print help information")
        ("version,v", "Show the version");

    // parse options
    boost::program_options::variables_map vm;
    boost::program_options::store(boost::program_options::parse_command_line(argc, argv, general_options), vm);
    boost::program_options::notify(vm);

    // process options as required
    if (vm.count("help")) {
        //std::cout << options.help() << std::endl;
        return 0;
    }

    // copy the config info to C code
    samba_config.CopyConfigToCGlobals();
    

    PLPlotApp *theApp = new PLPlotApp;
    wxApp::SetInstance( theApp );
    wxEntryStart(argc, argv);
    wxTheApp->CallOnInit();
    theApp->OnInit();

    theApp->OnRun();
    theApp->OnExit();
    wxEntryCleanup();
    return 0;
    // shift to C code
    return main_prev(argc, argv);
}
