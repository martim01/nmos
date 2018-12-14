#include "device.h"

const std::string Device::TYPE[2] = {"urn:x-nmos:device:generic", "urn:x-nmos:device:pipeline"};

Device::Device(std::string sLabel, std::string sDescription, enumType eType, std::string sNodeId) :
    Resource(sLabel, sDescription),
    m_eType(eType),
    m_sNodeId(sNodeId)
{

}

Json::Value Device::ToJson() const
{
    Json::Value jsDevice(Resource::ToJson());

    jsDevice["type"] = TYPE[m_eType];
    jsDevice["node_id"] = m_sNodeId;

    jsDevice["controls"] = Json::Value(Json::arrayValue);
    for(std::set<std::pair<std::string, std::string> >::iterator itControl = m_setControls.begin(); itControl != m_setControls.end(); ++itControl)
    {
        Json::Value jsControl = Json::Value(Json::objectValue);
        jsControl["type"] = (*itControl).first;
        jsControl["href"] = (*itControl).second;
        jsDevice["controls"].append(jsControl);
    }
    return jsDevice;
}

void Device::AddControl(std::string sType, std::string sUri)
{
    m_setControls.insert(make_pair(sType, sUri));
}
void Device::RemoveControl(std::string sType, std::string sUri)
{
    m_setControls.erase(make_pair(sType, sUri));
}
