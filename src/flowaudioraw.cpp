#include "flowaudioraw.h"
#include "flowaudio.h"

FlowAudioRaw::FlowAudioRaw(std::string sLabel, std::string sDescription, std::string sSourceId, std::string sDeviceId, unsigned int nSampleRate, enumFormat eFormat) :
    FlowAudio(sLabel, sDescription, sSourceId, sDeviceId, nSampleRate),
    m_eFormat(eFormat)
{

}

bool FlowAudioRaw::Commit()
{
    if(FlowAudio::Commit())
    {
        switch(m_eFormat)
        {
            case L24:
                m_json["media_type"] = "audio/L24";
                m_json["bit_depth"] = 24;
                break;
            case L20:
                m_json["media_type"] = "audio/L20";
                m_json["bit_depth"] = 20;
                break;
            case L16:
                m_json["media_type"] = "audio/L16";
                m_json["bit_depth"] = 16;
                break;
            case L8:
                m_json["media_type"] = "audio/L8";
                m_json["bit_depth"] = 8;
                break;
        }
        return true;
    }
    return false;
}

void FlowAudioRaw::SetFormat(enumFormat eFormat)
{
    m_eFormat = eFormat;
    UpdateVersionTime();
}
