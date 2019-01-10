#include "flow.h"

Flow::Flow(std::string sLabel, std::string sDescription, std::string sFormat, std::string sSourceId, std::string sDeviceId) :
    Resource("flow", sLabel, sDescription),
    m_sFormat(sFormat),
    m_sSourceId(sSourceId),
    m_sDeviceId(sDeviceId),
    m_nMediaClkOffset(0)
{

}

Flow::Flow(std::string sFormat) : Resource("flow"),
    m_sFormat(sFormat)
{

}

bool Flow::UpdateFromJson(const Json::Value& jsData)
{
    Resource::UpdateFromJson(jsData);
    m_bIsOk &= (jsData["device_id"].isString && jsData["source_id"].isString() && jsData["parents"].isArray());
    if(m_bIsOk)
    {
        m_sDeviceId = jsData["device_id"].asString();
        m_sSourceId = jsData["source_id"].asString();

        for(Json::ArrayIndex ai = 0; ai < jsData["parents"].size(); ai++)
        {
            if(jsData["parents"][ai].isString() == false)
            {
                m_bIsOk = false;
                break;
            }
            else
            {
                m_setParent.insert(jsData["parents"][ai].asString());
            }
        }
    }
    return m_bIsOk;
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


void Flow::SetMediaClkOffset(unsigned long nOffset)
{
    m_nMediaClkOffset = nOffset;
}
