#include "flowaudio.h"
#include "utils.h"

using namespace pml::nmos;

FlowAudio::FlowAudio(const std::string& sLabel, const std::string& sDescription, const std::string& sSourceId, const std::string& sDeviceId, unsigned int nSampleRate) :
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
    if(jsData["sample_rate"]["numerator"].isInt() == false)
    {
        m_bIsOk = false;
        m_ssJsonError << "'samplerate' 'numerator' is not an int" ;
    }
    if(jsData["sample_rate"]["denominator"].isInt() == false && JsonMemberExistsAndIsNotNull(jsData["sample_rate"], "denominator"))
    {
        m_bIsOk = false;
        m_ssJsonError << "'samplerate' 'denominator' is not null and not an int" ;
    }

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

bool FlowAudio::Commit(const ApiVersion& version)
{
    if(Flow::Commit(version))
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

