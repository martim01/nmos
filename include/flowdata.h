#pragma once
#include "flow.h"
#include "nmosdlldefine.h"

class NMOS_EXPOSE FlowData : public Flow
{
    public:

        FlowData(const std::string& sLabel, const std::string& sDescription, const std::string& sSourceId, const std::string& sDeviceId, const std::string& sMediaType);
        FlowData(const std::string& sMediaType);
        virtual bool UpdateFromJson(const Json::Value& jsData);

        virtual bool Commit(const ApiVersion& version);
        void SetMediaType(const std::string& sMediaType);

    private:
        std::string m_sMediaType;
};


