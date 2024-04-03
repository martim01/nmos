#include "sourceaudio.h"
#include <algorithm>


using namespace pml::nmos;


SourceAudio::SourceAudio(const std::string& sLabel, const std::string& sDescription, const std::string& sDeviceId) :
    Source(sLabel, sDescription, sDeviceId, Source::AUDIO),
    m_nCommitedChannelCount(0)
{

}

SourceAudio::SourceAudio() : Source(Source::AUDIO)
{

}

void SourceAudio::AddChannels(const std::vector<std::pair<channelSymbol, channelLabel>>& vChannels)
{
    m_vChannel.insert(m_vChannel.end(), vChannels.begin(), vChannels.end());
}

void SourceAudio::AddChannel(const channelSymbol& symbol, const channelLabel& label, bool bUpdateVersion)
{
    m_vChannel.push_back({symbol, label});
    if(bUpdateVersion)
    {
        UpdateVersionTime();
    }
}

void SourceAudio::SetChannel(size_t nChannel, const channelSymbol& symbol, const channelLabel& label, bool UpdateVersion)
{
    if(nChannel < m_vChannel.size())
    {
        m_vChannel[nChannel] = {symbol, label};
    }
    if(UpdateVersion)
    {
        UpdateVersionTime();
    }
}

void SourceAudio::RemoveChannel(size_t nChannel)
{
    if(nChannel < m_vChannel.size())
    {
        m_vChannel.erase(m_vChannel.begin()+nChannel);
    }
    UpdateVersionTime();
}

void SourceAudio::ClearChannels(bool bUpdateVersion)
{
    m_vChannel.clear();
    if(bUpdateVersion)
    {
        UpdateVersionTime();
    }
}


bool SourceAudio::UpdateFromJson(const Json::Value& jsData)
{
    Source::UpdateFromJson(jsData);
    if(jsData["channels"].isArray() == false)
    {
        m_bIsOk = false;
        m_ssJsonError << "'channels' is not an array" ;
    }
    else if(jsData["channels"].size() ==0 )
    {
        m_bIsOk = false;
        m_ssJsonError << "'channels' is an empty array" ;
    }
    if(m_bIsOk)
    {
        m_vChannel.clear();
        m_vChannel.resize(jsData["channels"].size());
        for(Json::ArrayIndex ai = 0; ai < jsData["channels"].size(); ++ai)
        {
            if(jsData["channels"][ai].isObject() == false)
            {
                m_bIsOk = false;
                m_ssJsonError << "'channels' #" << ai << " is not an object" ;
                break;
            }
            else if(jsData["channels"][ai]["label"].isString() == false)
            {
                m_bIsOk = false;
                m_ssJsonError << "'channels' #" << ai << " 'label' is not a string" ;
                break;
            }
            else if(jsData["channels"][ai]["symbol"].isString() == false)
            {
                m_bIsOk = false;
                m_ssJsonError << "'channels' #" << ai << " 'symbol' is not a string" ;
                break;
            }
            else
            {
                // @todo should we check the channel symbol is valid??
                m_vChannel[ai] = std::make_pair(channelSymbol(jsData["channels"][ai]["symbol"].asString()), channelLabel(jsData["channels"][ai]["label"].asString()));
            }
        }
    }
    return m_bIsOk;
}

bool SourceAudio::Commit(const ApiVersion& version)
{
    if(Source::Commit(version))
    {
        m_json["channels"] = Json::Value(Json::arrayValue);

        for(const auto& [symbol, label] : m_vChannel)
        {
            Json::Value jsChannel;
            jsChannel["label"] = label.Get();
            jsChannel["symbol"] =  symbol.Get();

            m_json["channels"].append(jsChannel);
        }
        m_nCommitedChannelCount = m_vChannel.size();
        return true;
    }
    return false;
}

std::shared_ptr<SourceAudio> SourceAudio::Create(const Json::Value& jsResponse)
{
    auto pResource  = std::make_shared<SourceAudio>();
    if(pResource->UpdateFromJson(jsResponse))
    {
        return pResource;
    }
    return nullptr;
}

size_t SourceAudio::GetNumberOfChannels() const
{
    return m_nCommitedChannelCount;
}
