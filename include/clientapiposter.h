#pragma once
#include "nmosdlldefine.h"
#include <string>
#include <memory>
#include <set>
#include <list>
#include <algorithm>
#include "curlregister.h"
#include "optional.hpp"
#include "connection.h"
#include "activation.h"

namespace pml
{
    namespace nmos
    {
        class Self;
        class Device;
        class Source;
        class Flow;
        class Sender;
        class Receiver;


        template<class T> struct resourcechanges
        {
            std::list<std::shared_ptr<const T> > lstAdded;
            std::list<std::shared_ptr<const T> > lstUpdated;
            std::list<std::shared_ptr<const T> > lstRemoved;

            resourcechanges& operator += (const resourcechanges<T>& changed)
            {
                std::copy(changed.lstAdded.begin(), changed.lstAdded.end(), std::back_inserter(lstAdded));
                std::copy(changed.lstUpdated.begin(), changed.lstUpdated.end(), std::back_inserter(lstUpdated));
                std::copy(changed.lstRemoved.begin(), changed.lstRemoved.end(), std::back_inserter(lstRemoved));
                return *this;
            }
        };


        class NMOS_EXPOSE ClientApiPoster
        {
            public:
                ClientApiPoster(){};
                virtual ~ClientApiPoster();

                enum enumChange{RESOURCE_ADDED=1, RESOURCE_UPDATED};

                void _ModeChanged(bool bQueryApi);
                void _NodeChanged(const resourcechanges<Self>& changed);
                void _DeviceChanged(const resourcechanges<Device>& changed);
                void _SourceChanged(const resourcechanges<Source>& changed);
                void _FlowChanged(const resourcechanges<Flow>& changed);
                void _SenderChanged(const resourcechanges<Sender>& changed);
                void _ReceiverChanged(const resourcechanges<Receiver>& changed);


                void _RequestTargetResult(unsigned long nResult, const std::string& sResponse, const std::string& sResourceId);
                void _RequestPatchSenderResult(const curlResponse& resp, const std::experimental::optional<connectionSender<activationResponse>>& con, const std::string& sResourceId);
                void _RequestPatchReceiverResult(const curlResponse& resp, const std::experimental::optional<connectionReceiver<activationResponse>>& con, const std::string& sResourceId);
                void _RequestGetSenderStagedResult(const curlResponse& resp, const std::experimental::optional<connectionSender<activationResponse>>& con, const std::string& sResourceId);
                void _RequestGetSenderActiveResult(const curlResponse& resp, const std::experimental::optional<connectionSender<activationResponse>>& con, const std::string& sResourceId);
                void _RequestGetSenderTransportFileResult(const curlResponse& resp, const std::experimental::optional<std::string>& sTransportFile, const std::string& sResourceId);
                void _RequestGetSenderConstraintsResult(const curlResponse& resp, const std::vector<Constraints>& vConstraints, const std::string& sResourceId);
                void _RequestGetReceiverStagedResult(const curlResponse& resp, const std::experimental::optional<connectionReceiver<activationResponse>>& con, const std::string& sResourceId);
                void _RequestGetReceiverActiveResult(const curlResponse& resp, const std::experimental::optional<connectionReceiver<activationResponse>>& con, const std::string& sResourceId);
                void _RequestGetReceiverConstraintsResult(const curlResponse& resp, const std::vector<Constraints>& vConstraints, const std::string& sResourceId);

                void _RequestConnectResult(const std::string& sSenderId, const std::string& sReceiverId, bool bSuccess, const std::string& sResponse);

                void _QueryServerFound(const std::string& sUrl, unsigned short nPriority);
                void _QueryServerRemoved(const std::string& sUrl);
                void _QueryServerChanged(const std::string& sUrl);

                void _QuerySubscription(const std::string& sSubscriptionId, int nResource, const std::string& sQuery);
                void _QuerySubscriptionRemoved(const std::string& sSubscriptionId);

            protected:

                /** @brief Called when the API changes from P2P to Query and vice versa
                *   @param bQueryApi if true then a query node has been found
                **/
                virtual void ModeChanged(bool bQueryApi)=0;

                /** @brief called when one or more Self resources that match a subscribed query are added, updated or removed
                *   @param lstNodesAdded a list of Selfs that have been added
                *   @param lstNodesUpdated a list of Selfs that have been updated
                *   @param lstNodesRemoved a list of Selfs that have been removed
                **/
                virtual void NodeChanged(const resourcechanges<Self>& changed)=0;

                /** @brief called when one or more Device resources that match a subscribed query are added, updated or removed
                *   @param lstNodesAdded a list of Devices that have been added
                *   @param lstNodesUpdated a list of Devices that have been updated
                *   @param lstNodesRemoved a list of Devices that have been removed
                **/
                virtual void DeviceChanged(const resourcechanges<Device>& changed)=0;

                /** @brief called when one or more Source resources that match a subscribed query are added, updated or removed
                *   @param lstNodesAdded a list of Sources that have been added
                *   @param lstNodesUpdated a list of Sources that have been updated
                *   @param lstNodesRemoved a list of Sources that have been removed
                **/
                virtual void SourceChanged(const resourcechanges<Source>& changed)=0;

                /** @brief called when one or more Flow resources that match a subscribed query are added, updated or removed
                *   @param lstNodesAdded a list of Flows that have been added
                *   @param lstNodesUpdated a list of Flows that have been updated
                *   @param lstNodesRemoved a list of Flows that have been removed
                **/
                virtual void FlowChanged(const resourcechanges<Flow>& changed)=0;

                /** @brief called when one or more Sender resources that match a subscribed query are added, updated or removed
                *   @param lstNodesAdded a list of Senders that have been added
                *   @param lstNodesUpdated a list of Senders that have been updated
                *   @param lstNodesRemoved a list of Senders that have been removed
                **/
                virtual void SenderChanged(const resourcechanges<Sender>& changed)=0;

                /** @brief called when one or more Receiver resources that match a subscribed query are added, updated or removed
                *   @param lstNodesAdded a list of Receivers that have been added
                *   @param lstNodesUpdated a list of Receivers that have been updated
                *   @param lstNodesRemoved a list of Receivers that have been removed
                **/
                virtual void ReceiverChanged(const resourcechanges<Receiver>& changed)=0;

                /** @brief Called when when a query is subscibed (either with the query node or in p2p mode)
                *   @param sSubscriptionId the return id of the subscription. On failure this will be an empty string
                *   @param nResource the nmos resource type we are querying
                *   @param sQuery the query string if any
                **/
                virtual void QuerySubscription(const std::string& sSubscriptionId, int nResource, const std::string& sQuery)=0;

                /** @brief Called when a query subscritption is removed
                *   @param sSubscriptionId the id of the remove subscription
                **/
                virtual void QuerySubscriptionRemoved(const std::string& sSubscriptionId)=0;

                virtual void QueryServerChanged(const std::string& sUrl)=0;
                virtual void QueryServerFound(const std::string& sUrl, unsigned short nPriority)=0;
                virtual void QueryServerRemoved(const std::string& sUrl)=0;

                /** @brief Called when a ClientApi::Subscribe or ClientApi::Unsubscribe gets an answer from the target webserver
                *   @param nResult the http result code
                *   @param sResonse on success this will contain the uuId of the Sender, on failure this will contain a message with the reason for failure
                *   @param sResourceId contains the uuId of the Receiver
                **/
                virtual void RequestTargetResult(unsigned long nResult, const std::string& sResponse, const std::string& sResourceId)=0;


                virtual void RequestPatchSenderResult(const curlResponse& resp, const std::experimental::optional<connectionSender<activationResponse>>& con, const std::string& sResourceId) = 0;
                virtual void RequestPatchReceiverResult(const curlResponse& resp, const std::experimental::optional<connectionReceiver<activationResponse>>& con, const std::string& sResourceId) = 0;
                virtual void RequestGetSenderStagedResult(const curlResponse& resp, const std::experimental::optional<connectionSender<activationResponse>>& con, const std::string& sResourceId) = 0;
                virtual void RequestGetSenderActiveResult(const curlResponse& resp, const std::experimental::optional<connectionSender<activationResponse>>& con, const std::string& sResourceId) = 0;
                virtual void RequestGetSenderConstraintsResult(const curlResponse& resp, const std::vector<Constraints>& vConstraints, const std::string& sResourceId) = 0;
                virtual void RequestGetSenderTransportFileResult(const curlResponse& resp, const std::experimental::optional<std::string>& sTransportFile, const std::string& sResourceId) = 0;
                virtual void RequestGetReceiverStagedResult(const curlResponse& resp, const std::experimental::optional<connectionReceiver<activationResponse>>& con, const std::string& sResourceId) = 0;
                virtual void RequestGetReceiverActiveResult(const curlResponse& resp, const std::experimental::optional<connectionReceiver<activationResponse>>& con, const std::string& sResourceId) = 0;
                virtual void RequestGetReceiverConstraintsResult(const curlResponse& resp, const std::vector<Constraints>& vConstraints, const std::string& sResourceId) = 0;

                virtual void RequestConnectResult(const std::string& sSenderId, const std::string& sReceiverId, bool bSuccess, const std::string& sResponse)=0;


            private:

        };
    };
};
