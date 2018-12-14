#include "flowaudioraw.h"
#include "flowaudio.h"

FlowAudioRaw::FlowAudioRaw(std::string sLabel, std::string sDescription, std::string sSourceId, std::string sDeviceId, unsigned int nSampleRate, enumFormat eFormat) :
    FlowAudio(sLabel, sDescription, sSourceId, sDeviceId, nSampleRate),
    m_eFormat(eFormat)
{

}

Json::Value FlowAudioRaw::ToJson() const
{
    Json::Value jsFlow(FlowAudio::ToJson());
    switch(m_eFormat)
    {
        case L24:
            jsFlow["media_type"] = "audio/L24";
            jsFlow["bit_depth"] = 24;
            break;
        case L20:
            jsFlow["media_type"] = "audio/L20";
            jsFlow["bit_depth"] = 20;
            break;
        case L16:
            jsFlow["media_type"] = "audio/L16";
            jsFlow["bit_depth"] = 16;
            break;
        case L8:
            jsFlow["media_type"] = "audio/L8";
            jsFlow["bit_depth"] = 8;
            break;
    }
    return jsFlow;
}
