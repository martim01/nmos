#include "flowvideocoded.h"

FlowVideoCoded::FlowVideoCoded(std::string sLabel, std::string sDescription, std::string sSourceId, std::string sDeviceId, std::string sMediaType, unsigned int nFrameWidth, unsigned int nFrameHeight, enumColour eColour, enumInterlace eInterlace, enumTransfer eTransfer) :
    FlowVideo(sLabel, sDescription, sSourceId, sDeviceId, sMediaType, nFrameWidth, nFrameHeight, eColour, eInterlace, eTransfer)
{

}


FlowVideoCoded::FlowVideoCoded(const std::string& sMediaType) : FlowVideo(sMediaType)
{

}

bool FlowVideoCoded::UpdateFromJson(const Json::Value& jsData)
{
    return FlowVideo::UpdateFromJson(jsData);
}

bool FlowVideoCoded::Commit(const ApiVersion& version)
{
    if(FlowVideo::Commit(version))
    {
        return true;
    }
    return false;
}



std::string FlowVideoCoded::CreateSDPLines(unsigned short nRtpPort) const
{
    // @todo create VideoCoded SDP information
    return "";
}
