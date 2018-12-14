#pragma once
#include "device.h"
#include "resourceholder.h"

class Devices : public ResourceHolder
{
    public:
        Devices();
        ~Devices();
        std::string AddDevice(std::string sLabel, std::string sDescription, Device::enumType eType, std::string sNodeId);
        void RemoveDevice(std::string sDeviceId);

        Json::Value Commit() const;

        std::map<std::string, Device*>::const_iterator GetDeviceBegin()
        {
            return m_mDevice.begin();
        }
        std::map<std::string, Device*>::const_iterator GetDeviceEnd()
        {
            return m_mDevice.end();
        }
        std::map<std::string, Device*>::const_iterator FindDevice(std::string sDeviceId)
        {
            return m_mDevice.find(sDeviceId);
        }

    private:
        std::map<std::string, Device*> m_mDevice;


};

