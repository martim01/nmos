#pragma once
#include "nmosdlldefine.h"

class ClientApiPrivate;

class NMOS_EXPOSE ClientApi
{
    public:

        enum flagResource {NODES=1, DEVICES=2, SOURCES=4, FLOWS=8, SENDERS=16, RECEIVERS=32, ALL=63};

        static ClientApi& Get();

        void Start(flagResource eInterested=ALL);
        void Stop();

        void ChangeInterest(flagResource eInterest);

    private:
        friend void NodeBrowser();
        friend void ClientThread();
        friend class ClientPoster;

        ClientApi();
        ~ClientApi();

        ClientApiPrivate* m_pApi;
};
