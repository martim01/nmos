#include "source.h"

 Source::Source(std::string sLabel, std::string sDescription, std::string sDeviceId, enumFormat eFormat) : Resource("source", sLabel, sDescription)
 , m_sDeviceId(sDeviceId)
 , m_eFormat(eFormat)
 {

 }

void Source::AddParentId(std::string sId)
{
    m_setParent.insert(sId);
    UpdateVersionTime();
}

void Source::RemoveParentId(std::string sId)
{
    m_setParent.erase(sId);
    UpdateVersionTime();
}

void Source::SetClock(std::string sClock)
{
    m_sClock = sClock;
    UpdateVersionTime();
}

bool Source::Commit()
{
    if(Resource::Commit())
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
