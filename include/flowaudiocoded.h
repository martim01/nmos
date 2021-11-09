#pragma once
#include "flowaudio.h"
#include "nmosdlldefine.h"

class NMOS_EXPOSE FlowAudioCoded : public FlowAudio
{
    public:

        FlowAudioCoded(const std::string& sLabel, const std::string& sDescription, const std::string& sSourceId, const std::string& sDeviceId, unsigned int nSampleRate, const std::string& sMediaType);
        virtual bool Commit(const ApiVersion& version);

        FlowAudioCoded();
        virtual bool UpdateFromJson(const Json::Value& jsData);
        void SetMediaType(const std::string& sMediaType);

    private:
        unsigned int m_nSampleRate;
        std::string m_sMediaType;

        static std::map<std::string, unsigned short> m_mRtpTypes;
};
