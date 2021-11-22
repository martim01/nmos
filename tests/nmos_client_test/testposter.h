#pragma once
#include "clientapiposter.h"


class TestPoster : public pml::nmos::ClientApiPoster
{
    public:
        TestPoster(){};
        ~TestPoster(){};


    protected:

        void ModeChanged(bool bQueryApi) override;
        void NodeChanged(const std::list<std::shared_ptr<const pml::nmos::Self> >& lstNodesAdded, const std::list<std::shared_ptr<const pml::nmos::Self> >& lstNodesUpdated, const std::list<std::shared_ptr<const pml::nmos::Self> >& lstNodesRemoved) override;
        void DeviceChanged(const std::list<std::shared_ptr<const pml::nmos::Device> >& lstDevicesAdded, const std::list<std::shared_ptr<const pml::nmos::Device> >& lstDevicesUpdated, const std::list<std::shared_ptr<const pml::nmos::Device> >& lstDevicesRemoved) override;
        void SourceChanged(const std::list<std::shared_ptr<const pml::nmos::Source> >& lstSourcesAdded, const std::list<std::shared_ptr<const pml::nmos::Source> >& lstSourcesUpdated, const std::list<std::shared_ptr<const pml::nmos::Source> >& lstSourcesRemoved) override;
        void FlowChanged(const std::list<std::shared_ptr<const pml::nmos::Flow> >& lstFlowsAdded, const std::list<std::shared_ptr<const pml::nmos::Flow> >& lstFlowsUpdated, const std::list<std::shared_ptr<const pml::nmos::Flow> >& lstFlowsRemoved) override;
        void SenderChanged(const std::list<std::shared_ptr<const pml::nmos::Sender> >& lstSendersAdded, const std::list<std::shared_ptr<const pml::nmos::Sender> >& lstSendersUpdated, const std::list<std::shared_ptr<const pml::nmos::Sender> >& lstSendersRemoved) override;
        void ReceiverChanged(const std::list<std::shared_ptr<const pml::nmos::Receiver> >& lstReceiversAdded, const std::list<std::shared_ptr<const pml::nmos::Receiver> >& lstReceiversUpdated, const std::list<std::shared_ptr<const pml::nmos::Receiver> >& lstReceiversRemoved) override;

        void QuerySubscription(const std::string& sSubscriptionId, int nResource, const std::string& sQuery) override;
        void QuerySubscriptionRemoved(const std::string& sSubscriptionId) override;

        void RequestTargetResult(unsigned long nResult, const std::string& sResponse, const std::string& sResourceId) override;
        void RequestPatchSenderResult(unsigned long nResult, const std::string& sResponse, const std::string& sResourceId) override;
        void RequestPatchReceiverResult(unsigned long nResult, const std::string& sResponse, const std::string& sResourceId) override;
        void RequestConnectResult(const std::string& sSenderId, const std::string& sReceiverId, bool bSuccess, const std::string& sResponse) override;

        void RequestGetSenderStagedResult(unsigned long nResult, const std::string& sResponse, const std::string& sResourceId) override;
        void RequestGetSenderActiveResult(unsigned long nResult, const std::string& sResponse, const std::string& sResourceId) override;

        void RequestGetSenderTransportFileResult(unsigned long nResult, const std::string& sResponse, const std::string& sResourceId) override;
        void RequestGetReceiverStagedResult(unsigned long nResult, const std::string& sResponse, const std::string& sResourceId) override;
        void RequestGetReceiverActiveResult(unsigned long nResult, const std::string& sResponse, const std::string& sResourceId) override;

        void QueryServerChanged(const std::string& sUrl) override;
        void QueryServerFound(const std::string& sUrl, unsigned short nPriority) override;
        void QueryServerRemoved(const std::string& sUrl) override;


    private:

};

