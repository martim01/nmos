#pragma once
#include "clientapiposter.h"


class TestPoster : public pml::nmos::ClientApiPoster
{
    public:
        TestPoster(){};
        ~TestPoster(){};


    protected:

        void ModeChanged(bool bQueryApi) override;
        void NodeChanged(const pml::nmos::resourcechanges<pml::nmos::Self>& changed) override;
        void DeviceChanged(const pml::nmos::resourcechanges<pml::nmos::Device>& changed) override;
        void SourceChanged(const pml::nmos::resourcechanges<pml::nmos::Source>& changed) override;
        void FlowChanged(const pml::nmos::resourcechanges<pml::nmos::Flow>& changed) override;
        void SenderChanged(const pml::nmos::resourcechanges<pml::nmos::Sender>& changed) override;
        void ReceiverChanged(const pml::nmos::resourcechanges<pml::nmos::Receiver>& changed) override;

        void QuerySubscription(const std::string& sSubscriptionId, int nResource, const std::string& sQuery) override;
        void QuerySubscriptionRemoved(const std::string& sSubscriptionId) override;

        void RequestTargetResult(unsigned long nResult, const std::string& sResponse, const std::string& sResourceId) override;
        void RequestPatchSenderResult(const pml::restgoose::clientResponse& resp, const std::optional<pml::nmos::connectionSender<pml::nmos::activationResponse>>& con, const std::string& sResourceId) override;
        void RequestPatchReceiverResult(const pml::restgoose::clientResponse& resp, const std::optional<pml::nmos::connectionReceiver<pml::nmos::activationResponse>>& con, const std::string& sResourceId) override;
        void RequestConnectResult(const std::string& sSenderId, const std::string& sReceiverId, bool bSuccess, const std::string& sResponse) override;

        void RequestGetSenderStagedResult(const pml::restgoose::clientResponse& resp, const std::optional<pml::nmos::connectionSender<pml::nmos::activationResponse>>& con, const std::string& sResourceId) override;
        void RequestGetSenderActiveResult(const pml::restgoose::clientResponse& resp, const std::optional<pml::nmos::connectionSender<pml::nmos::activationResponse>>& con, const std::string& sResourceId) override;
        void RequestGetSenderTransportFileResult(const pml::restgoose::clientResponse& resp, const std::optional<std::string>& sTransportFile, const std::string& sResourceId) override;

        void RequestGetSenderConstraintsResult(const pml::restgoose::clientResponse& resp, const std::vector<pml::nmos::Constraints>& vConstraints, const std::string& sResourceId) override;
        void RequestGetReceiverConstraintsResult(const pml::restgoose::clientResponse& resp, const std::vector<pml::nmos::Constraints>& vConstraints, const std::string& sResourceId) override;

        void RequestGetReceiverStagedResult(const pml::restgoose::clientResponse& resp, const std::optional<pml::nmos::connectionReceiver<pml::nmos::activationResponse>>& con, const std::string& sResourceId) override;
        void RequestGetReceiverActiveResult(const pml::restgoose::clientResponse& resp, const std::optional<pml::nmos::connectionReceiver<pml::nmos::activationResponse>>& con, const std::string& sResourceId) override;

        void QueryServerChanged(const std::string& sUrl) override;
        void QueryServerFound(const std::string& sUrl, unsigned short nPriority) override;
        void QueryServerRemoved(const std::string& sUrl) override;


    private:

};

