#include "SambaConfig.hpp"

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