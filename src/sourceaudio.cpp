#include "sourceaudio.h"


SourceAudio::SourceAudio(std::string sLabel, std::string sDescription, std::string sDeviceId) :
    Source(sLabel, sDescription, sDeviceId, Source::AUDIO),
    m_nCommitedChannelCount(0)
{

}

SourceAudio::SourceAudio() : Source(Source::AUDIO)
{

}

void SourceAudio::AddChannel(std::string sLabel, std::string sSymbol)
{
    m_mChannel.insert(make_pair(sSymbol, sLabel));
    UpdateVersionTime();
}

void SourceAudio::RemoveChannel(std::string sSymbol)
{
    m_mChannel.erase(sSymbol);
    UpdateVersionTime();
}


bool SourceAudio::UpdateFromJson(const Json::Value& jsData)
{
    Source::UpdateFromJson(jsData);
    m_bIsOk &= (jsData["channels"].isArray() && jsData["channels"].size() >0);
    if(m_bIsOk)
    {
        for(Json::ArrayIndex ai = 0; ai < jsData["channels"].size(); ++ai)
        {
            if(jsData["channels"][ai].isObject() == false || jsData["channels"][ai]["label"].isString() == false || jsData["channels"][ai]["symbol"].isString() == false)
            {
                m_bIsOk = false;
                break;
            }
            else
            {
                // @todo should we check the channel symbol is valid??
                m_mChannel.insert(make_pair(jsData["channels"][ai]["symbol"].asString(), jsData["channels"][ai]["label"].asString()));
            }
        }
    }
    return m_bIsOk;
}

bool SourceAudio::Commit()
{
    if(Source::Commit())
    {
        m_json["channels"] = Json::Value(Json::arrayValue);

        for(std::map<std::string, std::string>::const_iterator itChannel = m_mChannel.begin(); itChannel != m_mChannel.end(); ++itChannel)
        {
            Json::Value jsChannel;
            jsChannel["label"] = itChannel->second;
            jsChannel["symbol"] =  itChannel->first;

            m_json["channels"].append(jsChannel);
        }
        m_nCommitedChannelCount = m_mChannel.size();
        return true;
    }
    return false;
}

size_t SourceAudio::GetNumberOfChannels() const
{
    return m_nCommitedChannelCount;
}
