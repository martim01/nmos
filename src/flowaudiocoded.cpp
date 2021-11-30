#include "flowaudiocoded.h"


using namespace pml::nmos;

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

    #ifdef NMOS_NODE_API
    m_pSdpCreator  = std::make_unique<FlowAudioCodedSdpCreator>(this);
    #endif // NMOS_NODE_API
}

FlowAudioCoded::FlowAudioCoded() : FlowAudio()
{

}

std::shared_ptr<FlowAudioCoded> FlowAudioCoded::Create(const Json::Value& jsResponse)
{
    auto pResource = std::make_shared<FlowAudioCoded>();
    if(pResource->UpdateFromJson(jsResponse))
    {
        return pResource;
    }
    return nullptr;
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



std::string FlowAudioCoded::CreateSDPMediaLine(unsigned short nPort) const
{
    return std::string();
}

std::string FlowAudioCoded::CreateSDPAttributeLines(std::shared_ptr<const Source> pSource) const
{
    return std::string();
}
