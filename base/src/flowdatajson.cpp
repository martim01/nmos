#include "flowdatajson.h"
#include "utils.h"

using namespace pml::nmos;

FlowDataJson::FlowDataJson(const std::string& sLabel, const std::string& sDescription, const std::string& sSourceId, const std::string& sDeviceId) :
    FlowData(sLabel, sDescription, sSourceId, sDeviceId, "application/json")
{

}

FlowDataJson::FlowDataJson() : FlowData("application/json")
{

}

bool FlowDataJson::UpdateFromJson(const Json::Value& jsData)
{
    FlowData::UpdateFromJson(jsData);
    if(jsData["event_type"].isString())
    {
        m_sEventType = jsData["event_type"].asString();
    }
    else if(JsonMemberExistsAndIsNotNull(jsData, "event_type"))
    {
        m_bIsOk = false;
    }

    return m_bIsOk;
}

bool FlowDataJson::Commit(const ApiVersion& version)
{

    if(FlowData::Commit(version))
    {
        m_json["event_type"] = m_sEventType;
        return true;
    }
    return false;

}


std::shared_ptr<FlowDataJson> FlowDataJson::Create(const Json::Value& jsResponse)
{
    auto pResource  = std::make_shared<FlowDataJson>();
    if(pResource->UpdateFromJson(jsResponse))
    {
        return pResource;
    }
    return nullptr;
}

std::string FlowDataJson::CreateSDPMediaLine(unsigned short nPort) const
{
    return std::string();
}

std::string FlowDataJson::CreateSDPAttributeLines(std::shared_ptr<const Source> pSource) const
{
    return std::string();
}
