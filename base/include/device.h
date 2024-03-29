#pragma once
#include "resource.h"
#include <set>
#include <map>
#include "nmosdlldefine.h"
#include "namedtype.h"

using endpoint = NamedType<std::string, struct endpointParameter>;
using control = NamedType<std::string, struct ControlParameter>;

namespace pml
{
    namespace nmos
    {
        class NMOS_EXPOSE Device : public Resource
        {
            public:
                enum enumType {GENERIC, PIPELINE};

                Device(const std::string& sLabel, const std::string& sDescription, enumType eType, const std::string& sNodeId);
                Device();
                static std::shared_ptr<Device> Create(const Json::Value& jsResponse);

                virtual bool UpdateFromJson(const Json::Value& jsData);

                virtual ~Device(){}
                void AddControl(const control& type, const endpoint& theEndpoint);
                void RemoveControl(const control& type, const endpoint& theEndpoint);
                void ChangeType(enumType eType);

                const std::multimap<control, endpoint>& GetControls() const { return m_mmControls;}

                endpoint GetPreferredUrl(const control& type) const;
                void RemovePreferredUrl(const control& type);
                void SetPreferredUrl(const control& type, const endpoint& theEndpoint);

                void AddSender(const std::string& sId);
                void AddReceiver(const std::string& sId);

                void RemoveSender(const std::string& sId);
                void RemoveReceiver(const std::string& sId);

                std::string GetParentResourceId() const
                {
                    return m_sNodeId;
                }

                virtual bool Commit(const ApiVersion& version);
            private:

                enumType m_eType;
                std::string m_sNodeId;
                std::multimap<control, endpoint> m_mmControls;
                std::map<control, endpoint> m_mPreferred;
                std::set<std::string> m_setSenders;
                std::set<std::string> m_setReceivers;

                static const std::string STR_TYPE[2];
        };
    };
};

