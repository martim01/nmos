#pragma once
#include "flowaudio.h"

class FlowAudioRaw : public FlowAudio
{
    public:
        enum enumFormat { L24, L20, L16, L8};

        FlowAudioRaw(std::string sLabel, std::string sDescription, std::string sSourceId, std::string sDeviceId, unsigned int nSampleRate, enumFormat eFormat);
        virtual Json::Value ToJson() const;

    private:
        unsigned int m_nSampleRate;
        enumFormat m_eFormat;
};

