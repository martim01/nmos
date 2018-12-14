#include "flowaudio.h"

FlowAudio::FlowAudio(std::string sLabel, std::string sDescription, std::string sSourceId, std::string sDeviceId, unsigned int nSampleRate) :
    Flow(sLabel, sDescription, "urn:x-nmos:format:audio", sSourceId, sDeviceId),
    m_nSampleRate(nSampleRate)
{

}

Json::Value FlowAudio::ToJson() const
{
    Json::Value jsFlow(Flow::ToJson());


    jsFlow["sample_rate"]["numerator"] = m_nSampleRate;

    return jsFlow;
}
