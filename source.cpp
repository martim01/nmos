#include "source.h"

 Source::Source(std::string sLabel, std::string sDescription, std::string sDeviceId, enumFormat eFormat) : Resource(sLabel, sDescription)
 , m_sDeviceId(sDeviceId)
 , m_eFormat(eFormat)
 {

 }

void Source::AddParentId(std::string sId)
{
    m_setParent.insert(sId);
}

void Source::RemoveParentId(std::string sId)
{
    m_setParent.erase(sId);
}

void Source::SetClock(std::string sClock)
{
    m_sClock = sClock;
}

Json::Value Source::ToJson() const
{
    Json::Value jsonSource(Resource::ToJson());

    jsonSource["device_id"] = m_sDeviceId;
    jsonSource["clock_id"] = m_sClock;

    switch(m_eFormat)
    {
        case AUDIO:
            jsonSource["format"] = "urn:x-nmos:format:audio";
            break;
        case VIDEO:
            jsonSource["format"] = "urn:x-nmos:format:video";
            break;
        case DATA:
            jsonSource["format"] = "urn:x-nmos:format:data";
            break;
        case MUX:
            jsonSource["format"] = "urn:x-nmos:format:mux";
            break;
    }

    jsonSource["parents"] = Json::Value(Json::objectValue);
    for(std::set<std::string>::iterator itParent = m_setParent.begin(); itParent != m_setParent.end(); ++itParent)
    {
        jsonSource["parents"].append((*itParent));
    }
    return jsonSource;
}
