#pragma once
#include "receiver.h"
#include "resourceholder.h"

class Receivers : public ResourceHolder
{
    public:
        Receivers();
        ~Receivers();

        void AddReceiver(Receiver* aReceiver);
        void RemoveReceiver(Receiver* aReceiver);

        Json::Value Commit() const;

        std::map<std::string, Receiver*>::const_iterator GetReceiverBegin()
        {
            return m_mReceiver.begin();
        }
        std::map<std::string, Receiver*>::const_iterator GetReceiverEnd()
        {
            return m_mReceiver.end();
        }
        std::map<std::string, Receiver*>::const_iterator FindReceiver(std::string sReceiverId)
        {
            return m_mReceiver.find(sReceiverId);
        }
    private:
        std::map<std::string, Receiver*> m_mReceiver;

};



