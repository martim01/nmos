#include "clientapi.h"
#include "clientprivate.h"

ClientApi& ClientApi::Get()
{
    static ClientApi api;
    return api;
}

void ClientApi::Start()
{
    m_pApi->Start(ALL);
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

std::map<std::string, std::shared_ptr<Self> >::const_iterator ClientApi::GetNodeBegin()
{
    return m_pApi->GetNodeBegin();
}

std::map<std::string, std::shared_ptr<Self> >::const_iterator ClientApi::GetNodeEnd()
{
    return m_pApi->GetNodeEnd();
}

std::map<std::string, std::shared_ptr<Self> >::const_iterator ClientApi::FindNode(const std::string& sUid)
{
    return m_pApi->FindNode(sUid);
}

std::map<std::string, std::shared_ptr<Device> >::const_iterator ClientApi::GetDeviceBegin()
{
    return m_pApi->GetDeviceBegin();
}

std::map<std::string, std::shared_ptr<Device> >::const_iterator ClientApi::GetDeviceEnd()
{
    return m_pApi->GetDeviceEnd();
}

std::map<std::string, std::shared_ptr<Device> >::const_iterator ClientApi::FindDevice(const std::string& sUid)
{
    return m_pApi->FindDevice(sUid);
}

std::map<std::string, std::shared_ptr<Source> >::const_iterator ClientApi::GetSourceBegin()
{
    return m_pApi->GetSourceBegin();
}

std::map<std::string, std::shared_ptr<Source> >::const_iterator ClientApi::GetSourceEnd()
{
    return m_pApi->GetSourceEnd();
}

std::map<std::string, std::shared_ptr<Source> >::const_iterator ClientApi::FindSource(const std::string& sUid)
{
    return m_pApi->FindSource(sUid);
}

std::map<std::string, std::shared_ptr<Flow> >::const_iterator ClientApi::GetFlowBegin()
{
    return m_pApi->GetFlowBegin();
}

std::map<std::string, std::shared_ptr<Flow> >::const_iterator ClientApi::GetFlowEnd()
{
    return m_pApi->GetFlowEnd();
}

std::map<std::string, std::shared_ptr<Flow> >::const_iterator ClientApi::FindFlow(const std::string& sUid)
{
    return m_pApi->FindFlow(sUid);
}

std::map<std::string, std::shared_ptr<Sender> >::const_iterator ClientApi::GetSenderBegin()
{
    return m_pApi->GetSenderBegin();
}

std::map<std::string, std::shared_ptr<Sender> >::const_iterator ClientApi::GetSenderEnd()
{
    return m_pApi->GetSenderEnd();
}

std::map<std::string, std::shared_ptr<Sender> >::const_iterator ClientApi::FindSender(const std::string& sUid)
{
    return m_pApi->FindSender(sUid);
}

std::map<std::string, std::shared_ptr<Receiver> >::const_iterator ClientApi::GetReceiverBegin()
{
    return m_pApi->GetReceiverBegin();
}

std::map<std::string, std::shared_ptr<Receiver> >::const_iterator ClientApi::GetReceiverEnd()
{
    return m_pApi->GetReceiverEnd();
}

std::map<std::string, std::shared_ptr<Receiver> >::const_iterator ClientApi::FindReceiver(const std::string& sUid)
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
