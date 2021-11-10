#include "clientapi.h"
#include "clientprivate.h"

using namespace pml::nmos;
ClientApi& ClientApi::Get()
{
    static ClientApi api;
    return api;
}

void ClientApi::Start()
{
    m_pApi->Start();
}


void ClientApi::Stop()
{
    m_pApi->Stop();
}



ClientApi::ClientApi() :
    m_pApi(new ClientApiImpl())
{

}

ClientApi::~ClientApi()
{

}

const std::map<std::string, std::shared_ptr<Self> >& ClientApi::GetNodes()
{
    return m_pApi->GetNodes();
}

std::map<std::string, std::shared_ptr<Self> >::const_iterator ClientApi::FindNode(const std::string& sUid)
{
    return m_pApi->FindNode(sUid);
}

const std::map<std::string, std::shared_ptr<Device> >& ClientApi::GetDevices()
{
    return m_pApi->GetDevices();
}

std::map<std::string, std::shared_ptr<Device> >::const_iterator ClientApi::FindDevice(const std::string& sUid)
{
    return m_pApi->FindDevice(sUid);
}

const std::map<std::string, std::shared_ptr<Source> >& ClientApi::GetSources()
{
    return m_pApi->GetSources();
}


std::map<std::string, std::shared_ptr<Source> >::const_iterator ClientApi::FindSource(const std::string& sUid)
{
    return m_pApi->FindSource(sUid);
}

const std::map<std::string, std::shared_ptr<Flow> >& ClientApi::GetFlows()
{
    return m_pApi->GetFlows();
}


std::map<std::string, std::shared_ptr<Flow> >::const_iterator ClientApi::FindFlow(const std::string& sUid)
{
    return m_pApi->FindFlow(sUid);
}

const std::map<std::string, std::shared_ptr<SenderBase> >& ClientApi::GetSenders()
{
    return m_pApi->GetSenders();
}


std::map<std::string, std::shared_ptr<SenderBase> >::const_iterator ClientApi::FindSender(const std::string& sUid)
{
    return m_pApi->FindSender(sUid);
}

const std::map<std::string, std::shared_ptr<ReceiverBase> >& ClientApi::GetReceivers()
{
    return m_pApi->GetReceivers();
}

std::map<std::string, std::shared_ptr<ReceiverBase> >::const_iterator ClientApi::FindReceiver(const std::string& sUid)
{
    return m_pApi->FindReceiver(sUid);
}



bool ClientApi::Subscribe(const std::string& sSenderId, const std::string& sReceiverId)
{
    return m_pApi->Subscribe(sSenderId, sReceiverId);
}

bool ClientApi::Unsubscribe(const std::string& sReceiverId)
{
    return m_pApi->Unsubscribe(sReceiverId);
}


bool ClientApi::RequestSenderStaged(const std::string& sSenderId)
{
    return m_pApi->RequestSenderStaged(sSenderId);
}

bool ClientApi::RequestSenderActive(const std::string& sSenderId)
{
    return m_pApi->RequestSenderActive(sSenderId);
}

bool ClientApi::RequestSenderTransportFile(const std::string& sSenderId)
{
    return m_pApi->RequestSenderTransportFile(sSenderId);
}

bool ClientApi::RequestReceiverStaged(const std::string& sReceiverId)
{
    return m_pApi->RequestReceiverStaged(sReceiverId);
}

bool ClientApi::RequestReceiverActive(const std::string& sReceiverId)
{
    return m_pApi->RequestReceiverActive(sReceiverId);
}

bool ClientApi::PatchSenderStaged(const std::string& sSenderId, const connectionSender& aConnection)
{
    return m_pApi->PatchSenderStaged(sSenderId, aConnection);
}

bool ClientApi::PatchReceiverStaged(const std::string& sReceiverId, const connectionReceiver& aConnection)
{
    return m_pApi->PatchReceiverStaged(sReceiverId, aConnection);
}

void ClientApi::SetPoster(std::shared_ptr<ClientApiPoster> pPoster)
{
    m_pApi->SetPoster(pPoster);
}


bool ClientApi::Connect(const std::string& sSenderId, const std::string& sReceiverId)
{
    return m_pApi->Connect(sSenderId, sReceiverId);
}

bool ClientApi::Disconnect(const std::string& sReceiverId)
{
    return m_pApi->Disconnect(sReceiverId);
}


bool ClientApi::AddQuerySubscription(flagResource eResource, const std::string& sQuery, unsigned long nUpdateRate)
{
    return m_pApi->AddQuerySubscription(eResource, sQuery, nUpdateRate);
}

bool ClientApi::RemoveQuerySubscription(const std::string& sSubscriptionId)
{
    return m_pApi->RemoveQuerySubscription(sSubscriptionId);
}
