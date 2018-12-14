#pragma once
#include "flowvideo.h"

class FlowVideoCoded : public FlowVideo
{
    public:

        FlowVideoCoded(std::string sLabel, std::string sDescription, std::string sSourceId, std::string sDeviceId, std::string sMediaType, unsigned int nFrameWidth, unsigned int nFrameHeight, enumColour eColour, enumInterlace eInterlace=PROGRESSIVE, enumTransfer eTransfer=SDR);
        virtual Json::Value ToJson() const;

    private:
        std::string m_sMediaType;
};
