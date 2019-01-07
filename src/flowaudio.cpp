#include "flowaudio.h"

FlowAudio::FlowAudio(std::string sLabel, std::string sDescription, std::string sSourceId, std::string sDeviceId, unsigned int nSampleRate) :
    Flow(sLabel, sDescription, "urn:x-nmos:format:audio", sSourceId, sDeviceId),
    m_nSampleRate(nSampleRate)
{

}

bool FlowAudio::Commit()
{
    if(Flow::Commit())
    {
        m_json["sample_rate"]["numerator"] = m_nSampleRate;
        return true;
    }
    return false;
}

void FlowAudio::SetSampleRate(unsigned int nSampleRate)
{
    m_nSampleRate = nSampleRate;
}

