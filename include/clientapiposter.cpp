#include "clientapiposter.h"

void ClientApiPoster::_NodeChanged(std::shared_ptr<Self> pNode, enumChange eChange)
{
    NodeChanged(pNode, eChange);
}

void ClientApiPoster::_DeviceChanged(std::shared_ptr<Device> pDevice, enumChange eChange)
{
    DeviceChanged(pDevice, eChange);
}

void ClientApiPoster::_SourceChanged(std::shared_ptr<Source> pSource, enumChange eChange)
{
    SourceChanged(pSource, eChange);
}

void ClientApiPoster::_FlowChanged(std::shared_ptr<Flow> pFlow, enumChange eChange)
{
    FlowChanged(pFlow, eChange);
}

void ClientApiPoster::_SenderChanged(std::shared_ptr<Sender> pSender, enumChange eChange)
{
    SenderChanged(pSender, eChange);
}

void ClientApiPoster::_ReceiverChanged(std::shared_ptr<Receiver> pReceiver, enumChange eChange)
{
    ReceiverChanged(pReceiver, eChange);
}


void ClientApiPoster::_NodesRemoved(const std::set<std::string>& setRemoved)
{
    NodesRemoved(setRemoved);
}

void ClientApiPoster::_DevicesRemoved(const std::set<std::string>& setRemoved)
{
    DevicesRemoved(setRemoved);
}

void ClientApiPoster::_SourcesRemoved(const std::set<std::string>& setRemoved)
{
    SourcesRemoved(setRemoved);
}

void ClientApiPoster::_FlowsRemoved(const std::set<std::string>& setRemoved)
{
    FlowsRemoved(setRemoved);
}

void ClientApiPoster::_SendersRemoved(const std::set<std::string>& setRemoved)
{
    SendersRemoved(setRemoved);
}

void ClientApiPoster::_ReceiversRemoved(const std::set<std::string>& setRemoved)
{
    ReceiversRemoved(setRemoved);
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
