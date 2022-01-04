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

    std::cout << "Samba Start:  " << samba_config.GetStartTime();

    wxLongLong temp1{wxGetUTCTimeUSec()}, temp2{wxGetUTCTimeUSec()};
    for (int i = 0; i < 100; i++)
    {
        temp2 = wxGetUTCTimeUSec();
        std::cout << temp2 - temp1 << std::endl;
        temp1 = temp2;
    }
    std::cout << "-----------------------------___--" << std::endl;

    struct timeval temp1_g, temp2_g;
    gettimeofday(&temp1_g,0);
    for (int i = 0; i < 100; i++)
    {
        gettimeofday(&temp2_g,0);
        std::cout << temp2_g.tv_usec - temp1_g.tv_usec << std::endl;
        temp1_g = temp2_g;
    }



    return 0;
    return main_prev(argc, argv);
}

/*	char existe,on_recommence,affiche; char rep;
#ifdef GADGET
	char titre[80];
#endif

//-	glutInit(&argc, argv);
	gettimeofday(&SambaHeureDebut,0);
	// malloc_error_break();
    if((argc > 1) && !strcmp(argv[1],"-hw")) SambaConfigHwSw();
//	memset(&CompteRendu,1,sizeof(CompteRendu));
	bzero(&CompteRendu,sizeof(CompteRendu));
	SambaAideDemandee = (argc > 1)
		&& (!strcmp(argv[1],"-help") || !strcmp(argv[1],"--help") || !strcmp(argv[1],"--h") 
			|| !strcmp(argv[1],"-h") || !strcmp(argv[1],"-?"));
	if(!SambaAideDemandee) {
		SambaConfigHwSw();
		SambaInitBasique();
	}
	existe = SambaParms(argc,argv);
	affiche = SambaInitOpium();*/
