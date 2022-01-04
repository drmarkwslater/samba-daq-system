#include <vector>
#include <string>
#include <iostream>

#include <wx/time.h>
#include <sys/time.h>

#include "SambaConfig.hpp"

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

    // convert c-style args to vector of strings
    const std::vector<std::string> cmdLineArgs { argv, argv+argc };

    // Create the config object
    SambaConfig samba_config;

    std::cout << "Samba Start:  " << samba_config.GetStartTime() << std::endl;

    // copy the config info to C code
    samba_config.CopyConfigToCGlobals();
    
    // shift to C code
    return main_prev(argc, argv);
}
