#pragma once
#include "resource.h"
#include <set>
#include "dlldefine.h"

class NMOS_EXPOSE Flow : public Resource
{
    public:
        Flow(std::string sLabel, std::string sDescription, std::string sFormat, std::string sSourceId, std::string sDeviceId);
        virtual ~Flow(){}
        void AddParentId(std::string sId);
        void RemoveParentId(std::string sId);

        virtual bool Commit();

        const std::string& GetDeviceId() const
        {
            return m_sDeviceId;
        }
        const std::string& GetSourceId() const
        {
            return m_sSourceId;
        }
    private:
        std::string m_sFormat;
        std::string m_sSourceId;
        std::string m_sDeviceId;
        std::set<std::string> m_setParent;
};

