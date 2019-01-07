#pragma once
#include "flowaudio.h"
#include "dlldefine.h"

class NMOS_EXPOSE FlowAudioRaw : public FlowAudio
{
    public:
        enum enumFormat { L24, L20, L16, L8};
        enum enumPacket { US_125, US_250, US_333, US_1000, US_4000};;

        FlowAudioRaw(std::string sLabel, std::string sDescription, std::string sSourceId, std::string sDeviceId, unsigned int nSampleRate, enumFormat eFormat);
        virtual bool Commit();

        void SetFormat(enumFormat eFormat);
        void SetPacketTime(enumPacket ePacketTime);

        virtual std::string CreateSDPLines(unsigned short nRtpPort) const;

    private:
        enumFormat m_eFormat;
        enumPacket m_ePacketTime;
};

