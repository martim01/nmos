#pragma once
#include "sender.h"
#include "resourceholder.h"

class Senders : public ResourceHolder
{
    public:
        Senders();
        ~Senders();
        void AddSender(Sender* pSender);
        void RemoveSender(Sender* pSender);

        Json::Value Commit() const;

        std::map<std::string, Sender*>::const_iterator GetSenderBegin()
        {
            return m_mSender.begin();
        }
        std::map<std::string, Sender*>::const_iterator GetSenderEnd()
        {
            return m_mSender.end();
        }
        std::map<std::string, Sender*>::const_iterator FindSender(std::string sSenderId)
        {
            return m_mSender.find(sSenderId);
        }
    private:
        std::map<std::string, Sender*> m_mSender;

};


