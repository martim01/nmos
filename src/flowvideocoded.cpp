#include "flowvideocoded.h"

FlowVideoCoded::FlowVideoCoded(std::string sLabel, std::string sDescription, std::string sSourceId, std::string sDeviceId, std::string sMediaType, unsigned int nFrameWidth, unsigned int nFrameHeight, enumColour eColour, enumInterlace eInterlace, enumTransfer eTransfer) :
    FlowVideo(sLabel, sDescription, sSourceId, sDeviceId, nFrameWidth, nFrameHeight, eColour, eInterlace, eTransfer),
    m_sMediaType(sMediaType)
{

}

bool FlowVideoCoded::Commit()
{
    if(FlowVideo::Commit())
    {
        m_json["media_type"] = m_sMediaType;
        return true;
    }
    return false;
}


void FlowVideoCoded::SetMediaType(std::string sMediaType)
{
    m_sMediaType = sMediaType;
    UpdateVersionTime();
}

std::string FlowVideoCoded::CreateSDPLines(unsigned short nRtpPort) const
{
    // @todo create VideoCoded SDP information
    return "";
}
