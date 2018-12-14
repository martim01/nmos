#include "flow.h"

Flow::Flow(std::string sLabel, std::string sDescription, std::string sFormat, std::string sSourceId, std::string sDeviceId) :
    Resource(sLabel, sDescription),
    m_sFormat(sFormat),
    m_sSourceId(sSourceId),
    m_sDeviceId(sDeviceId)
{

}

Json::Value Flow::ToJson() const
{
    Json::Value jsFlow(Resource::ToJson());

    jsFlow["format"] = m_sFormat;
    jsFlow["device_id"] = m_sDeviceId;
    jsFlow["source_id"] = m_sSourceId;

    jsFlow["parents"] = Json::Value(Json::objectValue);
    for(std::set<std::string>::iterator itParent = m_setParent.begin(); itParent != m_setParent.end(); ++itParent)
    {
        jsFlow["parents"].append((*itParent));
    }
    return jsFlow;
}

void Flow::AddParentId(std::string sId)
{
    m_setParent.insert(sId);
}

void Flow::RemoveParentId(std::string sId)
{
    m_setParent.erase(sId);
}
