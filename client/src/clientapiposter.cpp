#include "clientapiposter.h"
#include "log.h"

using namespace pml::nmos;

ClientApiPoster::~ClientApiPoster()
{
}


void ClientApiPoster::_ModeChanged(bool bQueryApi)
{
    ModeChanged(bQueryApi);
}

void ClientApiPoster::_NodeChanged(const resourcechanges<Self>& changed)
{
    pmlLog(pml::LOG_DEBUG, "pml::nmos") << "NMOS: ClientApiPoster::_NodeChanged" ;
    NodeChanged(changed);
}

void ClientApiPoster::_DeviceChanged(const resourcechanges<Device>& changed)
{
    pmlLog(pml::LOG_DEBUG, "pml::nmos") << "NMOS: ClientApiPoster::_DeviceChanged" ;
    DeviceChanged(changed);
}

void ClientApiPoster::_SourceChanged(const resourcechanges<Source>& changed)
{
    pmlLog(pml::LOG_DEBUG, "pml::nmos") << "NMOS: ClientApiPoster::_SourceChanged" ;
    SourceChanged(changed);
}

void ClientApiPoster::_FlowChanged(const resourcechanges<Flow>& changed)
{
    pmlLog(pml::LOG_DEBUG, "pml::nmos") << "NMOS: ClientApiPoster::_FlowChanged" ;
    FlowChanged(changed);
}

void ClientApiPoster::_SenderChanged(const resourcechanges<Sender>& changed)
{
    pmlLog(pml::LOG_DEBUG, "pml::nmos") << "NMOS: ClientApiPoster::_SenderChanged" ;
    SenderChanged(changed);
}

void ClientApiPoster::_ReceiverChanged(const resourcechanges<Receiver>& changed)
{
    pmlLog(pml::LOG_DEBUG, "pml::nmos") << "NMOS: ClientApiPoster::_ReceiverChanged" ;
    ReceiverChanged(changed);
}


void ClientApiPoster::_RequestTargetResult(unsigned long nResult, const std::string& sResponse, const std::string& sResourceId)
{
    RequestTargetResult(nResult, sResponse, sResourceId);
}

void ClientApiPoster::_RequestPatchSenderResult(const pml::restgoose::clientResponse& resp, const std::optional<connectionSender<activationResponse>>& con, const std::string& sResourceId)
{
    RequestPatchSenderResult(resp, con, sResourceId);
}

void ClientApiPoster::_RequestPatchReceiverResult(const pml::restgoose::clientResponse& resp, const std::optional<connectionReceiver<activationResponse>>& con, const std::string& sResourceId)
{
    RequestPatchReceiverResult(resp, con, sResourceId);
}

void ClientApiPoster::_RequestConnectResult(const std::string& sSenderId, const std::string& sReceiverId, bool bSuccess, const std::string& sResponse)
{
    RequestConnectResult(sSenderId, sReceiverId, bSuccess, sResponse);
}


void ClientApiPoster::_RequestGetSenderStagedResult(const pml::restgoose::clientResponse& resp, const std::optional<connectionSender<activationResponse>>& con, const std::string& sResourceId)
{
    RequestGetSenderStagedResult(resp, con, sResourceId);
}

void ClientApiPoster::_RequestGetSenderActiveResult(const pml::restgoose::clientResponse& resp, const std::optional<connectionSender<activationResponse>>& con, const std::string& sResourceId)
{
    RequestGetSenderActiveResult(resp, con, sResourceId);
}

void ClientApiPoster::_RequestGetSenderTransportFileResult(const pml::restgoose::clientResponse& resp, const std::optional<std::string>& sTransportFile, const std::string& sResourceId)
{
    RequestGetSenderTransportFileResult(resp, sTransportFile, sResourceId);
}

void ClientApiPoster::_RequestGetReceiverStagedResult(const pml::restgoose::clientResponse& resp, const std::optional<connectionReceiver<activationResponse>>& con, const std::string& sResourceId)
{
    RequestGetReceiverStagedResult(resp, con, sResourceId);
}

void ClientApiPoster::_RequestGetReceiverActiveResult(const pml::restgoose::clientResponse& resp, const std::optional<connectionReceiver<activationResponse>>& con, const std::string& sResourceId)
{
    RequestGetReceiverActiveResult(resp, con, sResourceId);
}

void ClientApiPoster::_RequestGetSenderConstraintsResult(const pml::restgoose::clientResponse& resp, const std::vector<Constraints>& vConstraints, const std::string& sResourceId)
{
    RequestGetSenderConstraintsResult(resp, vConstraints, sResourceId);
}

void ClientApiPoster::_RequestGetReceiverConstraintsResult(const pml::restgoose::clientResponse& resp, const std::vector<Constraints>& vConstraints, const std::string& sResourceId)
{
    RequestGetReceiverConstraintsResult(resp, vConstraints, sResourceId);
}

void ClientApiPoster::_QuerySubscription(const std::string& sSubscriptionId, int nResource, const std::string& sQuery)
{
    QuerySubscription(sSubscriptionId,nResource,sQuery);
}

void ClientApiPoster::_QuerySubscriptionRemoved(const std::string& sSubscriptionId)
{
    QuerySubscriptionRemoved(sSubscriptionId);
}

void ClientApiPoster::_QueryServerFound(const std::string& sUrl, unsigned short nPriority)
{
    QueryServerFound(sUrl, nPriority);
}

void ClientApiPoster::_QueryServerRemoved(const std::string& sUrl)
{
    QueryServerRemoved(sUrl);
}

void ClientApiPoster::_QueryServerChanged(const std::string& sUrl)
{
    QueryServerChanged(sUrl);
}
