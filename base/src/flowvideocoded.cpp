#include "flowvideocoded.h"

using namespace pml::nmos;

FlowVideoCoded::FlowVideoCoded(const std::string& sLabel, const std::string& sDescription, const std::string& sSourceId, const std::string& sDeviceId, const std::string& sMediaType, unsigned int nFrameWidth, unsigned int nFrameHeight, enumColour eColour, enumInterlace eInterlace, enumTransfer eTransfer) :
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


std::shared_ptr<FlowVideoCoded> FlowVideoCoded::Create(const Json::Value& jsResponse)
{
    auto pResource  = std::make_shared<FlowVideoCoded>();
    if(pResource->UpdateFromJson(jsResponse))
    {
        return pResource;
    }
    return nullptr;
}


std::string FlowVideoCoded::CreateSDPMediaLine(unsigned short nPort) const
{
    return std::string();
}

std::string FlowVideoCoded::CreateSDPAttributeLines(std::shared_ptr<const Source> pSource) const
{
    return std::string();
}
