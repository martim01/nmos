#include "device.h"

const std::string Device::STR_TYPE[2] = {"urn:x-nmos:device:generic", "urn:x-nmos:device:pipeline"};

Device::Device(const std::string& sLabel, const std::string& sDescription, enumType eType, const std::string& sNodeId) :
    Resource("device", sLabel, sDescription),
    m_eType(eType),
    m_sNodeId(sNodeId)
{

}


Device::Device() : Resource("device")
{

}

bool Device::UpdateFromJson(const Json::Value& jsData)
{
    Resource::UpdateFromJson(jsData);
    if(jsData["type"].isString() == false)
    {
        m_bIsOk = false;
        m_ssJsonError << "'type' is not a string" ;
    }
    if(jsData["node_id"].isString() == false)
    {
        m_bIsOk = false;
        m_ssJsonError << "'node_id' is not a string" ;
    }
    if(jsData["senders"].isArray() == false)
    {
        m_bIsOk = false;
        m_ssJsonError << "'sender' is not an array" ;
    }
    if(jsData["receivers"].isArray() == false)
    {
        m_bIsOk = false;
        m_ssJsonError << "'receivers' is not an array" ;
    }
    if(jsData["controls"].isArray() == false)
    {
        m_bIsOk = false;
        m_ssJsonError << "'controls' is not an array" ;
    }
    if(m_bIsOk)
    {
        if(jsData["type"].asString() == STR_TYPE[GENERIC])
        {
            m_eType = GENERIC;
        }
        else
        {
            m_eType = PIPELINE;
        }
        m_sNodeId = jsData["node_id"].asString();

        for(Json::ArrayIndex ai = 0; ai < jsData["senders"].size(); ++ai)
        {
            if(jsData["senders"][ai].isString() == false)
            {
                m_bIsOk = false;
                m_ssJsonError << "'senders' entry #" << ai << " not a string" ;
                break;
            }
            else
            {
                m_setSenders.insert(jsData["senders"][ai].asString());
            }
        }
        for(Json::ArrayIndex ai = 0; ai < jsData["receivers"].size(); ++ai)
        {
            if(jsData["receivers"][ai].isString() == false)
            {
                m_bIsOk = false;
                m_ssJsonError << "'receivers' entry #" << ai << " not a string" ;
                break;
            }
            else
            {
                m_setReceivers.insert(jsData["receivers"][ai].asString());
            }
        }
        for(Json::ArrayIndex ai = 0; ai < jsData["controls"].size(); ai++)
        {
            if(jsData["controls"][ai].isObject() == false)
            {
                m_ssJsonError << "'controls' entry #" << ai << " not an object" ;
                m_bIsOk = false;
                break;
            }
            if(jsData["controls"][ai]["href"].isString() == false)
            {
                m_ssJsonError << "'controls' entry #" << ai << " 'href' not a string" ;
                m_bIsOk = false;
                break;
            }
            if(jsData["controls"][ai]["type"].isString() == false)
            {
                m_bIsOk = false;
                m_ssJsonError << "'controls' entry #" << ai << " type not a string" ;
                break;
            }
            else
            {
                m_setControls.insert(std::make_pair(jsData["controls"][ai]["type"].asString(), jsData["controls"][ai]["href"].asString()));
            }
        }
    }
    return m_bIsOk;
}

bool Device::Commit(const ApiVersion& version)
{
    if(Resource::Commit(version))
    {   //something has changed
        m_json["type"] = STR_TYPE[m_eType];
        m_json["node_id"] = m_sNodeId;
        m_json["senders"] = Json::Value(Json::arrayValue);
        for(std::set<std::string>::const_iterator itSender = m_setSenders.begin(); itSender != m_setSenders.end(); ++itSender)
        {
            m_json["senders"].append((*itSender));
        }

        m_json["receivers"] = Json::Value(Json::arrayValue);
        for(std::set<std::string>::const_iterator itReceiver = m_setReceivers.begin(); itReceiver != m_setReceivers.end(); ++itReceiver)
        {
            m_json["receivers"].append((*itReceiver));
        }

        m_json["controls"] = Json::Value(Json::arrayValue);
        for(std::set<std::pair<std::string, std::string> >::iterator itControl = m_setControls.begin(); itControl != m_setControls.end(); ++itControl)
        {
            Json::Value jsControl = Json::Value(Json::objectValue);
            jsControl["type"] = (*itControl).first;
            jsControl["href"] = (*itControl).second;
            m_json["controls"].append(jsControl);
        }
        return true;
    }
    return false;
}

void Device::AddControl(const std::string& sType, const std::string& sUri)
{
    m_setControls.insert(make_pair(sType, sUri));
    UpdateVersionTime();
}
void Device::RemoveControl(const std::string& sType, const std::string& sUri)
{
    m_setControls.erase(make_pair(sType, sUri));
    UpdateVersionTime();
}

void Device::ChangeType(enumType eType)
{
    m_eType = eType;
    UpdateVersionTime();
}

std::set<std::pair<std::string, std::string> >::const_iterator Device::GetControlsBegin() const
{
    return m_setControls.begin();
}

std::set<std::pair<std::string, std::string> >::const_iterator Device::GetControlsEnd() const
{
    return m_setControls.end();
}
