#pragma once
#include <string>
#include <memory>
#include "nmosdlldefine.h"
#include "resourceholder.h"
#include <chrono>
#include "self.h"
#include "device.h"
#include "source.h"
#include "flow.h"
#include "receiverbase.h"
#include "senderbase.h"

namespace pml
{
    namespace nmos
    {
        class NodeApiPrivate;
        class EventPoster;
        class Self;
        class Device;
        class Source;
        class Flow;
        class Receiver;
        class Sender;
        class ApiVersion;
        class NmosServer;


        class NMOS_EXPOSE NodeApi
        {
            public:

                static NodeApi& Get();

                /** @brief sets the initial data the Node needs to run
                *   @param pPoster pointer to an optional EventPoster class that can be used to update the main thread with events
                *   @param nDiscoveryPort the TCP port to run the discovery web serv on
                *   @param nConnectionPort the TCP port to run the connection port on
                *   @param sLabel the name to give this node
                *   @param sDescription a description aof this node
                **/
                void Init(std::shared_ptr<EventPoster> pPoster, unsigned short nDiscoveryPort, unsigned short nConnectionPort, const std::string& sLabel, const std::string& sDescription);

                /** @brief adds a device control (other than IS05)
                *   @param sDeviceId the id of the device to add the control to
                *   @param sApi the api that this control is for.This is the part of the url after x-nmos and before the version (e.g node for is04, connection for is05, channelmapping for is08)
                *   @param version the ApiVersion this control is for
                *   @param nPort the tcp/ip port to use
                *   @param sUrn the urn of the control
                *   @param pNmosServer a pointer to the derived NmosServer object that will handle the control
                *   @return <i>bool</i> true if the device is found
                *   @note To add the same server to more than one device pass the same port number and NULL for the Server
                **/
                bool AddControl(const std::string& sDeviceId, const std::string& sApi, const ApiVersion& version, unsigned short nPort, const std::string& sUrn, std::shared_ptr<NmosServer> pNmosServer);

                /** @brief Launch the thread that starts the web servers and dns publisher and browser
                *   @return <i>bool</i> true on succesfully starting the thread
                **/
                bool StartServices();

                /** @brief Stop the thread that is running the nmos services
                **/
                void StopServices();

                /** @brief Commit any changes that have been made to the model either updating the dns txt or the registry server
                *   @return <i>bool</i> true if there have been any changes to the model
                **/
                bool Commit();


                /** @brief Adds a device to the model
                *   @param pResource a pointer to the device
                *   @return <i>bool</i> true if succesfully added
                *   @note NodeApi takes ownership of the resource and will delete it on exit.
                *   @note You need to call Commit in order to publish the resource to the outside world
                **/
                bool AddDevice(std::shared_ptr<Device> pResource);


                /** @brief Adds a source to the model.
                *   @param pResource a pointer to the source
                *   @return <i>bool</i> true if succesfully added. This will fail if a source with the same id already exists or there is no device with the device_id of the source
                *   @note NodeApi takes ownership of the resource and will delete it on exit.
                *   @note You need to call Commit in order to publish the resource to the outside world
                **/
                bool AddSource(std::shared_ptr<Source> pResource);

                /** @brief Adds a flow to the model.
                *   @param pResource a pointer to the flow
                *   @return <i>bool</i> true if succesfully added. This will fail if a flow with the same id already exists or there is no device with the device_id of the flow or no source with the source_id of the flow
                *   @note NodeApi takes ownership of the resource and will delete it on exit.
                *   @note You need to call Commit in order to publish the resource to the outside world
                **/
                bool AddFlow(std::shared_ptr<Flow> pResource);

                /** @brief Adds a source to the model.
                *   @param pResource a pointer to the receiver
                *   @return <i>bool</i> true if succesfully added. This will fail if a receiver with the same id already exists or there is no flow with the flow_id of the receiver
                *   @note NodeApi takes ownership of the resource and will delete it on exit.
                *   @note You need to call Commit in order to publish the resource to the outside world
                **/
                bool AddReceiver(std::shared_ptr<Receiver> pResource);

                /** @brief Adds a sender to the model.
                *   @param pResource a pointer to the sender
                *   @return <i>bool</i> true if succesfully added. This will fail if a sender with the same id already exists or there is no flow with the flow_id of the sender
                *   @note NodeApi takes ownership of the resource and will delete it on exit.
                *   @note You need to call Commit in order to publish the resource to the outside world
                **/
                bool AddSender(std::shared_ptr<Sender> pResource);

                /** @brief Returns a reference to the Node /self
                *   @return <i>Self&</i>
                **/
                Self& GetSelf();

                /** @brief Returns a const reference to the ResourceHolder that contains all the sources
                *   @return </i>const ResourceHolder&</i>
                **/
                const ResourceHolder<Source>& GetSources();

                /** @brief Returns a const reference to the ResourceHolder that contains all the devices
                *   @return </i>const ResourceHolder&</i>
                **/
                const ResourceHolder<Device>& GetDevices();

                /** @brief Returns a const reference to the ResourceHolder that contains all the flows
                *   @return </i>const ResourceHolder&</i>
                **/
                const ResourceHolder<Flow>& GetFlows();

                /** @brief Returns a const reference to the ResourceHolder that contains all the receivers
                *   @return </i>const ResourceHolder&</i>
                **/
                const ResourceHolder<Receiver>& GetReceivers();

                /** @brief Returns a const reference to the ResourceHolder that contains all the senders
                *   @return </i>const ResourceHolder&</i>
                **/
                const ResourceHolder<Sender>& GetSenders();


                void RemoveSender(const std::string& sId);
                void RemoveReceiver(const std::string& sId);
                void RemoveSource(const std::string& sId);


                /** @brief Returns a pointer to the receiver with the given id if one exists or null.
                *   @param sId the uuid of the receiver
                *   @return <i>Receiver*</i> pointer to the given receiver or null
                **/
                std::shared_ptr<Receiver> GetReceiver(const std::string& sId);

                /** @brief Returns a pointer to the Sender with the given id if one exists or null.
                *   @param sId the uuid of the Sender
                *   @return <i>Sender*</i> pointer to the given Sender or null
                **/
                std::shared_ptr<Sender> GetSender(const std::string& sId);


                /** @brief To be called by the main thread when an IS-04 connection is made. The server tread will have called Target
                *   @param sInterfaceIp the ip address of the interface that is being used by the receiver to receive the flow
                *   @param nPort the port of the Server that is being used for IS-04
                *   @param bOk true if the receiver has succesfully connected to the sender. False otherwise
                **/
                void TargetTaken(const std::string& sInterfaceIp, unsigned short nPort, bool bOk);

                /** @brief To be called by the main thread when an IS-05 Sender Patch is being made. Also allows the main thread to set the parameters which will overwrite any "auto" stage params on activation
                *   @param nPort the port of the Server that is being used for IS-04
                *   @param bOk true if the Sender is allowed to take the patch.
                *   @param sId the uuid of the Sender
                *   @param sId the uuid of the Sender
                *   @param vSourceDestIp vector of source IP address from which RTP packets will be sent (IP address of interface bound to this output) and IP address to which RTP packets.will be sent. Can be a multicast address. Should be 2 entries for 2022-7 and otherwise 1
                *   @param sSDP the transport file that the sender should serve. This should be created by the main program logic from the staged transport parameters and associated flow etc. If empty then the Sender will create it's own from the connection parameters
                *   @note sSourceIp and sDestinationIp should be set to be the same as the Sender's staged parameters unless they are set to auto. In which case the main program logic should choose them.
                *   @note EventPoster::PatchSender is called on the request of the Sender being patched whatever the activation mode is. This function should be called by the main thread to allow the server to respond.
                **/
                void SenderPatchAllowed(unsigned short nPort, bool bOk, const std::string& sId, const std::vector<std::pair<std::string, std::string>>& vSourceDestIp, const std::string& sSDP="");

                /** @brief To be called by the main thread when an IS-05 Receiver Patch is being maded
                *   @param nPort the port of the Server that is being used for IS-04
                *   @param bOk true if the Receiver is allowed to take the patch
                *   @param sId the uuid of the Receiver
                *   @param sInterfaceIp IP address of the network interface the receiver should use
                *   @note EventPoster::PatchSender is called on the request of the Receiver being patched whatever the activation mode is. This function should be called by the main thread to allow the server to respond.
                **/
                void ReceiverPatchAllowed(unsigned short nPort, bool bOk, const std::string& sId, const std::string& sInterfaceIp);

                /** @brief returns the port number used for the connection api
                *   @return <i>unsigned short</i>
                **/
                unsigned short GetConnectionPort() const;

                /** @brief returns the port number used for the discovery api
                *   @return <i>unsigned short</i>
                **/
                unsigned short GetDiscoveryPort() const;

                /** @brief returns the name of the current registration node
                *   @param std::string the url or empty if in peer-peer mode
                **/
                const std::string GetRegistrationNode() const;

                /** @brief returns the version that the registration node is using
                *   @param ApiVersion - will be 0.0 if in peer-peer mode
                **/
                ApiVersion GetRegistrationVersion() const;


                /** @brief Sets the regularity that heartbeat messages are sent to a registry
                *   @param nMilliseconds the time between heartbeats in milliseconds
                **/
                void SetHeartbeatTime(unsigned long nMilliseconds);

                /** @brief Gets the regularity that heartbeat messages are sent to a registry
                *   @return <i>unsigned long</i> the time between heartbeats in milliseconds
                **/
                const std::chrono::system_clock::time_point& GetHeartbeatTime();


                //void ReceiverActivated(const std::string& sId);
                //void SenderActivated(const std::string& sId);

                /** @brief Adds a domain that the dns-sd browser should browse
                *   @param sDomain the domain name
                *   @return <i>bool</i> true if successful else false
                *   @note this function cannot be called when the NodeApi service is running
                **/
                bool AddBrowseDomain(const std::string& sDomain);

                /** @brief Removes a domain that the dns-sd browser should browse
                *   @param sDomain the domain name
                *   @return <i>bool</i> true if successful else false
                *   @note this function cannot be called when the NodeApi service is running
                **/
                bool RemoveBrowseDomain(const std::string& sDomain);

            private:
                NodeApi();
                ~NodeApi();
                std::unique_ptr<NodeApiPrivate> m_pImpl;
        };
    };
};

