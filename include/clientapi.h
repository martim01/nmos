#pragma once
#include "nmosdlldefine.h"
#include <string>
#include <map>
#include <memory>
#include "connection.h"
#include "optional.hpp"

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
        class ClientApiPoster;
        class ClientApiImpl;
        struct curlResponse;
        struct connectionSender;
        struct connectionReceiver;
        class ConstraintsSender;
        class ConstraintsReceiver;

        class NMOS_EXPOSE ClientApi
        {
            public:

                enum enumResource { NODES=0, DEVICES=1, SOURCES=2, FLOWS=3, SENDERS=4, RECEIVERS=5};

                static ClientApi& Get();

                /** @brief Set the object derived from ClientApiPoster that will pass message back to the main thread
                *   @param pPoster pointer to an object derived from ClientApiPoster
                **/
                void SetPoster(std::shared_ptr<ClientApiPoster> pPoster);

                /** @brief Start the ClientApi services
                **/
                void Start();

                ///< @brief Stop the ClientApi services
                void Stop();


                /** @brief Adds a query subscription. That is tells the ClientApi what resources to notify us about
                *   @param eResource the resource type
                *   @param sQuery the query string. If empty then any resource of the given type cause a notification
                *   @param nUpdateRate the update rate in ms for messages. Ignored in P2P mode
                *   @return <i>bool</i>
                **/
                bool AddQuerySubscription(enumResource eResource, const std::string& sQuery="", unsigned long nUpdateRate=0);

                /** @brief Removes a query subscription
                *   @param sSubscriptionId the id of the subscription
                *   @return <i>bool</i>
                **/
                bool RemoveQuerySubscription(const std::string& sSubscriptionId);


                /** @brief Performs an IS-04 receiver subscription request
                *   @param sSenderId the uuId of the sender we want the receiver to subscribe to
                *   @param sReceiverId the uuId of the receiver
                *   @return <i>bool</i> true if a sender and receiver with the given uuIds are found and it the url of the receiver can be found, otherwise false
                *   @note the request is performed asynchronously and the user should handle ClientApiPoster::RequestTargetResult for the initial response and ClientApiPoster::ReceiverChanged for the final response
                **/
                bool Subscribe(const std::string& sSenderId, const std::string& sReceiverId);

                /** @brief Performs an IS-04 receiver subscription request using {} as the sender to unsubscribe the receiver
                *   @param sReceiverId the uuId of th receiver
                *   @return <i>bool</i> true if a receiver with the given uuIds are found and it the url of the receiver can be found, otherwise false
                *   @note the request is performed asynchronously and the user should handle ClientApiPoster::RequestTargetResult for the initial response and ClientApiPoster::ReceiverChanged for the final response
                **/
                bool Unsubscribe(const std::string& sReceiverId);

                /** @brief Requests the IS-05 staged parameters from the given sender
                *   @param sSenderId the uuId of the Sender
                *   @return <i>bool</i> true if a sender with the given uuIds are found and it the url of is-05 connection api for the sender can be found, otherwise false
                *   @note the request is performed asynchronoulsy and the user should handle ClientApiPoster::RequestSenderStageResult for the response
                **/
                std::pair<curlResponse, std::experimental::optional<connectionSender>> RequestSenderStaged(const std::string& sSenderId, bool bAsync);

                /** @brief Requests the IS-05 active parameters from the given sender
                *   @param sSenderId the uuId of the Sender
                *   @return <i>bool</i> true if a sender with the given uuIds are found and it the url of is-05 connection api for the sender can be found, otherwise false
                *   @note the request is performed asynchronoulsy and the user should handle ClientApiPoster::RequestSenderActiveResult for the response
                **/
                std::pair<curlResponse, std::experimental::optional<connectionSender>> RequestSenderActive(const std::string& sSenderId, bool bAsync);


                /** @brief Requests the IS-05 transport file from the given sender
                *   @param sSenderId the uuId of the Sender
                *   @return <i>bool</i> true if a sender with the given uuIds are found and it the url of is-05 connection api for the sender can be found, otherwise false
                *   @note the request is performed asynchronoulsy and the user should handle ClientApiPoster::RequestSenderTransportFileResult for the response
                **/
                 std::pair<curlResponse, std::experimental::optional<std::string>> RequestSenderTransportFile(const std::string& sSenderId, bool bAsync);


                std::pair<curlResponse, std::experimental::optional<std::vector<ConstraintsSender>>> RequestSenderConstraints(const std::string& sSenderId, bool bAsync);


                /** @brief Requests the IS-05 staged parameters from the given Receiver
                *   @param sReceiverId the uuId of the Receiver
                *   @return <i>bool</i> true if a Receiver with the given uuIds are found and it the url of is-05 connection api for the Receiver can be found, otherwise false
                *   @note the request is performed asynchronoulsy and the user should handle ClientApiPoster::RequestReceiverStageResult for the response
                **/
                std::pair<curlResponse, std::experimental::optional<connectionReceiver>>  RequestReceiverStaged(const std::string& sReceiverId, bool bAsync);



                /** @brief Requests the IS-05 active parameters from the given Receiver
                *   @param sReceiverId the uuId of the Receiver
                *   @return <i>bool</i> true if a Receiver with the given uuIds are found and it the url of is-05 connection api for the Receiver can be found, otherwise false
                *   @note the request is performed asynchronoulsy and the user should handle ClientApiPoster::RequestReceiverActiveResult for the response
                **/
                std::pair<curlResponse, std::experimental::optional<connectionReceiver>>  RequestReceiverActive(const std::string& sReceiverId, bool bAsync);

                std::pair<curlResponse, std::experimental::optional<std::vector<ConstraintsReceiver>>>  RequestReceiverConstraints(const std::string& sReceiverId, bool bAsync);



                /** @brief Attempts to patch a sender's staged parameters
                *   @param sSenderId the uuId of the sender
                *   @param aConnection the connectionSender object that defines the parameters to be pached
                *   @return <i>bool</i> true if a Sender with the given uuIds are found and it the url of is-05 connection api for the Sender can be found, otherwise false
                *   @note the request is performed asynchronoulsy and the user should handle ClientApiPoster::RequestPatchSenderResult for the response
                **/
                std::pair<curlResponse, std::experimental::optional<connectionSender>> PatchSenderStaged(const std::string& sSenderId, const connectionSender& aConnection, bool bAsync);

                /** @brief Attempts to patch a Receiver's staged parameters
                *   @param sReceiverId the uuId of the Receiver
                *   @param aConnection the connectionReceiver object that defines the parameters to be pached
                *   @return <i>bool</i> true if a Receiver with the given uuIds are found and if the url of is-05 connection api for the Receiver can be found, otherwise false
                *   @note the request is performed asynchronoulsy and the user should handle ClientApiPoster::RequestPatchReceiverResult for the response
                **/
                std::pair<curlResponse, std::experimental::optional<connectionReceiver>> PatchReceiverStaged(const std::string& sReceiverId, const connectionReceiver& aConnection, bool bAsync);

                /** @brief Attempts to connect a receiver to a sender using IS-05 connection. This function will stage and activate the sender with the given connection parameters, download the transport file from the sender and stage and activate the given transport file to the receiver.
                *   @param sSenderId the uuId of the sender
                *   @param sReceiverId the uuId of the receiver
                *   @return <i>bool</i> true if a Sender and Receiver with the given uuIds are found and if the urls of is-05 connection api for the Sender and Receiver can be found, otherwise false
                *   @note the request is performed asynchronoulsy and the user should handle ClientApiPoster::RequestPatchReceiverResult for the response
                **/
                bool Connect(const std::string& sSenderId, const std::string& sReceiverId);

                /** @brief Attempts to disconnect a receiver from a sender using IS-05 connection. This function will stage and activate the sender should it be a unicast stream and then stage and activate the receiver to remove any connection
                *   @param sReceiverId the uuId of the receiver
                *   @return <i>bool</i> true if a Receiver with the given uuId is found and if the url of is-05 connection api for the Receiver can be found, otherwise false
                *   @note the request is performed asynchronoulsy and the user should handle ClientApiPoster::RequestPatchReceiverResult for the response
                **/
                bool Disconnect(const std::string& sReceiverId);



                std::shared_ptr<const Self> FindNode(const std::string& sUid);
                std::shared_ptr<const Device> FindDevice(const std::string& sUid);
                std::shared_ptr<const Source> FindSource(const std::string& sUid);
                std::shared_ptr<const Flow> FindFlow(const std::string& sUid);
                std::shared_ptr<const Sender> FindSender(const std::string& sUid);
                std::shared_ptr<const Receiver> FindReceiver(const std::string& sUid);

                const std::map<std::string, std::shared_ptr<Self> >& GetNodes();
                const std::map<std::string, std::shared_ptr<Device> >& GetDevices();
                const std::map<std::string, std::shared_ptr<Source> >& GetSources();
                const std::map<std::string, std::shared_ptr<Flow> >& GetFlows();
                const std::map<std::string, std::shared_ptr<Sender> >& GetSenders();
                const std::map<std::string, std::shared_ptr<Receiver> >& GetReceivers();


                bool AddBrowseDomain(const std::string& sDomain);
                bool RemoveBrowseDomain(const std::string& sDomain);

            private:
                friend void NodeBrowser();
                friend void ClientThread();
                friend class ClientZCPoster;

                ClientApi();
                ~ClientApi();

                std::unique_ptr<ClientApiImpl> m_pApi;
        };
    };
};
