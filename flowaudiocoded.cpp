#include "flowaudiocoded.h"

FlowAudioCoded::FlowAudioCoded(std::string sLabel, std::string sDescription, std::string sSourceId, std::string sDeviceId, unsigned int nSampleRate, std::string sMediaType) :
    FlowAudio(sLabel, sDescription, sSourceId, sDeviceId, nSampleRate),
    m_sMediaType(sMediaType)
{

}

Json::Value FlowAudioCoded::ToJson() const
{
    Json::Value jsFlow(FlowAudio::ToJson());
    jsFlow["media_type"] = m_sMediaType;
    return jsFlow;
}
