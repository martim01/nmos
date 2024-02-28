#include "nodeapi.h"
#include "nodeapiprivate.h"

using namespace pml::nmos;


NodeApi& NodeApi::Get()
{
    static NodeApi api;
    return api;
}

NodeApi::NodeApi() : m_pImpl(std::make_unique<NodeApiPrivate>())
{
}

NodeApi::~NodeApi()
{
}

void NodeApi::Init(std::shared_ptr<EventPoster> pPoster, unsigned short nDiscoveryPort, unsigned short nConnectionPort, const std::string& sLabel, const std::string& sDescription)
{
    m_pImpl->Init(pPoster, nDiscoveryPort, nConnectionPort, sLabel, sDescription);
}

bool NodeApi::AddControl(const std::string& sDeviceId, const std::string& sApi, const ApiVersion& version, unsigned short nPort, const std::string& sUrn, std::shared_ptr<NmosServer> pNmosServer)
{
   return m_pImpl->AddControl(sDeviceId, sApi, version, nPort, sUrn, pNmosServer);
}

bool NodeApi::StartServices()
{
    return m_pImpl->StartServices();
}

void NodeApi::StopServices()
{
    m_pImpl->StopServices();
}

bool NodeApi::Commit()
{
    return m_pImpl->Commit();
}

bool NodeApi::AddDevice(std::shared_ptr<Device> pResource)
{
    return m_pImpl->AddDevice(pResource);
}

bool NodeApi::AddSource(std::shared_ptr<Source> pResource)
{
    return m_pImpl->AddSource(pResource);
}

bool NodeApi::AddFlow(std::shared_ptr<Flow> pResource)
{
    return m_pImpl->AddFlow(pResource);
}

bool NodeApi::AddReceiver(std::shared_ptr<Receiver> pResource)
{
    return m_pImpl->AddReceiver(pResource);
}

bool NodeApi::AddSender(std::shared_ptr<Sender> pResource)
{
    return m_pImpl->AddSender(pResource);
}

Self& NodeApi::GetSelf()
{
    return m_pImpl->GetSelf();
}

const ResourceHolder<Source>& NodeApi::GetSources()
{
    return m_pImpl->GetSources();
}

const ResourceHolder<Device>& NodeApi::GetDevices()
{
    return m_pImpl->GetDevices();
}

const ResourceHolder<Flow>& NodeApi::GetFlows()
{
    return m_pImpl->GetFlows();
}

const ResourceHolder<Receiver>& NodeApi::GetReceivers()
{
    return m_pImpl->GetReceivers();
}

const ResourceHolder<Sender>& NodeApi::GetSenders()
{
    return m_pImpl->GetSenders();
}

void NodeApi::RemoveSender(const std::string& sId)
{
    m_pImpl->RemoveSender(sId);
}

void NodeApi::RemoveReceiver(const std::string& sId)
{
    m_pImpl->RemoveReceiver(sId);
}

void NodeApi::RemoveSource(const std::string& sId)
{
    m_pImpl->RemoveSource(sId);
}

std::shared_ptr<Receiver> NodeApi::GetReceiver(const std::string& sId)
{
    return m_pImpl->GetReceiver(sId);
}

std::shared_ptr<Sender> NodeApi::GetSender(const std::string& sId)
{
    return m_pImpl->GetSender(sId);
}

void NodeApi::TargetTaken(const std::string& sInterfaceIp, unsigned short nPort, bool bOk)
{
    m_pImpl->TargetTaken(sInterfaceIp, nPort, bOk);
}

void NodeApi::SenderPatchAllowed(unsigned short nPort, bool bOk, const std::string& sId, const std::vector<std::string>& vDestIp, const std::string& sSDP)
{
    m_pImpl->SenderPatchAllowed(nPort, bOk, sId, vDestIp, sSDP);
}

void NodeApi::ReceiverPatchAllowed(unsigned short nPort, bool bOk, const std::string& sId, const std::string& sInterfaceIp)
{
    m_pImpl->ReceiverPatchAllowed(nPort, bOk, sId, sInterfaceIp);
}

unsigned short NodeApi::GetConnectionPort() const
{
    return m_pImpl->GetConnectionPort();
}

unsigned short NodeApi::GetDiscoveryPort() const
{
    return m_pImpl->GetDiscoveryPort();
}

const std::string NodeApi::GetRegistrationNode() const
{
    return m_pImpl->GetRegistrationNode();
}

ApiVersion NodeApi::GetRegistrationVersion() const
{
    return m_pImpl->GetRegistrationVersion();
}

void NodeApi::SetHeartbeatTime(unsigned long nMilliseconds)
{
    m_pImpl->SetHeartbeatTime(nMilliseconds);
}

const std::chrono::system_clock::time_point& NodeApi::GetHeartbeatTime()
{
    return m_pImpl->GetHeartbeatTime();
}

bool NodeApi::AddBrowseDomain(const std::string& sDomain)
{
    return m_pImpl->AddBrowseDomain(sDomain);
}

bool NodeApi::RemoveBrowseDomain(const std::string& sDomain)
{
    return m_pImpl->RemoveBrowseDomain(sDomain);
}

