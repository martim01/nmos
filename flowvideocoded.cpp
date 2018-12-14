#include "flowvideocoded.h"

FlowVideoCoded::FlowVideoCoded(std::string sLabel, std::string sDescription, std::string sSourceId, std::string sDeviceId, std::string sMediaType, unsigned int nFrameWidth, unsigned int nFrameHeight, enumColour eColour, enumInterlace eInterlace, enumTransfer eTransfer) :
    FlowVideo(sLabel, sDescription, sSourceId, sDeviceId, nFrameWidth, nFrameHeight, eColour, eInterlace, eTransfer),
    m_sMediaType(sMediaType)
{

}

Json::Value FlowVideoCoded::ToJson() const
{
    Json::Value jsFlow(FlowVideo::ToJson());
    jsFlow["media_type"] = m_sMediaType;
    return jsFlow;
}
