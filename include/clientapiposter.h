#pragma once
#include "nmosdlldefine.h"
#include <string>
#include <memory>
#include <set>


class Self;
class Device;
class Source;
class Flow;
class Sender;
class Receiver;

class NMOS_EXPOSE ClientApiPoster
{
    public:
        ClientApiPoster(){};
        ~ClientApiPoster(){};

        enum enumChange{RESOURCE_ADDED=1, RESOURCE_UPDATED};

        void _NodeChanged(std::shared_ptr<Self> pNode, enumChange eChange);
        void _DeviceChanged(std::shared_ptr<Device> pDevice, enumChange eChange);
        void _SourceChanged(std::shared_ptr<Source> pSource, enumChange eChange);
        void _FlowChanged(std::shared_ptr<Flow> pFlow, enumChange eChange);
        void _SenderChanged(std::shared_ptr<Sender> pSender, enumChange eChange);
        void _ReceiverChanged(std::shared_ptr<Receiver> pReceiver, enumChange eChange);

        void _NodesRemoved(const std::set<std::string>& setRemoved);
        void _DevicesRemoved(const std::set<std::string>& setRemoved);
        void _SourcesRemoved(const std::set<std::string>& setRemoved);
        void _FlowsRemoved(const std::set<std::string>& setRemoved);
        void _SendersRemoved(const std::set<std::string>& setRemoved);
        void _ReceiversRemoved(const std::set<std::string>& setRemoved);

        void _RequestTargetResult(unsigned long nResult, const std::string& sResponse, const std::string& sResourceId);
        void _RequestPatchSenderResult(unsigned long nResult, const std::string& sResponse, const std::string& sResourceId);
        void _RequestPatchReceiverResult(unsigned long nResult, const std::string& sResponse, const std::string& sResourceId);


    protected:
        virtual void NodeChanged(std::shared_ptr<Self> pNode, enumChange eChange)=0;
        virtual void DeviceChanged(std::shared_ptr<Device> pDevice, enumChange eChange)=0;
        virtual void SourceChanged(std::shared_ptr<Source> pSource, enumChange eChange)=0;
        virtual void FlowChanged(std::shared_ptr<Flow> pFlow, enumChange eChange)=0;
        virtual void SenderChanged(std::shared_ptr<Sender> pSender, enumChange eChange)=0;
        virtual void ReceiverChanged(std::shared_ptr<Receiver> pReceiver, enumChange eChange)=0;

        virtual void NodesRemoved(const std::set<std::string>& setRemoved)=0;
        virtual void DevicesRemoved(const std::set<std::string>& setRemoved)=0;
        virtual void SourcesRemoved(const std::set<std::string>& setRemoved)=0;
        virtual void FlowsRemoved(const std::set<std::string>& setRemoved)=0;
        virtual void SendersRemoved(const std::set<std::string>& setRemoved)=0;
        virtual void ReceiversRemoved(const std::set<std::string>& setRemoved)=0;

        virtual void RequestTargetResult(unsigned long nResult, const std::string& sResponse, const std::string& sResourceId)=0;
        virtual void RequestPatchSenderResult(unsigned long nResult, const std::string& sResponse, const std::string& sResourceId)=0;
        virtual void RequestPatchReceiverResult(unsigned long nResult, const std::string& sResponse, const std::string& sResourceId)=0;

    private:

};
