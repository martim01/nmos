#pragma once
#include "resource.h"
#include <set>
#include "dlldefine.h"

class NMOS_EXPOSE Device : public Resource
{
    public:
        enum enumType {GENERIC, PIPELINE};

        Device(std::string sLabel, std::string sDescription, enumType eType, std::string sNodeId);
        virtual ~Device(){}
        void AddControl(std::string sType, std::string sUri);
        void RemoveControl(std::string sType, std::string sUri);
        void ChangeType(enumType eType);

        virtual bool Commit();
    private:
        enumType m_eType;
        std::string m_sNodeId;
        std::set<std::pair<std::string, std::string> > m_setControls;

        static const std::string TYPE[2];
};


