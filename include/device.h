#pragma once
#include "resource.h"
#include <set>
#include "nmosdlldefine.h"

class NMOS_EXPOSE Device : public Resource
{
    public:
        enum enumType {GENERIC, PIPELINE};

        Device(const std::string& sLabel, const std::string& sDescription, enumType eType, const std::string& sNodeId);
        Device();
        virtual bool UpdateFromJson(const Json::Value& jsData);

        virtual ~Device(){}
        void AddControl(const std::string& sType, const std::string& sUri);
        void RemoveControl(const std::string& sType, const std::string& sUri);
        void ChangeType(enumType eType);

        std::set<std::pair<std::string, std::string> >::const_iterator GetControlsBegin() const;
        std::set<std::pair<std::string, std::string> >::const_iterator GetControlsEnd() const;


        std::string GetParentResourceId() const
        {
            return m_sNodeId;
        }

        virtual bool Commit(const ApiVersion& version);
    private:
        enumType m_eType;
        std::string m_sNodeId;
        std::set<std::pair<std::string, std::string> > m_setControls;
        std::set<std::string> m_setSenders;
        std::set<std::string> m_setReceivers;
        static const std::string STR_TYPE[2];
};


