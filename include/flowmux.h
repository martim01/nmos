#pragma once
#include "flow.h"
#include "nmosdlldefine.h"

class NMOS_EXPOSE FlowMux : public Flow
{
    public:
        FlowMux(const std::string& sLabel, const std::string& sDescription, const std::string& sSourceId, const std::string& sDeviceId, const std::string& sMediaType);
        FlowMux();
        virtual bool UpdateFromJson(const Json::Value& jsData);
        virtual bool Commit(const ApiVersion& version);
        void SetMediaType(const std::string& sMediaType);

        virtual std::string CreateSDPLines(unsigned short nRtpPort) const;

    private:
        std::string m_sMediaType;
};



