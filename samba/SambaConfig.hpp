#ifndef SAMBACONFIG_HPP
#define SAMBACONFIG_HPP

#include <wx/time.h>

/* ========================================================================== */
/**
 * \file SambaConfig.hpp
 * \brief Contains the declaration of the SambaConfig class
 */

/**
 * \class SambaConfig
 * \brief Defines the class that contains the config and
 *  pertinent info for a Samba run
 *
 */
class SambaConfig
{
    public:
        SambaConfig();
        void InitStartTime();
        wxLongLong GetStartTime();
        
    private:
        wxLongLong startTime_{0};
};

#endif