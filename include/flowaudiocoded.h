#pragma once
#include "flowaudio.h"
#include "nmosdlldefine.h"

class NMOS_EXPOSE FlowAudioCoded : public FlowAudio
{
    public:

        FlowAudioCoded(std::string sLabel, std::string sDescription, std::string sSourceId, std::string sDeviceId, unsigned int nSampleRate, std::string sMediaType);
        virtual bool Commit(const ApiVersion& version);

        FlowAudioCoded();
        virtual bool UpdateFromJson(const Json::Value& jsData);
        void SetMediaType(std::string sMediaType);

        std::string CreateSDPLines(unsigned short nRtpPort) const;

    private:
        unsigned int m_nSampleRate;
        std::string m_sMediaType;

        static std::map<std::string, unsigned short> m_mRtpTypes;
};
