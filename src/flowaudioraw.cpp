#include "flowaudioraw.h"
#include "flowaudio.h"
#include <sstream>
#include "nodeapi.h"
#include "sourceaudio.h"

FlowAudioRaw::FlowAudioRaw(std::string sLabel, std::string sDescription, std::string sSourceId, std::string sDeviceId, unsigned int nSampleRate, enumFormat eFormat) :
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
    m_bIsOk &= (jsData["media_type"].isString() && jsData["bit_depth"].isInt());
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
        }
    }
    return m_bIsOk;
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

void FlowAudioRaw::SetPacketTime(enumPacket ePacketTime)
{
    m_ePacketTime = ePacketTime;
}

std::string FlowAudioRaw::CreateSDPLines(unsigned short nRtpPort) const
{
    //we return
    std::stringstream sstr;
    sstr << "m=audio " << nRtpPort << " RTP/AVP 96\r\n";

    sstr << "a=rtpmap:96 L";
    switch(m_eFormat)
    {
    case L24:
        sstr << "24/";
        break;
    case L20:
        sstr << "20/";
        break;
    case L16:
        sstr << "16/";
        break;
    case L8:
        sstr << "8/";
        break;
    }

    sstr << m_nSampleRateNumerator << "/";

    //Get the number of channels from the associated source
    std::map<std::string, std::shared_ptr<Resource> >::const_iterator itResource = NodeApi::Get().GetSources().FindResource(GetSourceId());
    if(itResource != NodeApi::Get().GetSources().GetResourceEnd())
    {
        std::shared_ptr<SourceAudio> pSource = std::dynamic_pointer_cast<SourceAudio>(itResource->second);
        if(pSource)
        {
            sstr << pSource->GetNumberOfChannels();
        }
        else
        {
            sstr << "0";
        }
    }
    else
    {
        sstr << "0";
    }
    sstr << "\r\n";

    // @todo channel order - needed in SMPTE2110. Not needed in AES67 but it shouldnt matter to include it


    //packet time
    switch(m_ePacketTime)
    {
        case US_125:
            sstr << "a=ptime:0.125\r\n";
            break;
        case US_250:
            sstr << "a=ptime:0.250\r\n";
            break;
        case US_333:
            sstr << "a=ptime:0.333\r\n";
            break;
        case US_1000:
            sstr << "a=ptime:1\r\n";
            break;
        case US_4000:
            sstr << "a=ptime:4\r\n";
            break;
    }

    //mediaclk:direct=
    sstr << "a=mediaclk:direct=" << m_nMediaClkOffset << "\r\n";

    return sstr.str();
}
