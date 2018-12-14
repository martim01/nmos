#include "devices.h"

Devices::Devices()
{

}

Devices::~Devices()
{
    for(std::map<std::string, Device*>::const_iterator itDevice = m_mDevice.begin(); itDevice != m_mDevice.end(); ++itDevice)
    {
        delete itDevice->second;
    }
    m_mDevice.clear();
}

std::string Devices::AddDevice(std::string sLabel, std::string sDescription, Device::enumType eType, std::string sNodeId)
{
    Device* pDevice = new Device(sLabel, sDescription, eType, sNodeId);
    m_mDevice.insert(make_pair(pDevice->GetId(), pDevice));
    return pDevice->GetId();
}

void Devices::RemoveDevice(std::string sDeviceId)
{
    std::map<std::string, Device*>::iterator itDevice = m_mDevice.find(sDeviceId);
    if(itDevice != m_mDevice.end())
    {
        delete itDevice->second;
        m_mDevice.erase(itDevice);
    }
}

Json::Value Devices::ToJson() const
{
    Json::Value json(Json::arrayValue);
    for(std::map<std::string, Device*>::const_iterator itDevice = m_mDevice.begin(); itDevice != m_mDevice.end(); ++itDevice)
    {
        json.append(itDevice->second->ToJson());
    }
    return json;
}
