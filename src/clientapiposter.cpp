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
    pmlLog(pml::LOG_DEBUG) << "NMOS: ClientApiPoster::_NodeChanged" ;
    NodeChanged(changed);
}

void ClientApiPoster::_DeviceChanged(const resourcechanges<Device>& changed)
{
    pmlLog(pml::LOG_DEBUG) << "NMOS: ClientApiPoster::_DeviceChanged" ;
    DeviceChanged(changed);
}

void ClientApiPoster::_SourceChanged(const resourcechanges<Source>& changed)
{
    pmlLog(pml::LOG_DEBUG) << "NMOS: ClientApiPoster::_SourceChanged" ;
    SourceChanged(changed);
}

void ClientApiPoster::_FlowChanged(const resourcechanges<Flow>& changed)
{
    pmlLog(pml::LOG_DEBUG) << "NMOS: ClientApiPoster::_FlowChanged" ;
    FlowChanged(changed);
}

void ClientApiPoster::_SenderChanged(const resourcechanges<Sender>& changed)
{
    pmlLog(pml::LOG_DEBUG) << "NMOS: ClientApiPoster::_SenderChanged" ;
    SenderChanged(changed);
}

void ClientApiPoster::_ReceiverChanged(const resourcechanges<Receiver>& changed)
{
    pmlLog(pml::LOG_DEBUG) << "NMOS: ClientApiPoster::_ReceiverChanged" ;
    ReceiverChanged(changed);
}


void ClientApiPoster::_RequestTargetResult(unsigned long nResult, const std::string& sResponse, const std::string& sResourceId)
{
    RequestTargetResult(nResult, sResponse, sResourceId);
}

void ClientApiPoster::_RequestPatchSenderResult(unsigned long nResult, const std::string& sResponse, const std::string& sResourceId)
{
    RequestPatchSenderResult(nResult, sResponse, sResourceId);
}

void ClientApiPoster::_RequestPatchReceiverResult(unsigned long nResult, const std::string& sResponse, const std::string& sResourceId)
{
    RequestPatchReceiverResult(nResult, sResponse, sResourceId);
}

void ClientApiPoster::_RequestConnectResult(const std::string& sSenderId, const std::string& sReceiverId, bool bSuccess, const std::string& sResponse)
{
    RequestConnectResult(sSenderId, sReceiverId, bSuccess, sResponse);
}


void ClientApiPoster::_RequestGetSenderStagedResult(unsigned long nResult, const std::string& sResponse, const std::string& sResourceId)
{
    RequestGetSenderStagedResult(nResult, sResponse, sResourceId);
}

void ClientApiPoster::_RequestGetSenderActiveResult(unsigned long nResult, const std::string& sResponse, const std::string& sResourceId)
{
    RequestGetSenderActiveResult(nResult, sResponse, sResourceId);
}

void ClientApiPoster::_RequestGetSenderTransportFileResult(unsigned long nResult, const std::string& sResponse, const std::string& sResourceId)
{
    RequestGetSenderTransportFileResult(nResult, sResponse, sResourceId);
}

void ClientApiPoster::_RequestGetReceiverStagedResult(unsigned long nResult, const std::string& sResponse, const std::string& sResourceId)
{
    RequestGetReceiverStagedResult(nResult, sResponse, sResourceId);
}

void ClientApiPoster::_RequestGetReceiverActiveResult(unsigned long nResult, const std::string& sResponse, const std::string& sResourceId)
{
    RequestGetReceiverActiveResult(nResult, sResponse, sResourceId);
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
