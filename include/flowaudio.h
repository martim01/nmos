#pragma once
#include "flow.h"
#include "dlldefine.h"

class NMOS_EXPOSE FlowAudio : public Flow
{
    public:
        FlowAudio(std::string sLabel, std::string sDescription, std::string sSourceId, std::string sDeviceId, unsigned int nSampleRate);
        virtual bool Commit(const ApiVersion& version);

        FlowAudio();
        virtual bool UpdateFromJson(const Json::Value& jsData);

        void SetSampleRate(unsigned int nSampleRate);

        virtual std::string CreateSDPLines(unsigned short nRtpPort) const=0;

    protected:
        unsigned int m_nSampleRateNumerator;
        unsigned int m_nSampleRateDenominator;
};


