#pragma once
#include "flowaudio.h"
#include "dlldefine.h"

class NMOS_EXPOSE FlowAudioRaw : public FlowAudio
{
    public:
        enum enumFormat { L24, L20, L16, L8};

        FlowAudioRaw(std::string sLabel, std::string sDescription, std::string sSourceId, std::string sDeviceId, unsigned int nSampleRate, enumFormat eFormat);
        virtual bool Commit();

        void SetFormat(enumFormat eFormat);

    private:
        unsigned int m_nSampleRate;
        enumFormat m_eFormat;
};

