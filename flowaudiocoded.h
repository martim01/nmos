#pragma once
#include "flowaudio.h"
#include "dlldefine.h"

class NMOS_EXPOSE FlowAudioCoded : public FlowAudio
{
    public:

        FlowAudioCoded(std::string sLabel, std::string sDescription, std::string sSourceId, std::string sDeviceId, unsigned int nSampleRate, std::string sMediaType);
        virtual bool Commit();

        void SetMediaType(std::string sMediaType);

    private:
        unsigned int m_nSampleRate;
        std::string m_sMediaType;
};
