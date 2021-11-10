#pragma once
#include "resource.h"
#include <set>
#include "nmosdlldefine.h"

namespace pml
{
    namespace nmos
    {
        class NMOS_EXPOSE Source : public Resource
        {
            public:
                enum enumFormat{AUDIO, VIDEO, DATA, MUX};


                Source(const std::string& sLabel, const std::string& sDescription, const std::string& sDeviceId, enumFormat eFormat);
                Source(enumFormat eFormat);
                virtual bool UpdateFromJson(const Json::Value& jsData);
                virtual ~Source(){}


                void AddParentId(const std::string& sId);
                void RemoveParentId(const std::string& sId);

                void SetClock(const std::string& sClock);
                const std::string& GetClock()
                {   return m_sClock;    }

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
    };
};

