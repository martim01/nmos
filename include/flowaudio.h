#pragma once
#include "flow.h"
#include "nmosdlldefine.h"

namespace pml
{
    namespace nmos
    {
        class NMOS_EXPOSE FlowAudio : public Flow
        {
            public:
                FlowAudio(const std::string& sLabel, const std::string& sDescription, const std::string& sSourceId, const std::string& sDeviceId, unsigned int nSampleRate);
                virtual bool Commit(const ApiVersion& version);

                FlowAudio();
                virtual bool UpdateFromJson(const Json::Value& jsData);

                void SetSampleRate(unsigned int nSampleRate);
                unsigned int GetSampleRateNumerator() const { return m_nSampleRateNumerator;}
                unsigned int GetSampleRateDenominator() const { return m_nSampleRateDenominator;}

            protected:
                unsigned int m_nSampleRateNumerator;
                unsigned int m_nSampleRateDenominator;
        };
    };
};

