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
        bool StartServices(EventPoster* pPoster);

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




    protected:
        friend class NmosThread;
        friend class ServiceBrowser;
        friend class MicroServer;

        enum {REG_FAILED = 0, REG_START, REG_DEVICES, REG_SOURCES, REG_FLOWS, REG_SENDERS, REG_RECEIVERS, REG_DONE};

        bool StartHttpServers();
        bool BrowseForRegistrationNode();
        void SignalBrowse();
        void SignalCommit();

        bool WaitForCommit(unsigned long nMilliseconds);

        bool FindRegistrationNode();
        bool IsRunning();
        void StopRun();
        void ModifyTxtRecords();

        int RegisterSimple();
        int UnregisterSimple();
        long RegistrationHeartbeat();
        int UpdateRegisterSimple();

        int GetJson(std::string sPath, std::string& sResponse, unsigned short nPort);
        int PutJson(std::string sPath, std::string sJson, std::string& sRepsonse, unsigned short nPort);

    private:
        NodeApi();
        ~NodeApi();


        void StopHttpServers();

        bool StartmDNSServer();
        void StopmDNSServer();
        void SetmDNSTxt(bool bSecure);


        void StopRegistrationBrowser();

        void SplitString(std::vector<std::string>& vSplit, std::string str, char cSplit);

        int GetJsonNmos(std::string& sReturn, unsigned short nPort);
        int GetJsonNmosNodeApi(std::string& sReturn);
        int GetJsonNmosConnectionApi(std::string& sReturn);
        int GetJsonNmosConnectionSingleApi(std::string& sReturn);
        int GetJsonNmosConnectionBulkApi(std::string& sReturn);

        Json::Value GetJsonSources();
        Json::Value GetJsonDevices();
        Json::Value GetJsonFlows();
        Json::Value GetJsonReceivers();
        Json::Value GetJsonSenders();

        Json::Value GetJsonError(unsigned long nCode = 404, std::string sError="Resource not found");



        long RegisterResources(ResourceHolder& holder);
        long ReregisterResources(ResourceHolder& holder);
        long RegisterResource(const std::string& sType, const Json::Value& json);

        bool StartUnregistration();
        void UnregisterResources(ResourceHolder& holder);
        bool UnregisterResource(const std::string& sType, const std::string& sId);

        bool FindQueryNode();



        Self m_self;
        ResourceHolder m_devices;
        ResourceHolder m_senders;
        ResourceHolder m_receivers;
        ResourceHolder m_sources;
        ResourceHolder m_flows;

        ResourceHolder m_query;





        std::vector<std::string> m_vPath;
        std::string m_sRegistrationNode;
        std::string m_sQueryNode;

        ServicePublisher* m_pNodeApiPublisher;
        ServiceBrowser* m_pRegistrationBrowser;

        CurlRegister* m_pRegisterCurl;
        unsigned int m_nRegistrationStatus;

        std::mutex m_mutex;
        std::condition_variable m_cvBrowse;
        std::condition_variable m_cvCommit;

        bool m_bRun;
        EventPoster* m_pPoster;
        unsigned short m_nConnectionPort;

        std::map<unsigned short, MicroServer*> m_mConnectionServers;

        enum {BASE=0, NMOS=1, API_TYPE=2,VERSION=3,ENDPOINT=4, RESOURCE=5};
        enum {SZ_BASE=1, SZ_NMOS=2, SZ_API_TYPE=3,SZ_VERSION=4,SZ_ENDPOINT=5};

        enum {SZC_TYPE=5, SZC_DIRECTION=6};

        static const std::string STR_RESOURCE[7];
};
