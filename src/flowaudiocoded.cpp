#include "flowaudiocoded.h"

FlowAudioCoded::FlowAudioCoded(std::string sLabel, std::string sDescription, std::string sSourceId, std::string sDeviceId, unsigned int nSampleRate, std::string sMediaType) :
    FlowAudio(sLabel, sDescription, sSourceId, sDeviceId, nSampleRate),
    m_sMediaType(sMediaType)
{

}

bool FlowAudioCoded::Commit()
{
    if(FlowAudio::Commit())
    {
        m_json["media_type"] = m_sMediaType;
        return true;
    }
    return false;
}

void FlowAudioCoded::SetMediaType(std::string sMediaType)
{
    m_sMediaType = sMediaType;
    UpdateVersionTime();
}
