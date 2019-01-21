#include "flow.h"

Flow::Flow(std::string sLabel, std::string sDescription, std::string sFormat, std::string sSourceId, std::string sDeviceId) :
    Resource("flow", sLabel, sDescription),
    m_nMediaClkOffset(0),
    m_sFormat(sFormat),
    m_sSourceId(sSourceId),
    m_sDeviceId(sDeviceId)

{

}

Flow::Flow(std::string sFormat) : Resource("flow"),
    m_sFormat(sFormat)
{

}

bool Flow::UpdateFromJson(const Json::Value& jsData)
{
    Resource::UpdateFromJson(jsData);
    if(jsData["device_id"].isString() == false)
    {
        m_bIsOk = false;
        m_ssJsonError << "'device_id' is not a string" << std::endl;
    }
    if(jsData["source_id"].isString() == false)
    {
        m_bIsOk = false;
        m_ssJsonError << "'device_id' is not a string" << std::endl;
    }
    if(jsData["parents"].isArray() == false)
    {
        m_bIsOk = false;
        m_ssJsonError << "'parents' is not an array" << std::endl;
    }
    if(m_bIsOk)
    {
        m_sDeviceId = jsData["device_id"].asString();
        m_sSourceId = jsData["source_id"].asString();

        for(Json::ArrayIndex ai = 0; ai < jsData["parents"].size(); ai++)
        {
            if(jsData["parents"][ai].isString() == false)
            {
                m_bIsOk = false;
                m_ssJsonError << "'parents' #" << ai << " is not a string" << std::endl;
            }
            else
            {
                m_setParent.insert(jsData["parents"][ai].asString());
            }
        }
    }
    return m_bIsOk;
}

bool Flow::Commit(const ApiVersion& version)
{
    if(Resource::Commit(version))
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
