#pragma once
#include "flowvideo.h"
#include "nmosdlldefine.h"

class NMOS_EXPOSE FlowVideoCoded : public FlowVideo
{
    public:

        FlowVideoCoded(const std::string& sLabel, const std::string& sDescription, const std::string& sSourceId, const std::string& sDeviceId, const std::string& sMediaType, unsigned int nFrameWidth, unsigned int nFrameHeight, enumColour eColour, enumInterlace eInterlace=PROGRESSIVE, enumTransfer eTransfer=SDR);
        FlowVideoCoded(const std::string& sMediaType);
        virtual bool UpdateFromJson(const Json::Value& jsData);
        virtual bool Commit(const ApiVersion& version);

    private:
};
