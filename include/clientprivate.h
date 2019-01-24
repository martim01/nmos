#pragma once
#include "clientholder.h"
#include <mutex>
#include <condition_variable>
#include <list>
#include "json/json.h"

class ServiceBrowser;
class EventPoster;
class dnsInstance;


class ClientApiPrivate
{
    public:
        enum enumMode {MODE_P2P, MODE_REGISTRY};
        enum enumSignal {CLIENT_SIG_NONE=0, CLIENT_SIG_INSTANCE_RESOLVED, CLIENT_SIG_INSTANCE_REMOVED, CLIENT_SIG_NODE_BROWSED};

        ClientApiPrivate();
        ~ClientApiPrivate();

        void Start(int nFlags);
        void Stop();
        void ChangeInterest(int nFlags);

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

        ServiceBrowser* GetBrowser();
        void DeleteServiceBrowser();

        bool Wait(unsigned long nMilliseconds);
        bool IsRunning();
        void StopRun();
        void SetInstanceResolved(std::shared_ptr<dnsInstance> pInstance);
        void SetInstanceRemoved(std::shared_ptr<dnsInstance> pInstance);
        enumSignal GetSignal();
        void HandleInstanceResolved();
        void HandleInstanceRemoved();
        void NodeDetailsDone();
        void GetNodeDetails();
        int GetInterestFlags();

    private:
        friend class ClientPoster;
        void ConnectToQueryServer();
        void RemoveResources(const std::string& sIpAddress);
        void StoreNodeVersion(const std::string& sIpAddress, std::shared_ptr<dnsInstance> pInstance);
        bool UpdateResource(ClientHolder& holder, const Json::Value& jsData);


        enumMode m_eMode;

        ClientHolder m_nodes;
        ClientHolder m_devices;
        ClientHolder m_sources;
        ClientHolder m_flows;
        ClientHolder m_senders;
        ClientHolder m_receivers;

        ServiceBrowser* m_pBrowser;
        bool m_bRun;
        int m_nFlags;
        std::shared_ptr<dnsInstance> m_pInstance;

        std::list<std::shared_ptr<dnsInstance> > m_lstResolve;

        std::mutex m_mutex;
        std::condition_variable m_cvBrowse; //sync between nmos thread and ServiceBrowser thread
        enumSignal m_eSignal;
        unsigned short m_nCurlThreadCount;

};

