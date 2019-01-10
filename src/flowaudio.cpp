#include "flowaudio.h"

FlowAudio::FlowAudio(std::string sLabel, std::string sDescription, std::string sSourceId, std::string sDeviceId, unsigned int nSampleRate) :
    Flow(sLabel, sDescription, "urn:x-nmos:format:audio", sSourceId, sDeviceId),
    m_nSampleRateNumerator(nSampleRate),
    m_nSampleRateDenominator(1)
{

}

FlowAudio::FlowAudio() : Flow("urn:x-nmos:format:audio")
{

}

bool FlowAudio::UpdateFromJson(const Json::Value& jsData)
{
    Flow::UpdateFromJson(jsData);
    m_bIsOk &= (jsData["sample_rate"]["numerator"].isInt() && (jsData["sample_rate"]["denominator"].isInt() || jsData["sample_rate"]["denominator"].isNull()));
    if(m_bIsOk)
    {
        m_nSampleRateNumerator = jsData["sample_rate"]["numerator"].asInt();

        if(jsData["sample_rate"]["denominator"].isInt())
        {
            m_nSampleRateDenominator = jsData["sample_rate"]["denominator"].asInt();
        }
        else
        {
            m_nSampleRateDenominator = 1;
        }
    }
    return m_bIsOk;
}

bool FlowAudio::Commit()
{
    if(Flow::Commit())
    {
        m_json["sample_rate"]["numerator"] = m_nSampleRateNumerator;
        if(m_nSampleRateDenominator != 1)
        {
            m_json["sample_rate"]["denominator"] = m_nSampleRateDenominator;
        }
        return true;
    }
    return false;
}

void FlowAudio::SetSampleRate(unsigned int nSampleRate)
{
    m_nSampleRateNumerator = nSampleRate;
    m_nSampleRateDenominator = 1;
    UpdateVersionTime();
}

