#include "clientapi.h"
#include "clientprivate.h"

ClientApi& ClientApi::Get()
{
    static ClientApi api;
    return api;
}

void ClientApi::Start(flagResource eInterested)
{
    m_pApi->Start(eInterested);
}


void ClientApi::Stop()
{
    m_pApi->Stop();
}


void ClientApi::ChangeInterest(flagResource eInterest)
{
    m_pApi->ChangeInterest(eInterest);
}


ClientApi::ClientApi() :
    m_pApi(new ClientApiPrivate())
{

}

ClientApi::~ClientApi()
{
    delete m_pApi;
}

