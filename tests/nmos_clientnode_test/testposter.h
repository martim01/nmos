#pragma once
#include "clientapiposter.h"


class TestPoster : public ClientApiPoster
{
    public:
        TestPoster(){};
        ~TestPoster(){};


    protected:
        virtual void NodeChanged(std::shared_ptr<Self> pNode, enumChange eChange);
        virtual void DeviceChanged(std::shared_ptr<Device> pDevice, enumChange eChange);
        virtual void SourceChanged(std::shared_ptr<Source> pSource, enumChange eChange);
        virtual void FlowChanged(std::shared_ptr<Flow> pFlow, enumChange eChange);
        virtual void SenderChanged(std::shared_ptr<Sender> pSender, enumChange eChange);
        virtual void ReceiverChanged(std::shared_ptr<Receiver> pReceiver, enumChange eChange);

        virtual void NodesRemoved(const std::set<std::string>& setRemoved);
        virtual void DevicesRemoved(const std::set<std::string>& setRemoved);
        virtual void SourcesRemoved(const std::set<std::string>& setRemoved);
        virtual void FlowsRemoved(const std::set<std::string>& setRemoved);
        virtual void SendersRemoved(const std::set<std::string>& setRemoved);
        virtual void ReceiversRemoved(const std::set<std::string>& setRemoved);

        virtual void RequestTargetResult(unsigned long nResult, const std::string& sResponse, const std::string& sResourceId){}


        virtual void RequestPatchSenderResult(unsigned long nResult, const std::string& sResponse, const std::string& sResourceId){}
        virtual void RequestPatchReceiverResult(unsigned long nResult, const std::string& sResponse, const std::string& sResourceId){};

        virtual void RequestConnectResult(const std::string& sSenderId, const std::string& sReceiverId, bool bSuccess, const std::string& sResponse){};


    private:

};

