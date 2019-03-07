#pragma once
#include "nmosdlldefine.h"
#include <string>
#include <map>
#include <memory>
#include "connection.h"

class Self;
class Device;
class Source;
class Flow;
class Sender;
class Receiver;
class ClientApiPoster;
class ClientApiImpl;

class NMOS_EXPOSE ClientApi
{
    public:

        enum flagResource {NONE=0, NODES=1, DEVICES=2, SOURCES=4, FLOWS=8, SENDERS=16, RECEIVERS=32, ALL=63};

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
        bool AddQuerySubscription(flagResource eResource, const std::string& sQuery="", unsigned long nUpdateRate=0);

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
        bool RequestSenderStaged(const std::string& sSenderId);

        /** @brief Requests the IS-05 active parameters from the given sender
        *   @param sSenderId the uuId of the Sender
        *   @return <i>bool</i> true if a sender with the given uuIds are found and it the url of is-05 connection api for the sender can be found, otherwise false
        *   @note the request is performed asynchronoulsy and the user should handle ClientApiPoster::RequestSenderActiveResult for the response
        **/
        bool RequestSenderActive(const std::string& sSenderId);

        /** @brief Requests the IS-05 transport file from the given sender
        *   @param sSenderId the uuId of the Sender
        *   @return <i>bool</i> true if a sender with the given uuIds are found and it the url of is-05 connection api for the sender can be found, otherwise false
        *   @note the request is performed asynchronoulsy and the user should handle ClientApiPoster::RequestSenderTransportFileResult for the response
        **/
        bool RequestSenderTransportFile(const std::string& sSenderId);

        /** @brief Requests the IS-05 staged parameters from the given Receiver
        *   @param sReceiverId the uuId of the Receiver
        *   @return <i>bool</i> true if a Receiver with the given uuIds are found and it the url of is-05 connection api for the Receiver can be found, otherwise false
        *   @note the request is performed asynchronoulsy and the user should handle ClientApiPoster::RequestReceiverStageResult for the response
        **/
        bool RequestReceiverStaged(const std::string& sReceiverId);


        /** @brief Requests the IS-05 active parameters from the given Receiver
        *   @param sReceiverId the uuId of the Receiver
        *   @return <i>bool</i> true if a Receiver with the given uuIds are found and it the url of is-05 connection api for the Receiver can be found, otherwise false
        *   @note the request is performed asynchronoulsy and the user should handle ClientApiPoster::RequestReceiverActiveResult for the response
        **/
        bool RequestReceiverActive(const std::string& sReceiverId);


        /** @brief Attempts to patch a sender's staged parameters
        *   @param sSenderId the uuId of the sender
        *   @param aConnection the connectionSender object that defines the parameters to be pached
        *   @return <i>bool</i> true if a Sender with the given uuIds are found and it the url of is-05 connection api for the Sender can be found, otherwise false
        *   @note the request is performed asynchronoulsy and the user should handle ClientApiPoster::RequestPatchSenderResult for the response
        **/
        bool PatchSenderStaged(const std::string& sSenderId, const connectionSender& aConnection);

        /** @brief Attempts to patch a Receiver's staged parameters
        *   @param sReceiverId the uuId of the Receiver
        *   @param aConnection the connectionReceiver object that defines the parameters to be pached
        *   @return <i>bool</i> true if a Receiver with the given uuIds are found and if the url of is-05 connection api for the Receiver can be found, otherwise false
        *   @note the request is performed asynchronoulsy and the user should handle ClientApiPoster::RequestPatchReceiverResult for the response
        **/
        bool PatchReceiverStaged(const std::string& sReceiverId, const connectionReceiver& aConnection);

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

        ///< @brief Gets a const_iterator to the beginning of the map containing all discovered nodes
        std::map<std::string, std::shared_ptr<Self> >::const_iterator GetNodeBegin();

        ///< @brief Gets a const_iterator to the end of the map containing all discovered nodes
        std::map<std::string, std::shared_ptr<Self> >::const_iterator GetNodeEnd();

        ///< @brief Gets a const_iterator pointing to the node with the given id, or to the end of the map
        std::map<std::string, std::shared_ptr<Self> >::const_iterator FindNode(const std::string& sUid);

        ///< @brief Gets a const_iterator to the beginning of the map containing all discovered devices
        std::map<std::string, std::shared_ptr<Device> >::const_iterator GetDeviceBegin();

        ///< @brief Gets a const_iterator to the end of the map containing all discovered devices
        std::map<std::string, std::shared_ptr<Device> >::const_iterator GetDeviceEnd();

        ///< @brief Gets a const_iterator pointing to the device with the given id, or to the end of the map
        std::map<std::string, std::shared_ptr<Device> >::const_iterator FindDevice(const std::string& sUid);

        ///< @brief Gets a const_iterator to the beginning of the map containing all discovered sources
        std::map<std::string, std::shared_ptr<Source> >::const_iterator GetSourceBegin();

        ///< @brief Gets a const_iterator to the end of the map containing all discovered sources
        std::map<std::string, std::shared_ptr<Source> >::const_iterator GetSourceEnd();

        ///< @brief Gets a const_iterator pointing to the source with the given id, or to the end of the map
        std::map<std::string, std::shared_ptr<Source> >::const_iterator FindSource(const std::string& sUid);

        ///< @brief Gets a const_iterator to the beginning of the map containing all discovered flows
        std::map<std::string, std::shared_ptr<Flow> >::const_iterator GetFlowBegin();

        ///< @brief Gets a const_iterator to the end of the map containing all discovered flows
        std::map<std::string, std::shared_ptr<Flow> >::const_iterator GetFlowEnd();

        ///< @brief Gets a const_iterator pointing to the flow with the given id, or to the end of the map
        std::map<std::string, std::shared_ptr<Flow> >::const_iterator FindFlow(const std::string& sUid);

        ///< @brief Gets a const_iterator to the beginning of the map containing all discovered senders
        std::map<std::string, std::shared_ptr<Sender> >::const_iterator GetSenderBegin();

        ///< @brief Gets a const_iterator to the end of the map containing all discovered senders
        std::map<std::string, std::shared_ptr<Sender> >::const_iterator GetSenderEnd();

        ///< @brief Gets a const_iterator pointing to the sender with the given id, or to the end of the map
        std::map<std::string, std::shared_ptr<Sender> >::const_iterator FindSender(const std::string& sUid);

        ///< @brief Gets a const_iterator to the beginning of the map containing all discovered receivers
        std::map<std::string, std::shared_ptr<Receiver> >::const_iterator GetReceiverBegin();

        ///< @brief Gets a const_iterator to the end of the map containing all discovered receivers
        std::map<std::string, std::shared_ptr<Receiver> >::const_iterator GetReceiverEnd();

        ///< @brief Gets a const_iterator pointing to the receiver with the given id, or to the end of the map
        std::map<std::string, std::shared_ptr<Receiver> >::const_iterator FindReceiver(const std::string& sUid);


    private:
        friend void NodeBrowser();
        friend void ClientThread();
        friend class ClientPoster;
        friend class ClientZCPoster;

        ClientApi();
        ~ClientApi();

        std::unique_ptr<ClientApiImpl> m_pApi;
};
