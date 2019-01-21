#pragma once
#include "flow.h"
#include "dlldefine.h"

class NMOS_EXPOSE FlowData : public Flow
{
    public:

        FlowData(std::string sLabel, std::string sDescription, std::string sSourceId, std::string sDeviceId, std::string sMediaType);
        FlowData(const std::string sMediaType);
        virtual bool UpdateFromJson(const Json::Value& jsData);

        virtual bool Commit(const ApiVersion& version);
        void SetMediaType(std::string sMediaType);

    private:
        std::string m_sMediaType;
};


