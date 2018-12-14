#pragma once
#include "resource.h"
#include <set>

class Flow : public Resource
{
    public:
        Flow(std::string sLabel, std::string sDescription, std::string sFormat, std::string sSourceId, std::string sDeviceId);
        virtual ~Flow(){}
        void AddParentId(std::string sId);
        void RemoveParentId(std::string sId);

        virtual bool Commit();
    private:
        std::string m_sFormat;
        std::string m_sSourceId;
        std::string m_sDeviceId;
        std::set<std::string> m_setParent;
};

