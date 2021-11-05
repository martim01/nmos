#include "flowaudiocoded.h"

std::map<std::string, unsigned short>  FlowAudioCoded::m_mRtpTypes = std::map<std::string, unsigned short>();

FlowAudioCoded::FlowAudioCoded(const std::string& sLabel, const std::string& sDescription, const std::string& sSourceId, const std::string& sDeviceId, unsigned int nSampleRate, const std::string& sMediaType) :
    FlowAudio(sLabel, sDescription, sSourceId, sDeviceId, nSampleRate),
    m_sMediaType(sMediaType)
{
    m_mRtpTypes.insert(std::make_pair("audio/PCMU", 0));
    m_mRtpTypes.insert(std::make_pair("audio/GSM", 3));
    m_mRtpTypes.insert(std::make_pair("audio/G723", 4));
    m_mRtpTypes.insert(std::make_pair("audio/DVI4", 5));
    m_mRtpTypes.insert(std::make_pair("audio/LPC", 7));
    m_mRtpTypes.insert(std::make_pair("audio/PCMA", 8));
    m_mRtpTypes.insert(std::make_pair("audio/G722", 9));
    m_mRtpTypes.insert(std::make_pair("audio/CN", 13));
    m_mRtpTypes.insert(std::make_pair("audio/MPA", 14));
    m_mRtpTypes.insert(std::make_pair("audio/G728", 15));
    m_mRtpTypes.insert(std::make_pair("audio/G729", 18));
}

FlowAudioCoded::FlowAudioCoded() : FlowAudio()
{

}

bool FlowAudioCoded::UpdateFromJson(const Json::Value& jsData)
{
    FlowAudio::UpdateFromJson(jsData);
    if(jsData["media_type"].isString() == false)
    {
        m_bIsOk = false;
        m_ssJsonError << "'media_type' is not a string" ;
    }
    else
    {
        m_sMediaType = jsData["media_type"].asString();
    }
    return m_bIsOk;
}

bool FlowAudioCoded::Commit(const ApiVersion& version)
{
    if(FlowAudio::Commit(version))
    {
        m_json["media_type"] = m_sMediaType;
        return true;
    }
    return false;
}

void FlowAudioCoded::SetMediaType(const std::string& sMediaType)
{
    m_sMediaType = sMediaType;
    UpdateVersionTime();
}


std::string FlowAudioCoded::CreateSDPLines(unsigned short nRtpPort) const
{
    // @todo FlowAudioCoded::CreateSDPLines
    std::stringstream sstr;

    if(m_json["media_type"].isString())
    {
        std::string sMedia = m_json["media_type"].asString();
        std::map<std::string, unsigned short>::const_iterator itType = m_mRtpTypes.find(sMedia);
        if(itType != m_mRtpTypes.end())
        {
            sstr << "m=audio " << nRtpPort << " RTP/AVP " << itType->second << "\r\n"; //this is not 96 its the actual number
        }
        else
        {
            sstr << "m=audio " << nRtpPort << " RTP/AVP 103\r\n";
            sstr << "a=rtpmap:103 " << sMedia << "\r\n";    // @todo check what more is needed here ofr different audio codings
        }

    }
    return sstr.str();
}
