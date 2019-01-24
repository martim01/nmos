#pragma once
#include "nmosdlldefine.h"
#include <string>
#include <map>
#include <memory>

class Self;
class ClientApiPrivate;

class NMOS_EXPOSE ClientApi
{
    public:

        enum flagResource {NODES=1, DEVICES=2, SOURCES=4, FLOWS=8, SENDERS=16, RECEIVERS=32, ALL=63};

        static ClientApi& Get();

        void Start(flagResource eInterested=ALL);
        void Stop();

        void ChangeInterest(flagResource eInterest);

        void Subscribe(const std::string& sSenderId, const std::string& sReceiverId);

        std::map<std::string, std::shared_ptr<Self> >::const_iterator GetNodeBegin();
        std::map<std::string, std::shared_ptr<Self> >::const_iterator GetNodeEnd();


    private:
        friend void NodeBrowser();
        friend void ClientThread();
        friend class ClientPoster;

        ClientApi();
        ~ClientApi();

        ClientApiPrivate* m_pApi;
};
