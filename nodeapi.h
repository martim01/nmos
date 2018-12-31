#pragma once
#include "self.h"
#include "resourceholder.h"
#include <vector>
#include <string>
#include "dlldefine.h"
#include <mutex>
#include <condition_variable>

class ServiceBrowser;
class ServicePublisher;
class ServiceBrowserEvent;
class CurlRegister;
class CurlEvent;
class EventPoster;

class Device;
class Source;
class Flow;
class Receiver;
class Sender;
class NmosThread;
class MicroServer;


class NMOS_EXPOSE NodeApi
{
    public:
        enum enumResource{NR_NODES, NR_DEVICES, NR_SOURCES, NR_FLOWS, NR_SENDERS, NR_RECEIVERS, NR_SUBSCRIPTIONS};
        enum {CURL_REGISTER=1, CURL_HEARTBEAT, CURL_DELETE, CURL_QUERY};

        /** @brief returns the singleton
        *   @return <i>static NodeApi</i>
        **/
        static NodeApi& Get();

        /** @brief sets the initial data the Node needs to run
        *   @param nDiscoveryPort the TCP port to run the discovery web serv on
        *   @param nConnectionPort the TCP port to run the connection port on
        *   @param sLabel the name to give this node
        *   @param sDescription a description aof this node
        **/
        void Init(unsigned short nDiscoveryPort, unsigned short nConnectionPort, const std::string& sLabel, const std::string& sDescription);

        /** @brief Launch the thread that starts the web servers and dns publisher and browser
        *   @param pPoster pointer to an optional EventPoster class that can be used to update the main thread with events.
        *   @note NodeApi takes owenership of the EventPoster and will delete it on destruction
        *   @return <i>bool</i> true on succesfully starting the thread
        **/
        bool StartServices(std::shared_ptr<EventPoster> pPoster);

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
        bool AddDevice(Device* pResource);


        /** @brief Adds a source to the model.
        *   @param pResource a pointer to the source
        *   @return <i>bool</i> true if succesfully added. This will fail if a source with the same id already exists or there is no device with the device_id of the source
        *   @note NodeApi takes ownership of the resource and will delete it on exit.
        *   @note You need to call Commit in order to publish the resource to the outside world
        **/
        bool AddSource(Source* pResource);

        /** @brief Adds a flow to the model.
        *   @param pResource a pointer to the flow
        *   @return <i>bool</i> true if succesfully added. This will fail if a flow with the same id already exists or there is no device with the device_id of the flow or no source with the source_id of the flow
        *   @note NodeApi takes ownership of the resource and will delete it on exit.
        *   @note You need to call Commit in order to publish the resource to the outside world
        **/
        bool AddFlow(Flow* pResource);

        /** @brief Adds a source to the model.
        *   @param pResource a pointer to the receiver
        *   @return <i>bool</i> true if succesfully added. This will fail if a receiver with the same id already exists or there is no flow with the flow_id of the receiver
        *   @note NodeApi takes ownership of the resource and will delete it on exit.
        *   @note You need to call Commit in order to publish the resource to the outside world
        **/
        bool AddReceiver(Receiver* pResource);

        /** @brief Adds a sender to the model.
        *   @param pResource a pointer to the sender
        *   @return <i>bool</i> true if succesfully added. This will fail if a sender with the same id already exists or there is no flow with the flow_id of the sender
        *   @note NodeApi takes ownership of the resource and will delete it on exit.
        *   @note You need to call Commit in order to publish the resource to the outside world
        **/
        bool AddSender(Sender* pResource);

        /** @brief Returns a reference to the Node /self
        *   @return <i>Self&</i>
        **/
        Self& GetSelf();

        /** @brief Returns a const reference to the ResourceHolder that contains all the sources
        *   @return </i>const ResourceHolder&</i>
        **/
        const ResourceHolder& GetSources();

        /** @brief Returns a const reference to the ResourceHolder that contains all the devices
        *   @return </i>const ResourceHolder&</i>
        **/
        const ResourceHolder& GetDevices();

        /** @brief Returns a const reference to the ResourceHolder that contains all the flows
        *   @return </i>const ResourceHolder&</i>
        **/
        const ResourceHolder& GetFlows();

        /** @brief Returns a const reference to the ResourceHolder that contains all the receivers
        *   @return </i>const ResourceHolder&</i>
        **/
        const ResourceHolder& GetReceivers();

        /** @brief Returns a const reference to the ResourceHolder that contains all the senders
        *   @return </i>const ResourceHolder&</i>
        **/
        const ResourceHolder& GetSenders();


        /** @brief Returns a pointer to the receiver with the given id if one exists or null.
        *   @param sId the uuid of the receiver
        *   @return <i>Receiver*</i> pointer to the given receiver or null
        **/
        Receiver* GetReceiver(const std::string& sId);

        /** @brief Returns a pointer to the Sender with the given id if one exists or null.
        *   @param sId the uuid of the Sender
        *   @return <i>Sender*</i> pointer to the given Sender or null
        **/
        Sender* GetSender(const std::string& sId);

        /** @brief Signal a paused HttpServer that it may continue and reply to the question asked. Should be called by the main thread on handling a server blocking eventblocker event
        *   @param nPort the port of the server
        *   @param nCode the response code to return
        **/
        void SignalServer(unsigned short nPort, unsigned char nCode);


        /** @brief Returns a const reference to the ResourceHolder that contains all the resources returned from a query
        *   @return </i>const ResourceHolder&</i>
        **/
        const ResourceHolder& GetQueryResults();


        /** @brief Attempts to query a query server. The query is performed in a separate thread and EventPoster::CurlDone is called when the query returns
        *   @param eResource the type of resource to query for
        *   @param sQuery the query to perform
        *   @return <i>bool</i> Returns true if a query server has been found
        **/
        bool Query(enumResource eResource, const std::string& sQuery);


        /** @brief returns the port number used for the connection api
        *   @return <i>unsigned short</i>
        **/
        unsigned short GetConnectionPort() const;



        /** @brief Activates the stage parameters of the Sender with the given Id and updates any IS-O4 version information
        *   @param sId the uuid of the Sender
        *   @return <i>bool</i> true if the Sender was found
        **/
        bool ActivateSender(const std::string& sId);

        /** @brief Activates the stage parameters of the Receiver with the given Id and updates any IS-04 version information
        *   @param sId the uuid of the Receiver
        *   @return <i>bool</i> true if the Receiver was found
        **/
        bool ActivateReceiver(const std::string& sId);


    protected:
        friend class NmosThread;
        friend class ServiceBrowser;
        friend class MicroServer;

        enum {REG_FAILED = 0, REG_START, REG_DEVICES, REG_SOURCES, REG_FLOWS, REG_SENDERS, REG_RECEIVERS, REG_DONE};

        bool StartHttpServers();
        bool BrowseForRegistrationNode();
        void SignalBrowse();


        enum enumSignal{SIG_NONE=0, SIG_COMMIT=1};

        void Signal(enumSignal eSignal);
        enumSignal GetSignal() const;


        bool Wait(unsigned long nMilliseconds);

        bool FindRegistrationNode();
        bool IsRunning();
        void StopRun();
        void ModifyTxtRecords();

        int RegisterSimple();
        int UnregisterSimple();
        long RegistrationHeartbeat();
        int UpdateRegisterSimple();



    private:
        NodeApi();
        ~NodeApi();


        void StopHttpServers();

        bool StartmDNSServer();
        void StopmDNSServer();
        void SetmDNSTxt(bool bSecure);


        void StopRegistrationBrowser();







        long RegisterResources(ResourceHolder& holder);
        long ReregisterResources(ResourceHolder& holder);
        long RegisterResource(const std::string& sType, const Json::Value& json);

        bool StartUnregistration();
        void UnregisterResources(ResourceHolder& holder);
        bool UnregisterResource(const std::string& sType, const std::string& sId);

        bool FindQueryNode();

        bool CheckNodeApiPath();


        Self m_self;
        ResourceHolder m_devices;
        ResourceHolder m_senders;
        ResourceHolder m_receivers;
        ResourceHolder m_sources;
        ResourceHolder m_flows;

        ResourceHolder m_query;






        std::string m_sRegistrationNode;
        std::string m_sQueryNode;

        ServicePublisher* m_pNodeApiPublisher;
        ServiceBrowser* m_pRegistrationBrowser;

        CurlRegister* m_pRegisterCurl;
        unsigned int m_nRegistrationStatus;

        std::mutex m_mutex;
        std::condition_variable m_cvBrowse; //sync between nmos thread and ServiceBrowser thread
        std::condition_variable m_cvCommit; //sync between nmos thread and main thread (used to sleep until a Commit is called)


        bool m_bRun;
        std::shared_ptr<EventPoster> m_pPoster;
        unsigned short m_nConnectionPort;
        unsigned short m_nDiscoveryPort;

        std::map<unsigned short, MicroServer*> m_mServers;


        enumSignal m_eSignal;
        static const std::string STR_RESOURCE[7];
};
