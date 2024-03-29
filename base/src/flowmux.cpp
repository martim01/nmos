#include "flowmux.h"
using namespace pml::nmos;

FlowMux::FlowMux(const std::string& sLabel, const std::string& sDescription, const std::string& sSourceId, const std::string& sDeviceId, const std::string& sMediaType) :
    Flow(sLabel, sDescription, "urn:x-nmos:format:mux", sSourceId, sDeviceId),
    m_sMediaType(sMediaType)
{

}

FlowMux::FlowMux() : Flow("urn:x-nmos:format:mux")
{

}

bool FlowMux::UpdateFromJson(const Json::Value& jsData)
{
    Flow::UpdateFromJson(jsData);
    m_bIsOk &= (jsData["media_type"].isString());
    if(m_bIsOk)
    {
        m_sMediaType = jsData["media_type"].asString();
    }
    else
    {
        m_ssJsonError << "'media-type' is not a string" ;
    }
    return m_bIsOk;
}

bool FlowMux::Commit(const ApiVersion& version)
{
    if(Flow::Commit(version))
    {
        m_json["media_type"] = m_sMediaType;
        return true;
    }

    return false;
}


void FlowMux::SetMediaType(const std::string& sMediaType)
{
    m_sMediaType = sMediaType;
    UpdateVersionTime();
}

std::shared_ptr<FlowMux> FlowMux::Create(const Json::Value& jsResponse)
{
    auto pResource  = std::make_shared<FlowMux>();
    if(pResource->UpdateFromJson(jsResponse))
    {
        return pResource;
    }
    return nullptr;
}

std::string FlowMux::CreateSDPMediaLine(unsigned short nPort) const
{
    return std::string();
}

std::string FlowMux::CreateSDPAttributeLines(std::shared_ptr<const Source> pSource) const
{
    return std::string();
}
