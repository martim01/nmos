#include "clientapiposter.h"
#include "log.h"


void ClientApiPoster::_ModeChanged(bool bQueryApi)
{
    ModeChanged(bQueryApi);
}

void ClientApiPoster::_NodeChanged(const std::list<std::shared_ptr<Self> >& lstNodesAdded, const std::list<std::shared_ptr<Self> >& lstNodesUpdated, const std::list<std::shared_ptr<Self> >& lstNodesRemoved)
{
    pmlLog(pml::LOG_DEBUG) << "NMOS: ClientApiPoster::_NodeChanged" ;
    NodeChanged(lstNodesAdded, lstNodesUpdated, lstNodesRemoved);
}

void ClientApiPoster::_DeviceChanged(const std::list<std::shared_ptr<Device> >& lstDevicesAdded, const std::list<std::shared_ptr<Device> >& lstDevicesUpdated, const std::list<std::shared_ptr<Device> >& lstDevicesRemoved)
{
    pmlLog(pml::LOG_DEBUG) << "NMOS: ClientApiPoster::_DeviceChanged" ;
    DeviceChanged(lstDevicesAdded, lstDevicesUpdated, lstDevicesRemoved);
}

void ClientApiPoster::_SourceChanged(const std::list<std::shared_ptr<Source> >& lstSourcesAdded, const std::list<std::shared_ptr<Source> >& lstSourcesUpdated, const std::list<std::shared_ptr<Source> >& lstSourcesRemoved)
{
    pmlLog(pml::LOG_DEBUG) << "NMOS: ClientApiPoster::_SourceChanged" ;
    SourceChanged(lstSourcesAdded, lstSourcesUpdated, lstSourcesRemoved);
}

void ClientApiPoster::_FlowChanged(const std::list<std::shared_ptr<Flow> >& lstFlowsAdded, const std::list<std::shared_ptr<Flow> >& lstFlowsUpdated, const std::list<std::shared_ptr<Flow> >& lstFlowsRemoved)
{
    pmlLog(pml::LOG_DEBUG) << "NMOS: ClientApiPoster::_FlowChanged" ;
    FlowChanged(lstFlowsAdded, lstFlowsUpdated, lstFlowsRemoved);
}

void ClientApiPoster::_SenderChanged(const std::list<std::shared_ptr<Sender> >& lstSendersAdded, const std::list<std::shared_ptr<Sender> >& lstSendersUpdated, const std::list<std::shared_ptr<Sender> >& lstSendersRemoved)
{
    pmlLog(pml::LOG_DEBUG) << "NMOS: ClientApiPoster::_SenderChanged" ;
    SenderChanged(lstSendersAdded, lstSendersUpdated, lstSendersRemoved);
}

void ClientApiPoster::_ReceiverChanged(const std::list<std::shared_ptr<Receiver> >& lstReceiversAdded, const std::list<std::shared_ptr<Receiver> >& lstReceiversUpdated, const std::list<std::shared_ptr<Receiver> >& lstReceiversRemoved)
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
