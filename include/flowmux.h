#pragma once
#include "flow.h"
#include "nmosdlldefine.h"

class NMOS_EXPOSE FlowMux : public Flow
{
    public:
        FlowMux(std::string sLabel, std::string sDescription, std::string sSourceId, std::string sDeviceId, std::string sMediaType);
        FlowMux();
        virtual bool UpdateFromJson(const Json::Value& jsData);
        virtual bool Commit(const ApiVersion& version);
        void SetMediaType(std::string sMediaType);

        virtual std::string CreateSDPLines(unsigned short nRtpPort) const;

    private:
        std::string m_sMediaType;
};



