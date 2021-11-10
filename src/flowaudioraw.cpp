#include "flowaudioraw.h"
#include "flowaudio.h"
#include <sstream>
#include "sourceaudio.h"


#ifdef NMOS_NODE_API
#include "flowsdpcreatornode.h"
#endif // NMOS_NODE_API

using namespace pml::nmos;
FlowAudioRaw::FlowAudioRaw(const std::string& sLabel, const std::string& sDescription, const std::string& sSourceId, const std::string& sDeviceId, unsigned int nSampleRate, enumFormat eFormat) :
    FlowAudio(sLabel, sDescription, sSourceId, sDeviceId, nSampleRate),
    m_eFormat(eFormat),
    m_ePacketTime(US_1000)
{
    #ifdef NMOS_NODE_API
    m_pSdpCreator  = std::make_unique<FlowAudioRawSdpCreator>(this);
    #endif // NMOS_NODE_API
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

