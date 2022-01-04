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
        /**
         * \brief Create Config for a Samba run
         */
        SambaConfig();

        /**
         * \brief Initialise the start time, in usec, of Samba using wxGetUTCTimeUSec.
         * Could also use 'gettimeofday' - both seem fast enough.
         */
        void InitStartTime();

        /**
         * \brief Return the samba start time
         *
         * \return Samba start time in usec
         */
        wxLongLong GetStartTime();

        /**
         * \brief Copy all the info to the C variables. To be called just before tranferring over.
         */
        void CopyConfigToCGlobals();

    private:
        /// The start time of Samba
        wxLongLong startTime_{0};
};

#endif