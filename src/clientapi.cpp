#include "clientapi.h"
#include "clientprivate.h"
#include "curlregister.h"


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

std::shared_ptr<const Self> ClientApi::FindNode(const std::string& sUid)
{
    return m_pApi->FindNode(sUid);
}

const std::map<std::string, std::shared_ptr<Device> >& ClientApi::GetDevices()
{
    return m_pApi->GetDevices();
}

std::shared_ptr<const Device> ClientApi::FindDevice(const std::string& sUid)
{
    return m_pApi->FindDevice(sUid);
}

const std::map<std::string, std::shared_ptr<Source> >& ClientApi::GetSources()
{
    return m_pApi->GetSources();
}


std::shared_ptr<const Source> ClientApi::FindSource(const std::string& sUid)
{
    return m_pApi->FindSource(sUid);
}

const std::map<std::string, std::shared_ptr<Flow> >& ClientApi::GetFlows()
{
    return m_pApi->GetFlows();
}


std::shared_ptr<const Flow> ClientApi::FindFlow(const std::string& sUid)
{
    return m_pApi->FindFlow(sUid);
}

const std::map<std::string, std::shared_ptr<Sender> >& ClientApi::GetSenders()
{
    return m_pApi->GetSenders();
}


std::shared_ptr<const Sender> ClientApi::FindSender(const std::string& sUid)
{
    return m_pApi->FindSender(sUid);
}

const std::map<std::string, std::shared_ptr<Receiver> >& ClientApi::GetReceivers()
{
    return m_pApi->GetReceivers();
}

std::shared_ptr<const Receiver> ClientApi::FindReceiver(const std::string& sUid)
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


std::pair<curlResponse, std::experimental::optional<connectionSender<activationResponse>>>  ClientApi::RequestSenderStaged(const std::string& sSenderId, bool bAsync)
{
    return m_pApi->RequestSenderStaged(sSenderId, bAsync);
}

std::pair<curlResponse, std::experimental::optional<connectionSender<activationResponse>>>  ClientApi::RequestSenderActive(const std::string& sSenderId, bool bAsync)
{
    return m_pApi->RequestSenderActive(sSenderId, bAsync);
}

std::pair<curlResponse, std::experimental::optional<std::string>> ClientApi::RequestSenderTransportFile(const std::string& sSenderId, bool bAsync)
{
    return m_pApi->RequestSenderTransportFile(sSenderId, bAsync);
}

std::pair<curlResponse, std::experimental::optional<std::vector<Constraints>>> ClientApi::RequestSenderConstraints(const std::string& sSenderId, bool bAsync)
{
    return m_pApi->RequestSenderConstraints(sSenderId, bAsync);
}

std::pair<curlResponse, std::experimental::optional<connectionReceiver<activationResponse>>> ClientApi::RequestReceiverStaged(const std::string& sReceiverId, bool bAsync)
{
    return m_pApi->RequestReceiverStaged(sReceiverId, bAsync);
}

std::pair<curlResponse, std::experimental::optional<connectionReceiver<activationResponse>>> ClientApi::RequestReceiverActive(const std::string& sReceiverId, bool bAsync)
{
    return m_pApi->RequestReceiverActive(sReceiverId, bAsync);
}


std::pair<curlResponse, std::experimental::optional<std::vector<Constraints>>> ClientApi::RequestReceiverConstraints(const std::string& sReceiverId, bool bAsync)
{
    return m_pApi->RequestReceiverConstraints(sReceiverId, bAsync);
}

std::pair<curlResponse, std::experimental::optional<connectionSender<activationResponse>>> ClientApi::PatchSenderStaged(const std::string& sSenderId, const connectionSender<activationRequest>& aConnection, bool bAsync)
{
    return m_pApi->PatchSenderStaged(sSenderId, aConnection, bAsync);
}

std::pair<curlResponse, std::experimental::optional<connectionReceiver<activationResponse>>> ClientApi::PatchReceiverStaged(const std::string& sReceiverId, const connectionReceiver<activationRequest>& aConnection, bool bAsync)
{
    return m_pApi->PatchReceiverStaged(sReceiverId, aConnection, bAsync);
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


bool ClientApi::AddQuerySubscription(enumResource eResource, const std::string& sQuery, unsigned long nUpdateRate)
{
    return m_pApi->AddQuerySubscription(eResource, sQuery, nUpdateRate);
}

bool ClientApi::RemoveQuerySubscription(const std::string& sSubscriptionId)
{
    return m_pApi->RemoveQuerySubscription(sSubscriptionId);
}

bool ClientApi::AddBrowseDomain(const std::string& sDomain)
{
    return m_pApi->AddBrowseDomain(sDomain);
}

bool ClientApi::RemoveBrowseDomain(const std::string& sDomain)
{
    return m_pApi->RemoveBrowseDomain(sDomain);
}
