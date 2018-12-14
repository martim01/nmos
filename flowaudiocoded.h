#pragma once
#include "flowaudio.h"

class FlowAudioCoded : public FlowAudio
{
    public:

        FlowAudioCoded(std::string sLabel, std::string sDescription, std::string sSourceId, std::string sDeviceId, unsigned int nSampleRate, std::string sMediaType);
        virtual Json::Value ToJson() const;

    private:
        unsigned int m_nSampleRate;
        std::string m_sMediaType;
};
