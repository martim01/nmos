#pragma once
#include "self.h"
#include "resourceholder.h"
#include <vector>
#include <string>
#include "nmosdlldefine.h"
#include <mutex>
#include <condition_variable>
#include "nmosapiversion.h"
#include <chrono>
#include <thread>
#include <atomic>
#include "activator.h"
#include "RestGoose.h"
#include "connection.h"
#include "activation.h"
#include "nmosserver.h"

namespace pml
{
    namespace dnssd
    {
        struct dnsInstance;
        class Browser;
        class Publisher;
    }

    namespace nmos
    {

//        class ServiceBrowserEvent;
        class CurlRegister;
        class CurlEvent;
        class EventPoster;
        class ClientPoster;

        class Device;
        class Source;
        class Flow;
        class Receiver;
        class Sender;
        class NodeThread;
        class Server;
        class NodeZCPoster;


        class IS04Server;
        class IS05Server;
        struct TransportParamsRTPSender;

        class NodeApiPrivate
        {
            public:
                enum enumResource{NR_NODES, NR_DEVICES, NR_SOURCES, NR_FLOWS, NR_SENDERS, NR_RECEIVERS, NR_SUBSCRIPTIONS};
                enum {CURL_REGISTER=1, CURL_HEARTBEAT, CURL_DELETE, CURL_QUERY};

                NodeApiPrivate();
                ~NodeApiPrivate();

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

                /** @brief Returns a pointer to the Flow with the given id if one exists or null.
                *   @param sId the uuid of the Flow
                *   @return <i>Flow*</i> pointer to the given Sender or null
                **/
                std::shared_ptr<Flow> GetFlow(const std::string& sId);

                /** @brief Returns a pointer to the Source with the given id if one exists or null.
                *   @param sId the uuid of the Source
                *   @return <i>Source*</i> pointer to the given Sender or null
                **/
                std::shared_ptr<Source> GetSource(const std::string& sId);

                /** @brief Returns a pointer to the Device with the given id if one exists or null.
                *   @param sId the uuid of the Device
                *   @return <i>Device*</i> pointer to the given Sender or null
                **/
                std::shared_ptr<Device> GetDevice(const std::string& sId);


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
                *   @param sSourceIp IP address from which RTP packets will be sent (IP address of interface bound to this output)
                *   @param sDestinationIp IP address to which RTP packets will be sent. Can be a multicast address
                *   @param sSDP the transport file that the sender should serve. This should be created by the main program logic from the staged transport parameters and associated flow etc. If empty then the Sender will create it's own from the connection parameters
                *   @note sSourceIp and sDestinationIp should be set to be the same as the Sender's staged parameters unless they are set to auto. In which case the main program logic should choose them.
                *   @note EventPoster::PatchSender is called on the request of the Sender being patched whatever the activation mode is. This function should be called by the main thread to allow the server to respond.
                **/
                void SenderPatchAllowed(unsigned short nPort, bool bOk, const std::string& sId, const std::string& sSourceIp, const std::string& sDestinationIp, const std::string& sSDP="");

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

                unsigned short GetDiscoveryPort() const;


                Json::Value JsonConnectionVersions() const;


                const std::string GetRegistrationNode() const;
                ApiVersion GetRegistrationVersion() const;


                /** @brief Sets the regularity that heartbeat messages are sent to a registry
                *   @param nMilliseconds the time between heartbeats in milliseconds
                **/
                void SetHeartbeatTime(unsigned long nMilliseconds);

                /** @brief Gets the regularity that heartbeat messages are sent to a registry
                *   @return <i>unsigned long</i> the time between heartbeats in milliseconds
                **/
                const std::chrono::system_clock::time_point& GetHeartbeatTime();


                /** @brief Called be a receiver when it is activated
                **/
                void ReceiverActivated(const std::string& sId);
                void SenderActivated(const std::string& sId);


                bool AddBrowseDomain(const std::string& sDomain);
                bool RemoveBrowseDomain(const std::string& sDomain);




            protected:
                //friend class Browser;
                //friend class Server;
                friend class NodeZCPoster;
                friend class IS04Server;
                friend class IS05Server;
                friend class Activator;

                enum {REG_FAILED = 0, REG_START, REG_DEVICES, REG_SOURCES, REG_FLOWS, REG_SENDERS, REG_RECEIVERS, REG_DONE};


                pml::restgoose::response GetRoot(const query& theQuery, const postData& theData, const endpoint& theEndpoint, const userName& theUser);
                pml::restgoose::response GetNmosDiscoveryRoot(const query& theQuery, const postData& theData, const endpoint& theEndpoint, const userName& theUser);
                pml::restgoose::response GetNmosConnectionRoot(const query& theQuery, const postData& theData, const endpoint& theEndpoint, const userName& theUser);

                std::string CreateFlowSdp(const std::string& sId, const TransportParamsRTPSender& tpSender, const std::set<std::string>& setInterfaces);

                void CreateSDP(std::shared_ptr<Sender> pSender);
                void Activate(bool bCommit, std::shared_ptr<IOResource> pResource);

                void Activate(bool bCommit, std::shared_ptr<Sender> pSender);
                void CommitActivation(std::shared_ptr<Sender> pSender);
                bool Stage(const connectionSender<activationResponse>& conRequest, std::shared_ptr<Sender> pSender);

                void Activate(bool bCommit, std::shared_ptr<Receiver> pReceiver);
                void CommitActivation(std::shared_ptr<Receiver> pReceiver);
                bool Stage(const connectionReceiver<activationResponse>& conRequest, std::shared_ptr<Receiver> pReceiver);

                void SubscribeToSender(std::shared_ptr<Receiver> pReceiver, const std::string& sSenderId, const std::string& sSdp, const std::string& sInterfaceIp);



                void Run();
                bool RegisteredOperation();
                bool FindRegisterNode();
                bool HandleHeartbeatResponse(unsigned int nResponse);

                bool StartHttpServers();
                bool BrowseForRegistrationNode();
                void SignalBrowse();

                void PostRegisterStatus();
                long RegisterResource(const std::string& sType, const Json::Value& json);

                void SignalServer(unsigned short nPort, const pml::restgoose::response& resp);

                enum enumSignal{SIG_NONE=0, SIG_COMMIT=1, SIG_INSTANCE_FOUND, SIG_INSTANCE_REMOVED, SIG_BROWSE_DONE, SIG_EXIT};

                void Signal(enumSignal eSignal);
                enumSignal GetSignal() const;


                void HandleInstanceResolved(std::shared_ptr<pml::dnssd::dnsInstance> pInstance);
                void HandleInstanceRemoved(std::shared_ptr<pml::dnssd::dnsInstance> pInstance);

                bool Wait(unsigned long nMilliseconds);
                bool WaitUntil(const std::chrono::system_clock::time_point& timeout_time);

                bool FindRegistrationNode();
                bool IsRunning();
                void StopRun();
                void ModifyTxtRecords();

                int RegisterSimple();
                int UnregisterSimple();
                long RegistrationHeartbeat();
                void UpdateRegisterSimple(bool bSelf, std::list<std::shared_ptr<Resource>> lstChanges);

                struct regnode
                {
                    regnode(unsigned short np, const ApiVersion& ver) : bGood(true), nPriority(np), version(ver){}
                    bool bGood;
                    unsigned short nPriority;
                    ApiVersion version;
                };
                std::map<std::string, regnode> GetRegNodes();



            private:



                void StopHttpServers();

                bool StartmDNSPublisher();
                void StopmDNSPublisher();
                void SetmDNSTxt(bool bSecure);


                void StopRegistrationBrowsing();


                template<class T> long RegisterResources(ResourceHolder<T>& holder);
                template<class T> long ReregisterResources(ResourceHolder<T>& holder);
                template<class T> long UpdateRegisterResources(ResourceHolder<T>& holder);
        //        template<class T> bool UnregisterResources(ResourceHolder<T>& holder);






                bool StartUnregistration();

                bool UnregisterResource(const std::string& sType, const std::string& sId);

                //bool FindQueryNode();

                bool CheckNodeApiPath();

                void MarkRegNodeAsBad();

                Self m_self;
                ResourceHolder<Device> m_devices;
                ResourceHolder<Sender> m_senders;
                ResourceHolder<Receiver> m_receivers;
                ResourceHolder<Source> m_sources;
                ResourceHolder<Flow> m_flows;

                std::map<std::string, std::unique_ptr<pml::dnssd::Browser>> m_mBrowser;


                std::string m_sRegistrationNode;
                ApiVersion m_versionRegistration;
                //std::string m_sQueryNode;

                std::unique_ptr<pml::dnssd::Publisher> m_pNodeApiPublisher;

                unsigned int m_nRegistrationStatus;

                mutable std::mutex m_mutex;
                std::condition_variable m_cvBrowse; //sync between nmos thread and Browser thread
                std::condition_variable m_cvCommit; //sync between nmos thread and main thread (used to sleep until a Commit is called)

                std::unique_ptr<std::thread> m_pThread;

                std::atomic<bool> m_bRun;
                bool m_bBrowsing;
                std::shared_ptr<EventPoster> m_pPoster;
                std::shared_ptr<NodeZCPoster> m_pZCPoster;
                unsigned short m_nConnectionPort;
                unsigned short m_nDiscoveryPort;

                std::chrono::system_clock::time_point m_tpHeartbeat;

                std::string m_sDnsName;

                std::list<std::shared_ptr<pml::restgoose::Server>> m_lstServers;
                std::map<ApiVersion, std::unique_ptr<IS04Server>> m_mDiscoveryServers;
                std::map<ApiVersion, std::unique_ptr<IS05Server>> m_mConnectionServers;


                std::map<std::string, regnode> m_mRegNode;

                unsigned long m_nHeartbeatTime;

                enumSignal m_eSignal;

                Activator m_activator;
                static const std::string STR_RESOURCE[7];
        };
    };
};
