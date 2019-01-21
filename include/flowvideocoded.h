#pragma once
#include "flowvideo.h"
#include "dlldefine.h"

class NMOS_EXPOSE FlowVideoCoded : public FlowVideo
{
    public:

        FlowVideoCoded(std::string sLabel, std::string sDescription, std::string sSourceId, std::string sDeviceId, std::string sMediaType, unsigned int nFrameWidth, unsigned int nFrameHeight, enumColour eColour, enumInterlace eInterlace=PROGRESSIVE, enumTransfer eTransfer=SDR);
        FlowVideoCoded(const std::string& sMediaType);
        virtual bool UpdateFromJson(const Json::Value& jsData);
        virtual bool Commit(const ApiVersion& version);

        std::string CreateSDPLines(unsigned short nRtpPort) const;

    private:
};
