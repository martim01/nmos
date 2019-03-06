#pragma once
#include "clientholder.h"
#include <mutex>
#include <condition_variable>
#include <list>
#include "json/json.h"

#include "self.h"
#include "device.h"
#include "source.h"
#include "sender.h"
#include "flow.h"
#include "receiver.h"
#include "clientposter.h"

class ServiceBrowser;
class EventPoster;
class dnsInstance;
class CurlRegister;
class ClientApiPoster;

class ClientApiImpl
{
    public:
        enum enumMode {MODE_P2P=0, MODE_REGISTRY};
        enum enumSignal {CLIENT_SIG_NONE=0, CLIENT_SIG_INSTANCE_RESOLVED, CLIENT_SIG_INSTANCE_REMOVED, CLIENT_SIG_NODE_BROWSED, CLIENT_SIG_CURL_DONE};
        enum flagResource {NONE=0, NODES=1, DEVICES=2, SOURCES=4, FLOWS=8, SENDERS=16, RECEIVERS=32, ALL=63};


        ClientApiImpl();
        ~ClientApiImpl();

        void SetPoster(std::shared_ptr<ClientApiPoster> pPoster);
        std::shared_ptr<ClientApiPoster> GetPoster();

        void Start(int nFlags);
        void Stop();
        void ChangeInterest(int nFlags);

        enumMode GetMode();

        void AddNode(std::list<std::shared_ptr<Self> >& lstAdded, std::list<std::shared_ptr<Self> >& lstUpdated, const std::string& sIpAddress, const std::string& sData);
        void AddDevices(std::list<std::shared_ptr<Device> >& lstAdded, std::list<std::shared_ptr<Device> >& lstUpdated, const std::string& sIpAddress, const std::string& sData);
        void AddSources(std::list<std::shared_ptr<Source> >& lstAdded, std::list<std::shared_ptr<Source> >& lstUpdated, const std::string& sIpAddress, const std::string& sData);
        void AddFlows(std::list<std::shared_ptr<Flow> >& lstAdded, std::list<std::shared_ptr<Flow> >& lstUpdated, const std::string& sIpAddress, const std::string& sData);
        void AddSenders(std::list<std::shared_ptr<Sender> >& lstAdded, std::list<std::shared_ptr<Sender> >& lstUpdated, const std::string& sIpAddress, const std::string& sData);
        void AddReceivers(std::list<std::shared_ptr<Receiver> >& lstAdded, std::list<std::shared_ptr<Receiver> >& lstUpdated, const std::string& sIpAddress, const std::string& sData);

        void StoreDevices(const std::string& sIpAddress);
        void StoreSources(const std::string& sIpAddress);
        void StoreFlows(const std::string& sIpAddress);
        void StoreSenders(const std::string& sIpAddress);
        void StoreReceivers(const std::string& sIpAddress);

        void RemoveStaleDevices(std::list<std::shared_ptr<Device> >& lstRemoved);
        void RemoveStaleSources(std::list<std::shared_ptr<Source> >& lstRemoved);
        void RemoveStaleFlows(std::list<std::shared_ptr<Flow> >& lstRemoved);
        void RemoveStaleSenders(std::list<std::shared_ptr<Sender> >& lstRemoved);
        void RemoveStaleReceivers(std::list<std::shared_ptr<Receiver> >& lstRemoved);

        template<class T> bool RunQuery(std::list<std::shared_ptr<T> >& lstAdded, std::list<std::shared_ptr<T> >& lstUpdated, std::list<std::shared_ptr<T> >& lstRemoved, int nResourceType);


        static const std::string STR_RESOURCE[6];
        enum enumResource{NODE, DEVICE, SOURCE, FLOW, SENDER, RECEIVER};

        void Signal(enumSignal eSignal);


        void DeleteServiceBrowser();

        bool Wait(unsigned long nMilliseconds);
        bool IsRunning();
        void StopRun();
        void SetInstanceResolved(std::shared_ptr<dnsInstance> pInstance);
        void SetInstanceRemoved(std::shared_ptr<dnsInstance> pInstance);
        void SetCurlDone(unsigned long nResult, const std::string& sResponse, long nType, const std::string& sResourceId);
        enumSignal GetSignal();
        void HandleInstanceResolved();
        void HandleInstanceRemoved();
        void HandleCurlDone();
        void NodeDetailsDone();
        void GetNodeDetails();
        int GetInterestFlags();


        bool Subscribe(const std::string& sSenderId, const std::string& sReceiverId);
        bool Unsubscribe(const std::string& sReceiverId);

        bool RequestSenderStaged(const std::string& sSenderId);
        bool RequestSenderActive(const std::string& sSenderId);
        bool RequestSenderTransportFile(const std::string& sSenderId);

        bool RequestReceiverStaged(const std::string& sReceiverId);
        bool RequestReceiverActive(const std::string& sReceiverId);

        bool PatchSenderStaged(const std::string& sSenderId, const connectionSender& aConnection);
        bool PatchReceiverStaged(const std::string& sReceiverId, const connectionReceiver& aConnection);

        bool Connect(const std::string& sSenderId, const std::string& sReceiverId);
        bool Disconnect(const std::string& sReceiverId);

        std::map<std::string, std::shared_ptr<Self> >::const_iterator GetNodeBegin();
        std::map<std::string, std::shared_ptr<Self> >::const_iterator GetNodeEnd();
        std::map<std::string, std::shared_ptr<Self> >::const_iterator FindNode(const std::string& sUid);

        std::map<std::string, std::shared_ptr<Device> >::const_iterator GetDeviceBegin();
        std::map<std::string, std::shared_ptr<Device> >::const_iterator GetDeviceEnd();
        std::map<std::string, std::shared_ptr<Device> >::const_iterator FindDevice(const std::string& sUid);

        std::map<std::string, std::shared_ptr<Source> >::const_iterator GetSourceBegin();
        std::map<std::string, std::shared_ptr<Source> >::const_iterator GetSourceEnd();
        std::map<std::string, std::shared_ptr<Source> >::const_iterator FindSource(const std::string& sUid);

        std::map<std::string, std::shared_ptr<Flow> >::const_iterator GetFlowBegin();
        std::map<std::string, std::shared_ptr<Flow> >::const_iterator GetFlowEnd();
        std::map<std::string, std::shared_ptr<Flow> >::const_iterator FindFlow(const std::string& sUid);

        std::map<std::string, std::shared_ptr<Sender> >::const_iterator GetSenderBegin();
        std::map<std::string, std::shared_ptr<Sender> >::const_iterator GetSenderEnd();
        std::map<std::string, std::shared_ptr<Sender> >::const_iterator FindSender(const std::string& sUid);

        std::map<std::string, std::shared_ptr<Receiver> >::const_iterator GetReceiverBegin();
        std::map<std::string, std::shared_ptr<Receiver> >::const_iterator GetReceiverEnd();
        std::map<std::string, std::shared_ptr<Receiver> >::const_iterator FindReceiver(const std::string& sUid);


        void HandleConnect(const std::string& sSenderId, const std::string& sReceiverId, bool bSuccess, const std::string& sResponse);

        std::shared_ptr<EventPoster> GetClientPoster();

        bool AddQuerySubscription(int nResource, const std::string& sQuery, unsigned long nUpdateRate);
        bool RemoveQuerySubscription(const std::string& sSubscriptionId);

    private:
        friend class ClientPoster;


        void ConnectToQueryServer();
        void RemoveResources(const std::string& sIpAddress);
        void StoreNodeVersion(const std::string& sIpAddress, std::shared_ptr<dnsInstance> pInstance);

        std::string GetTargetUrl(std::shared_ptr<Receiver> pReceiver, ApiVersion& version);
        std::string GetConnectionUrlSingle(std::shared_ptr<Resource> pResource, const std::string& sDirection, const std::string& sEndpoint, ApiVersion& version);

        std::shared_ptr<Sender> GetSender(const std::string& sSenderId);
        std::shared_ptr<Receiver> GetReceiver(const std::string& sSenderId);

        bool RequestSender(const std::string& sSenderId, ClientPoster::enumCurlType eType);
        bool RequestReceiver(const std::string& sReceiverId, ClientPoster::enumCurlType eType);

        void HandleCurlDoneTarget();
        void HandleCurlDonePatchSender();
        void HandleCurlDonePatchReceiver();
        void HandleCurlDoneGetSenderStaged();
        void HandleCurlDoneGetSenderActive();
        void HandlCurlDoneGetSenderTransportFile();
        void HandleCurlDoneGetReceiverStaged();
        void HandleCurlDoneGetReceiverActive();

        bool AddQuerySubscriptionRegistry(int nResource, const std::string& sQuery, unsigned long nUpdateRate);
        bool RemoveQuerySubscriptionRegistry(const std::string& sSubscriptionId);

        bool AddQuerySubscriptionP2P(int nResource, const std::string& sQuery);
        bool RemoveQuerySubscriptionP2P(const std::string& sSubscriptionId);


        template<class T> bool RunQuery(std::list<std::shared_ptr<T> >& lstCheck, int nResource);
        template<class T> bool RunQuery(std::list<std::shared_ptr<T> >& lstCheck, const std::string& sQuery);

        bool MeetsQuery(const std::string& sQuery, std::shared_ptr<Resource> pResource);

        enumMode m_eMode;

        ClientHolder<Self> m_nodes;
        ClientHolder<Device> m_devices;
        ClientHolder<Source> m_sources;
        ClientHolder<Flow> m_flows;
        ClientHolder<Sender> m_senders;
        ClientHolder<Receiver> m_receivers;


        bool m_bRun;
        int m_nFlags;
        std::shared_ptr<dnsInstance> m_pInstance;

        std::list<std::shared_ptr<dnsInstance> > m_lstResolve;

        std::mutex m_mutex;
        std::condition_variable m_cvBrowse; //sync between nmos thread and ServiceBrowser thread
        enumSignal m_eSignal;
        unsigned short m_nCurlThreadCount;

        std::shared_ptr<ClientApiPoster> m_pPoster;

        unsigned long m_nCurlResult;
        std::string m_sCurlResponse;
        long m_nCurlType;
        std::string m_sCurlResourceId;

        std::shared_ptr<ClientPoster> m_pClientPoster;
        std::unique_ptr<CurlRegister> m_pCurl;
        std::multimap<unsigned short, std::shared_ptr<dnsInstance> > m_mQueryNodes;

        bool m_bStarted;

        struct query
        {
            std::string sId;
            std::string sHref;

            int nResource;
            std::string sQuery;
            unsigned long nRefreshRate;
        };

        std::multimap<int, query> m_mmQuery;
};

