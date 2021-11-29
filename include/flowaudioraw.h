#pragma once
#include "flowaudio.h"
#include "nmosdlldefine.h"

namespace pml
{
    namespace nmos
    {
        class NMOS_EXPOSE FlowAudioRaw : public FlowAudio
        {
            public:
                enum enumFormat { L24, L20, L16, L8};
                enum enumPacket { US_125, US_250, US_333, US_1000, US_4000};;

                FlowAudioRaw(const std::string& sLabel, const std::string& sDescription, const std::string& sSourceId, const std::string& sDeviceId, unsigned int nSampleRate, enumFormat eFormat);
                virtual bool Commit(const ApiVersion& version);
                static std::shared_ptr<FlowAudioRaw> Create(const Json::Value& jsResponse);
                FlowAudioRaw();

                virtual bool UpdateFromJson(const Json::Value& jsData);
                void SetFormat(enumFormat eFormat);
                void SetPacketTime(enumPacket ePacketTime);

                enumFormat GetFormat() const { return m_eFormat;}
                enumPacket GetPacketTime() const { return m_ePacketTime;}
            private:

                enumFormat m_eFormat;
                enumPacket m_ePacketTime;
        };
    };
};
