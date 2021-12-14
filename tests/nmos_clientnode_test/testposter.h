#pragma once
#include "clientapiposter.h"


class TestPoster : public ClientApiPoster
{
    public:
        TestPoster(){};
        ~TestPoster(){};


    protected:

        virtual void ModeChanged(bool bQueryApi);
        virtual void NodeChanged(const std::list<std::shared_ptr<Self> >& lstNodesAdded, const std::list<std::shared_ptr<Self> >& lstNodesUpdated, const std::list<std::shared_ptr<Self> >& lstNodesRemoved);
        virtual void DeviceChanged(const std::list<std::shared_ptr<Device> >& lstDevicesAdded, const std::list<std::shared_ptr<Device> >& lstDevicesUpdated, const std::list<std::shared_ptr<Device> >& lstDevicesRemoved);
        virtual void SourceChanged(const std::list<std::shared_ptr<Source> >& lstSourcesAdded, const std::list<std::shared_ptr<Source> >& lstSourcesUpdated, const std::list<std::shared_ptr<Source> >& lstSourcesRemoved);
        virtual void FlowChanged(const std::list<std::shared_ptr<Flow> >& lstFlowsAdded, const std::list<std::shared_ptr<Flow> >& lstFlowsUpdated, const std::list<std::shared_ptr<Flow> >& lstFlowsRemoved);
        virtual void SenderChanged(const std::list<std::shared_ptr<Sender> >& lstSendersAdded, const std::list<std::shared_ptr<Sender> >& lstSendersUpdated, const std::list<std::shared_ptr<Sender> >& lstSendersRemoved);
        virtual void ReceiverChanged(const std::list<std::shared_ptr<Receiver> >& lstReceiversAdded, const std::list<std::shared_ptr<Receiver> >& lstReceiversUpdated, const std::list<std::shared_ptr<Receiver> >& lstReceiversRemoved);

        virtual void QuerySubscription(const std::string& sSubscriptionId, int nResource, const std::string& sQuery);
        virtual void QuerySubscriptionRemoved(const std::string& sSubscriptionId);

        virtual void RequestTargetResult(unsigned long nResult, const std::string& sResponse, const std::string& sResourceId);
        virtual void RequestPatchSenderResult(unsigned long nResult, const std::string& sResponse, const std::string& sResourceId);
        virtual void RequestPatchReceiverResult(unsigned long nResult, const std::string& sResponse, const std::string& sResourceId);
        virtual void RequestConnectResult(const std::string& sSenderId, const std::string& sReceiverId, bool bSuccess, const std::string& sResponse);

        virtual void RequestGetSenderStagedResult(unsigned long nResult, const std::string& sResponse, const std::string& sResourceId);
        virtual void RequestGetSenderActiveResult(unsigned long nResult, const std::string& sResponse, const std::string& sResourceId);

        virtual void RequestGetSenderTransportFileResult(unsigned long nResult, const std::string& sResponse, const std::string& sResourceId);
        virtual void RequestGetReceiverStagedResult(unsigned long nResult, const std::string& sResponse, const std::string& sResourceId);
        virtual void RequestGetReceiverActiveResult(unsigned long nResult, const std::string& sResponse, const std::string& sResourceId);


    private:

};

