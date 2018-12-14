#include "receivers.h"

Receivers::Receivers()
{

}

Receivers::~Receivers()
{
    for(std::map<std::string, Receiver*>::const_iterator itReceiver = m_mReceiver.begin(); itReceiver != m_mReceiver.end(); ++itReceiver)
    {
        delete itReceiver->second;
    }
}

void Receivers::AddReceiver(Receiver* pReceiver)
{
    m_mReceiver.insert(make_pair(pReceiver->GetId(), pReceiver));
}

void Receivers::RemoveReceiver(Receiver* pReceiver)
{
    m_mReceiver.erase(pReceiver->GetId());
    delete pReceiver;
}

Json::Value Receivers::ToJson() const
{
    Json::Value json(Json::arrayValue);
    for(std::map<std::string, Receiver*>::const_iterator itReceiver = m_mReceiver.begin(); itReceiver != m_mReceiver.end(); ++itReceiver)
    {
        json.append(itReceiver->second->ToJson());
    }
    return json;
}
