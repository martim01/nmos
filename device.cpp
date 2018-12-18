#include "device.h"

const std::string Device::TYPE[2] = {"urn:x-nmos:device:generic", "urn:x-nmos:device:pipeline"};

Device::Device(std::string sLabel, std::string sDescription, enumType eType, std::string sNodeId) :
    Resource(sLabel, sDescription),
    m_eType(eType),
    m_sNodeId(sNodeId)
{

}

bool Device::Commit()
{
    if(Resource::Commit())
    {   //something has changed
        m_json["type"] = TYPE[m_eType];
        m_json["node_id"] = m_sNodeId;
        m_json["senders"] = Json::Value(Json::arrayValue);
        m_json["receivers"] = Json::Value(Json::arrayValue);
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

void Device::AddControl(std::string sType, std::string sUri)
{
    m_setControls.insert(make_pair(sType, sUri));
    UpdateVersionTime();
}
void Device::RemoveControl(std::string sType, std::string sUri)
{
    m_setControls.erase(make_pair(sType, sUri));
    UpdateVersionTime();
}

void Device::ChangeType(enumType eType)
{
    m_eType = eType;
    UpdateVersionTime();
}
