#pragma once
#include "resource.h"
#include <set>
#include <map>
#include "nmosdlldefine.h"
#include "namedtype.h"

using url = NamedType<std::string, struct UrlParameter>;
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
                virtual bool UpdateFromJson(const Json::Value& jsData);

                virtual ~Device(){}
                void AddControl(const control& type, const url& Uri);
                void RemoveControl(const control& type, const url& sUri);
                void ChangeType(enumType eType);

                const std::multimap<control, url>& GetControls() const { return m_mmControls;}

                url GetPreferredUrl(const control& type) const;
                void RemovePreferredUrl(const control& type);
                void SetPreferredUrl(const control& type, const url& Uri);


                std::string GetParentResourceId() const
                {
                    return m_sNodeId;
                }

                virtual bool Commit(const ApiVersion& version);
            private:

                enumType m_eType;
                std::string m_sNodeId;
                std::multimap<control, url> m_mmControls;
                std::map<control, url> m_mPreferred;
                std::set<std::string> m_setSenders;
                std::set<std::string> m_setReceivers;

                static const std::string STR_TYPE[2];
        };
    };
};

