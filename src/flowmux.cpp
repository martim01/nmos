#include "flowmux.h"

FlowMux::FlowMux(std::string sLabel, std::string sDescription, std::string sSourceId, std::string sDeviceId, std::string sMediaType) :
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
        m_ssJsonError << "'media-type' is not a string" << std::endl;
    }
    return m_bIsOk;
}

bool FlowMux::Commit()
{
    if(Flow::Commit())
    {
        m_json["media_type"] = m_sMediaType;
        return true;
    }

    return false;
}


void FlowMux::SetMediaType(std::string sMediaType)
{
    m_sMediaType = sMediaType;
    UpdateVersionTime();
}


std::string FlowMux::CreateSDPLines(unsigned short nRtpPort) const
{
    // @todo create FlowMux SDP information
    return "";
}
