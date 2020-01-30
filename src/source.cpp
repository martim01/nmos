#include "source.h"
#include "utils.h"


 Source::Source(const std::string& sLabel, const std::string& sDescription, const std::string& sDeviceId, enumFormat eFormat) : Resource("source", sLabel, sDescription)
 , m_sDeviceId(sDeviceId)
 , m_eFormat(eFormat)
 {

 }

 Source::Source(enumFormat eFormat) : Resource("source"),
  m_eFormat(eFormat)
{

}



void Source::AddParentId(const std::string& sId)
{
    m_setParent.insert(sId);
    UpdateVersionTime();
}

void Source::RemoveParentId(const std::string& sId)
{
    m_setParent.erase(sId);
    UpdateVersionTime();
}

void Source::SetClock(const std::string& sClock)
{
    m_sClock = sClock;
    UpdateVersionTime();
}

bool Source::UpdateFromJson(const Json::Value& jsData)
{
    Resource::UpdateFromJson(jsData);
    if(jsData["device_id"].isString() == false)
    {
        m_bIsOk = false;
        m_ssJsonError << "'device_id' is not a string" << std::endl;
    }
    if(jsData["caps"].isObject() == false)
    {
        m_bIsOk = false;
        m_ssJsonError << "'caps' is not an object" << std::endl;
    }
    if(jsData["parents"].isArray() == false)
    {
        m_bIsOk = false;
        m_ssJsonError << "'parents' is not an array" << std::endl;
    }
    if(jsData["clock_name"].isString() == false && JsonMemberExistsAndIsNotNull(jsData, "clock_name"))
    {
        m_bIsOk = false;
        m_ssJsonError << "'clock_name' is neither a string or null" << std::endl;
    }
    if(m_bIsOk)
    {
        m_sDeviceId = jsData["device_id"].asString();
        if(jsData["clock_name"].isString())
        {
            m_sClock = jsData["clock_name"].asString();
        }
        for(Json::ArrayIndex ai = 0; ai < jsData["parents"].size(); ai++)
        {
            if(jsData["parents"][ai].isString() == false)
            {
                m_bIsOk = false;
                m_ssJsonError << "'parents' #" << ai << " is not a string" << std::endl;
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

bool Source::Commit(const ApiVersion& version)
{
    if(Resource::Commit(version))
    {
        m_json["device_id"] = m_sDeviceId;
        if(m_sClock.empty())
        {
            m_json["clock_name"] = Json::nullValue;
        }
        else
        {
            m_json["clock_name"] = m_sClock;
        }

        switch(m_eFormat)
        {
            case AUDIO:
                m_json["format"] = "urn:x-nmos:format:audio";
                break;
            case VIDEO:
                m_json["format"] = "urn:x-nmos:format:video";
                break;
            case DATA:
                m_json["format"] = "urn:x-nmos:format:data";
                break;
            case MUX:
                m_json["format"] = "urn:x-nmos:format:mux";
                break;
        }
        m_json["caps"] = Json::objectValue;
        m_json["parents"] = Json::Value(Json::arrayValue);
        for(std::set<std::string>::iterator itParent = m_setParent.begin(); itParent != m_setParent.end(); ++itParent)
        {
            m_json["parents"].append((*itParent));
        }
        return true;
    }
    return false;
}

void Source::SetFormat(enumFormat eFormat)
{
    m_eFormat = eFormat;
    UpdateVersionTime();
}
