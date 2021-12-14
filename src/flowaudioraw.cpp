#include "flowaudioraw.h"
#include "flowaudio.h"
#include <sstream>
#include "sourceaudio.h"


using namespace pml::nmos;
FlowAudioRaw::FlowAudioRaw(const std::string& sLabel, const std::string& sDescription, const std::string& sSourceId, const std::string& sDeviceId, unsigned int nSampleRate, enumFormat eFormat) :
    FlowAudio(sLabel, sDescription, sSourceId, sDeviceId, nSampleRate),
    m_eFormat(eFormat),
    m_ePacketTime(US_1000)
{

}

FlowAudioRaw::FlowAudioRaw() : FlowAudio()
{

}

bool FlowAudioRaw::UpdateFromJson(const Json::Value& jsData)
{
    FlowAudio::UpdateFromJson(jsData);
    if(jsData["media_type"].isString() == false)
    {
        m_bIsOk = false;
        m_ssJsonError << "'media_type' is not a string" ;
    }
    if(jsData["bit_depth"].isInt() == false)
    {
        m_bIsOk = false;
        m_ssJsonError << "'bit_depth' is not an int" ;
    }
    if(m_bIsOk)
    {
        if(jsData["media_type"].asString() == "audio/L24" && jsData["bit_depth"].asInt() == 24)
        {
            m_eFormat = L24;
        }
        else if(jsData["media_type"].asString() == "audio/L20" && jsData["bit_depth"].asInt() == 20)
        {
            m_eFormat = L20;
        }
        else if(jsData["media_type"].asString() == "audio/L16" && jsData["bit_depth"].asInt() == 16)
        {
            m_eFormat = L16;
        }
        else if(jsData["media_type"].asString() == "audio/L8" && jsData["bit_depth"].asInt() == 8)
        {
            m_eFormat = L8;
        }
        else
        {
            m_bIsOk = false;
            m_ssJsonError << "'media_type' and bit_depth' not compatible" ;
        }
    }
    return m_bIsOk;
}

bool FlowAudioRaw::Commit(const ApiVersion& version)
{
    if(FlowAudio::Commit(version))
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

void FlowAudioRaw::SetPacketTime(enumPacket ePacketTime)
{
    m_ePacketTime = ePacketTime;
    UpdateVersionTime();
}

std::shared_ptr<FlowAudioRaw> FlowAudioRaw::Create(const Json::Value& jsResponse)
{
    auto pResource  = std::make_shared<FlowAudioRaw>();
    if(pResource->UpdateFromJson(jsResponse))
    {
        return pResource;
    }
    return nullptr;
}


std::string FlowAudioRaw::CreateSDPMediaLine(unsigned short nPort) const
{
    std::stringstream ss;
    ss << "m=audio " << nPort << " RTP/AVP 96\r\n";
    return ss.str();
}

std::string FlowAudioRaw::CreateSDPAttributeLines(std::shared_ptr<const Source> pSource) const
{
    std::stringstream ssLines;
    ssLines << "a=rtpmap:96 L";
    switch(m_eFormat)
    {
        case enumFormat::L24:
            ssLines << "24/";
            break;
        case enumFormat::L20:
            ssLines << "20/";
            break;
        case enumFormat::L16:
            ssLines << "16/";
            break;
        case enumFormat::L8:
            ssLines << "8/";
            break;
    }

    ssLines << m_nSampleRateNumerator << "/";

    //Get the number of channels from the associated source
    auto pAudioSource = std::dynamic_pointer_cast<const SourceAudio>(pSource);
    if(pAudioSource)
    {
        ssLines << pAudioSource->GetNumberOfChannels() << "\r\n";
    }
    else
    {
        ssLines << "0\r\n";
    }

    // @todo channel order - needed in SMPTE2110. Not needed in AES67 but it shouldnt matter to include it


    //packet time
    switch(m_ePacketTime)
    {
        case enumPacket::US_125:
            ssLines << "a=ptime:0.125\r\n";
            break;
        case enumPacket::US_250:
            ssLines << "a=ptime:0.250\r\n";
            break;
        case enumPacket::US_333:
            ssLines << "a=ptime:0.333\r\n";
            break;
        case enumPacket::US_1000:
            ssLines << "a=ptime:1\r\n";
            break;
        case enumPacket::US_4000:
            ssLines << "a=ptime:4\r\n";
            break;
    }

    //mediaclk:direct=
    ssLines << "a=mediaclk:direct=" << m_nMediaClkOffset << "\r\n";

    return ssLines.str();

}
