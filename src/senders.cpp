#include "senders.h"

Senders::Senders()
{

}

Senders::~Senders()
{
    for(std::map<std::string, Sender*>::const_iterator itSender = m_mSender.begin(); itSender != m_mSender.end(); ++itSender)
    {
        delete itSender->second;
    }
}

void Senders::AddSender(Sender* pSender)
{
    m_mSender.insert(make_pair(pSender->GetId(), pSender));
}

void Senders::RemoveSender(Sender* pSender)
{
    m_mSender.erase(pSender->GetId());
    delete pSender;
}

Json::Value Senders::Commit() const
{
    Json::Value json(Json::arrayValue);
    for(std::map<std::string, Sender*>::const_iterator itSender = m_mSender.begin(); itSender != m_mSender.end(); ++itSender)
    {
        json.append(itSender->second->Commit());
    }
    return json;
}
