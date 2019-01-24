#pragma once
#include "nmosdlldefine.h"
#include "clientholder.h"
#include <mutex>
#include <condition_variable>
#include <list>
#include "json/json.h"

class ServiceBrowser;
class EventPoster;
class dnsInstance;

static void ClientThread();

class NMOS_EXPOSE ClientApi
{
    public:
        enum enumMode {MODE_P2P, MODE_REGISTRY};
        enum enumSignal {CLIENT_SIG_NONE=0, CLIENT_SIG_INSTANCE_RESOLVED, CLIENT_SIG_INSTANCE_REMOVED, CLIENT_SIG_NODE_BROWSED};
        static ClientApi& Get();

        void Start();
        void Stop();

        enumMode GetMode();

        void AddNode(const std::string& sIpAddress, const std::string& sData);
        void AddDevices(const std::string& sIpAddress, const std::string& sData);
        void AddSources(const std::string& sIpAddress, const std::string& sData);
        void AddFlows(const std::string& sIpAddress, const std::string& sData);
        void AddSenders(const std::string& sIpAddress, const std::string& sData);
        void AddReceivers(const std::string& sIpAddress, const std::string& sData);

        static const std::string STR_RESOURCE[6];
        enum enumResource{NODE, DEVICE, SOURCE, FLOW, SENDER, RECEIVER};

        void Signal(enumSignal eSignal);
    private:

        friend void ClientThread();

        friend class ClientPoster;

        bool Wait(unsigned long nMilliseconds);
        bool IsRunning();
        void StopRun();

        enumSignal GetSignal();

        void SetInstanceResolved(std::shared_ptr<dnsInstance> pInstance);
        void SetInstanceRemoved(std::shared_ptr<dnsInstance> pInstance);

        void HandleInstanceResolved();
        void HandleInstanceRemoved();

        void ConnectToQueryServer();

        void GetNodeDetails();
        void RemoveResources(const std::string& sIpAddress);

        void NodeDetailsDone();
        void StoreNodeVersion(const std::string& sIpAddress, std::shared_ptr<dnsInstance> pInstance);

        bool UpdateResource(ClientHolder& holder, const Json::Value& jsData);

        ClientApi();
        ~ClientApi();

        enumMode m_eMode;

        ClientHolder m_nodes;
        ClientHolder m_devices;
        ClientHolder m_sources;
        ClientHolder m_flows;
        ClientHolder m_senders;
        ClientHolder m_receivers;

        ServiceBrowser* m_pBrowser;
        bool m_bRun;
        std::shared_ptr<dnsInstance> m_pInstance;

        std::list<std::shared_ptr<dnsInstance> > m_lstResolve;

        std::mutex m_mutex;
        std::condition_variable m_cvBrowse; //sync between nmos thread and ServiceBrowser thread
        enumSignal m_eSignal;
        unsigned short m_nCurlThreadCount;

};
