#include "SambaConfig.hpp"
#include <sys/time.h>

extern "C" void set_globals_from_cpp(struct timeval SambaHeureDebut_);

SambaConfig::SambaConfig()
{
    InitStartTime();
}

void SambaConfig::InitStartTime()
{
    startTime_ = wxGetUTCTimeUSec();
}

wxLongLong SambaConfig::GetStartTime()
{
    return startTime_;
}

void SambaConfig::CopyConfigToCGlobals()
{
    timeval SambaHeureDebut_;
    SambaHeureDebut_.tv_sec = startTime_.ToDouble() / 1000000;
    SambaHeureDebut_.tv_usec = startTime_.ToDouble() - SambaHeureDebut_.tv_sec * 1000000;

    set_globals_from_cpp(SambaHeureDebut_);
}