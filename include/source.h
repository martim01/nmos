#pragma once
#include "resource.h"
#include <set>
#include "dlldefine.h"

class NMOS_EXPOSE Source : public Resource
{
    public:
        enum enumFormat{AUDIO, VIDEO, DATA, MUX};


        Source(std::string sLabel, std::string sDescription, std::string sDeviceId, enumFormat eFormat);
        virtual ~Source(){}

        void SetFormat(enumFormat eFormat);
        void AddParentId(std::string sId);
        void RemoveParentId(std::string sId);

        void SetClock(std::string sClock);

        virtual bool Commit();
        const std::string& GetDeviceId() const
        {
            return m_sDeviceId;
        }

    private:
        std::string m_sDeviceId;
        std::set<std::string> m_setParent;
        std::string m_sClock;
        enumFormat m_eFormat;
};

