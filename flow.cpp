#include "flow.h"

Flow::Flow(std::string sLabel, std::string sDescription, std::string sFormat, std::string sSourceId, std::string sDeviceId) :
    Resource(sLabel, sDescription),
    m_sFormat(sFormat),
    m_sSourceId(sSourceId),
    m_sDeviceId(sDeviceId)
{

}

bool Flow::Commit()
{
    if(Resource::Commit())
    {
        m_json["format"] = m_sFormat;
        m_json["device_id"] = m_sDeviceId;
        m_json["source_id"] = m_sSourceId;

        m_json["parents"] = Json::Value(Json::arrayValue);
        for(std::set<std::string>::iterator itParent = m_setParent.begin(); itParent != m_setParent.end(); ++itParent)
        {
            m_json["parents"].append((*itParent));
        }
        return true;
    }
    return false;
}

void Flow::AddParentId(std::string sId)
{
    m_setParent.insert(sId);
    UpdateVersionTime();
}

void Flow::RemoveParentId(std::string sId)
{
    m_setParent.erase(sId);
    UpdateVersionTime();
}
