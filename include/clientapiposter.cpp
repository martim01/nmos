#include "clientapiposter.h"


void ClientApiPoster::_ModeChanged(bool bQueryApi)
{
    ModeChanged(bQueryApi);
}

void ClientApiPoster::_NodeChanged(const std::list<std::shared_ptr<Self> >& lstNodesAdded, const std::list<std::shared_ptr<Self> >& lstNodesUpdated, const std::list<std::shared_ptr<Self> >& lstNodesRemoved)
{
    NodeChanged(lstNodesAdded, lstNodesUpdated, lstNodesRemoved);
}

void ClientApiPoster::_DeviceChanged(const std::list<std::shared_ptr<Device> >& lstDevicesAdded, const std::list<std::shared_ptr<Device> >& lstDevicesUpdated, const std::list<std::shared_ptr<Device> >& lstDevicesRemoved)
{
    DeviceChanged(lstDevicesAdded, lstDevicesUpdated, lstDevicesRemoved);
}

void ClientApiPoster::_SourceChanged(const std::list<std::shared_ptr<Source> >& lstSourcesAdded, const std::list<std::shared_ptr<Source> >& lstSourcesUpdated, const std::list<std::shared_ptr<Source> >& lstSourcesRemoved)
{
    SourceChanged(lstSourcesAdded, lstSourcesUpdated, lstSourcesRemoved);
}

void ClientApiPoster::_FlowChanged(const std::list<std::shared_ptr<Flow> >& lstFlowsAdded, const std::list<std::shared_ptr<Flow> >& lstFlowsUpdated, const std::list<std::shared_ptr<Flow> >& lstFlowsRemoved)
{
    FlowChanged(lstFlowsAdded, lstFlowsUpdated, lstFlowsRemoved);
}

void ClientApiPoster::_SenderChanged(const std::list<std::shared_ptr<Sender> >& lstSendersAdded, const std::list<std::shared_ptr<Sender> >& lstSendersUpdated, const std::list<std::shared_ptr<Sender> >& lstSendersRemoved)
{
    SenderChanged(lstSendersAdded, lstSendersUpdated, lstSendersRemoved);
}

void ClientApiPoster::_ReceiverChanged(const std::list<std::shared_ptr<Receiver> >& lstReceiversAdded, const std::list<std::shared_ptr<Receiver> >& lstReceiversUpdated, const std::list<std::shared_ptr<Receiver> >& lstReceiversRemoved)
{
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
