#include "sourceaudio.h"


SourceAudio::SourceAudio(std::string sLabel, std::string sDescription, std::string sDeviceId) :
    Source(sLabel, sDescription, sDeviceId, Source::AUDIO)
{

}

void SourceAudio::AddChannel(std::string sLabel, std::string sSymbol)
{
    m_mChannel.insert(make_pair(sSymbol, sLabel));
}

void SourceAudio::RemoveChannel(std::string sSymbol)
{
    m_mChannel.erase(sSymbol);
}


Json::Value SourceAudio::ToJson() const
{
    Json::Value jsAudio(Source::ToJson());

    jsAudio["channels"] = Json::Value(Json::arrayValue);

    for(std::map<std::string, std::string>::const_iterator itChannel = m_mChannel.begin(); itChannel != m_mChannel.end(); ++itChannel)
    {
        Json::Value jsChannel;
        jsChannel["label"] = itChannel->second;
        jsChannel["symbol"] =  itChannel->first;

        jsAudio["channels"].append(jsChannel);
    }
    return jsAudio;
}
