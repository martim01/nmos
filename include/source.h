#pragma once
#include "resource.h"
#include <set>
#include "nmosdlldefine.h"

class NMOS_EXPOSE Source : public Resource
{
    public:
        enum enumFormat{AUDIO, VIDEO, DATA, MUX};


        Source(std::string sLabel, std::string sDescription, std::string sDeviceId, enumFormat eFormat);
        Source(enumFormat eFormat);
        virtual bool UpdateFromJson(const Json::Value& jsData);
        virtual ~Source(){}


        void AddParentId(std::string sId);
        void RemoveParentId(std::string sId);

        void SetClock(std::string sClock);

        virtual bool Commit(const ApiVersion& version);
        std::string GetParentResourceId() const
        {
            return m_sDeviceId;
        }

    protected:
        void SetFormat(enumFormat eFormat);
    private:
        std::string m_sDeviceId;
        std::set<std::string> m_setParent;
        std::string m_sClock;
        enumFormat m_eFormat;
};


