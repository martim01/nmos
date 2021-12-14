#include "flowdata.h"

using namespace pml::nmos;

FlowData::FlowData(const std::string& sLabel, const std::string& sDescription, const std::string& sSourceId, const std::string& sDeviceId, const std::string& sMediaType) :
    Flow(sLabel, sDescription, "urn:x-nmos:format:data", sSourceId, sDeviceId),
    m_sMediaType(sMediaType)
{

}

FlowData::FlowData(const std::string& sMediaType) : Flow("urn:x-nmos:format:data"),
    m_sMediaType(sMediaType)
{

}

bool FlowData::UpdateFromJson(const Json::Value& jsData)
{
    Flow::UpdateFromJson(jsData);
    m_bIsOk &= jsData["media_type"].isString();
    if(m_bIsOk)
    {
        m_sMediaType = jsData["media_type"].asString();
    }
    else
    {
        m_ssJsonError << "'media_type' is not a string" ;
    }
    return m_bIsOk;
}

bool FlowData::Commit(const ApiVersion& version)
{
    if(Flow::Commit(version))
    {
        m_json["media_type"] = m_sMediaType;
        return true;
    }
    return false;
}


void FlowData::SetMediaType(const std::string& sMediaType)
{
    m_sMediaType = sMediaType;
    UpdateVersionTime();
}
