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

void SourceAudio::AddChannels(const std::map<channelSymbol, channelLabel>& mChannels)
{
    m_mChannel.insert(mChannels.begin(), mChannels.end());
}

void SourceAudio::AddChannel(const channelSymbol& symbol, const channelLabel& label)
{
    m_mChannel.insert(make_pair(symbol, label));
    UpdateVersionTime();
}

void SourceAudio::RemoveChannel(const channelSymbol& symbol)
{
    m_mChannel.erase(symbol);
    UpdateVersionTime();
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
                m_mChannel.insert(make_pair(channelSymbol(jsData["channels"][ai]["symbol"].asString()), channelLabel(jsData["channels"][ai]["label"].asString())));
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

        for(const auto& pairChannel : m_mChannel)
        {
            Json::Value jsChannel;
            jsChannel["label"] = pairChannel.second.Get();
            jsChannel["symbol"] =  pairChannel.first.Get();

            m_json["channels"].append(jsChannel);
        }
        m_nCommitedChannelCount = m_mChannel.size();
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
