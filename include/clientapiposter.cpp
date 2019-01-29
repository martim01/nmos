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
