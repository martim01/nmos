#include "clientapiposter.h"
#include "log.h"

using namespace pml::nmos;
void ClientApiPoster::_ModeChanged(bool bQueryApi)
{
    ModeChanged(bQueryApi);
}

void ClientApiPoster::_NodeChanged(const std::list<std::shared_ptr<const Self> >& lstNodesAdded, const std::list<std::shared_ptr<const Self> >& lstNodesUpdated, const std::list<std::shared_ptr<const Self> >& lstNodesRemoved)
{
    pmlLog(pml::LOG_DEBUG) << "NMOS: ClientApiPoster::_NodeChanged" ;
    NodeChanged(lstNodesAdded, lstNodesUpdated, lstNodesRemoved);
}

void ClientApiPoster::_DeviceChanged(const std::list<std::shared_ptr<const Device> >& lstDevicesAdded, const std::list<std::shared_ptr<const Device> >& lstDevicesUpdated, const std::list<std::shared_ptr<const Device> >& lstDevicesRemoved)
{
    pmlLog(pml::LOG_DEBUG) << "NMOS: ClientApiPoster::_DeviceChanged" ;
    DeviceChanged(lstDevicesAdded, lstDevicesUpdated, lstDevicesRemoved);
}

void ClientApiPoster::_SourceChanged(const std::list<std::shared_ptr<const Source> >& lstSourcesAdded, const std::list<std::shared_ptr<const Source> >& lstSourcesUpdated, const std::list<std::shared_ptr<const Source> >& lstSourcesRemoved)
{
    pmlLog(pml::LOG_DEBUG) << "NMOS: ClientApiPoster::_SourceChanged" ;
    SourceChanged(lstSourcesAdded, lstSourcesUpdated, lstSourcesRemoved);
}

void ClientApiPoster::_FlowChanged(const std::list<std::shared_ptr<const Flow> >& lstFlowsAdded, const std::list<std::shared_ptr<const Flow> >& lstFlowsUpdated, const std::list<std::shared_ptr<const Flow> >& lstFlowsRemoved)
{
    pmlLog(pml::LOG_DEBUG) << "NMOS: ClientApiPoster::_FlowChanged" ;
    FlowChanged(lstFlowsAdded, lstFlowsUpdated, lstFlowsRemoved);
}

void ClientApiPoster::_SenderChanged(const std::list<std::shared_ptr<const Sender> >& lstSendersAdded, const std::list<std::shared_ptr<const Sender> >& lstSendersUpdated, const std::list<std::shared_ptr<const Sender> >& lstSendersRemoved)
{
    pmlLog(pml::LOG_DEBUG) << "NMOS: ClientApiPoster::_SenderChanged" ;
    SenderChanged(lstSendersAdded, lstSendersUpdated, lstSendersRemoved);
}

void ClientApiPoster::_ReceiverChanged(const std::list<std::shared_ptr<const Receiver> >& lstReceiversAdded, const std::list<std::shared_ptr<const Receiver> >& lstReceiversUpdated, const std::list<std::shared_ptr<const Receiver> >& lstReceiversRemoved)
{
    pmlLog(pml::LOG_DEBUG) << "NMOS: ClientApiPoster::_ReceiverChanged" ;
    ReceiverChanged(lstReceiversAdded, lstReceiversUpdated, lstReceiversRemoved);
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
