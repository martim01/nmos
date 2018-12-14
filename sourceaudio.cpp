#include "sourceaudio.h"


SourceAudio::SourceAudio(std::string sLabel, std::string sDescription, std::string sDeviceId) :
    Source(sLabel, sDescription, sDeviceId, Source::AUDIO)
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
        return true;
    }
    return false;
}
