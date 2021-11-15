#pragma once
#include "clientholder.h"
#include <mutex>
#include <condition_variable>
#include <list>
#include "json/json.h"
#include <atomic>
#include "self.h"
#include "device.h"
#include "source.h"
#include "senderbase.h"
#include "flow.h"
#include "receiverbase.h"
#include "clientposter.h"

namespace pml
{
    namespace nmos
    {

        class ServiceBrowser;
        class EventPoster;
        class dnsInstance;
        class CurlRegister;
        class ClientApiPoster;

        class ClientApiImpl
        {
            public:
                enum enumMode {MODE_P2P=0, MODE_REGISTRY};
                enum enumSignal {CLIENT_SIG_NONE=0, CLIENT_SIG_INSTANCE_RESOLVED, CLIENT_SIG_INSTANCE_REMOVED, CLIENT_SIG_NODE_BROWSED, CLIENT_SIG_CURL_DONE, CLIENT_SIG_BROWSE_DONE, THREAD_EXIT};
                enum flagResource {NONE=0, NODES=1, DEVICES=2, SOURCES=4, FLOWS=8, SENDERS=16, RECEIVERS=32, ALL=63};


                ClientApiImpl();
                ~ClientApiImpl();

                void SetPoster(std::shared_ptr<ClientApiPoster> pPoster);
                std::shared_ptr<ClientApiPoster> GetPoster();

                void Start();
                void Stop();

                enumMode GetMode();

                void AddNode(std::list<std::shared_ptr<Self> >& lstAdded, std::list<std::shared_ptr<Self> >& lstUpdated, const std::string& sIpAddress, const std::string& sData);
                void AddDevices(std::list<std::shared_ptr<Device> >& lstAdded, std::list<std::shared_ptr<Device> >& lstUpdated, const std::string& sIpAddress, const std::string& sData);
                void AddSources(std::list<std::shared_ptr<Source> >& lstAdded, std::list<std::shared_ptr<Source> >& lstUpdated, const std::string& sIpAddress, const std::string& sData);
                void AddFlows(std::list<std::shared_ptr<Flow> >& lstAdded, std::list<std::shared_ptr<Flow> >& lstUpdated, const std::string& sIpAddress, const std::string& sData);
                void AddSenders(std::list<std::shared_ptr<SenderBase> >& lstAdded, std::list<std::shared_ptr<SenderBase> >& lstUpdated, const std::string& sIpAddress, const std::string& sData);
                void AddReceivers(std::list<std::shared_ptr<ReceiverBase> >& lstAdded, std::list<std::shared_ptr<ReceiverBase> >& lstUpdated, const std::string& sIpAddress, const std::string& sData);

                void StoreDevices(const std::string& sIpAddress);
                void StoreSources(const std::string& sIpAddress);
                void StoreFlows(const std::string& sIpAddress);
                void StoreSenders(const std::string& sIpAddress);
                void StoreReceivers(const std::string& sIpAddress);

                void RemoveStaleDevices(std::list<std::shared_ptr<Device> >& lstRemoved);
                void RemoveStaleSources(std::list<std::shared_ptr<Source> >& lstRemoved);
                void RemoveStaleFlows(std::list<std::shared_ptr<Flow> >& lstRemoved);
                void RemoveStaleSenders(std::list<std::shared_ptr<SenderBase> >& lstRemoved);
                void RemoveStaleReceivers(std::list<std::shared_ptr<ReceiverBase> >& lstRemoved);

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
                void SetAllForNow(const std::string& sService);
                void SetCurlDone(unsigned long nResult, const std::string& sResponse, long nType, const std::string& sResourceId);
                enumSignal GetSignal();
                void HandleInstanceResolved();
                void HandleInstanceRemoved();
                void HandleCurlDone();
                void HandleBrowseDone();
                void NodeDetailsDone();
                void GetNodeDetails();


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

                const std::map<std::string, std::shared_ptr<Self> >& GetNodes();
                std::map<std::string, std::shared_ptr<Self> >::const_iterator FindNode(const std::string& sUid);

                const std::map<std::string, std::shared_ptr<Device> >& GetDevices();
                std::map<std::string, std::shared_ptr<Device> >::const_iterator FindDevice(const std::string& sUid);

                const std::map<std::string, std::shared_ptr<Source> >& GetSources();
                std::map<std::string, std::shared_ptr<Source> >::const_iterator FindSource(const std::string& sUid);

                const std::map<std::string, std::shared_ptr<Flow> >& GetFlows();
                std::map<std::string, std::shared_ptr<Flow> >::const_iterator FindFlow(const std::string& sUid);

                const std::map<std::string, std::shared_ptr<SenderBase> >& GetSenders();
                std::map<std::string, std::shared_ptr<SenderBase> >::const_iterator FindSender(const std::string& sUid);

                const std::map<std::string, std::shared_ptr<ReceiverBase> >& GetReceivers();
                std::map<std::string, std::shared_ptr<ReceiverBase> >::const_iterator FindReceiver(const std::string& sUid);


                void HandleConnect(const std::string& sSenderId, const std::string& sReceiverId, bool bSuccess, const std::string& sResponse);

                std::shared_ptr<EventPoster> GetClientPoster();
                std::shared_ptr<ZCPoster> GetClientZCPoster();

                bool AddQuerySubscription(int nResource, const std::string& sQuery, unsigned long nUpdateRate);
                bool RemoveQuerySubscription(const std::string& sSubscriptionId);

                void AddBrowseServices();
                void StartBrowsers();

                bool AddBrowseDomain(const std::string& sDomain);
                bool RemoveBrowseDomain(const std::string& sDomain);

            private:
                friend class ClientPoster;



                void ConnectToQueryServer();
                void RemoveResources(const std::string& sIpAddress);
                void StoreNodeVersion(const std::string& sIpAddress, std::shared_ptr<dnsInstance> pInstance);

                std::string GetTargetUrl(std::shared_ptr<ReceiverBase> pReceiver, ApiVersion& version);
                std::string GetConnectionUrlSingle(std::shared_ptr<Resource> pResource, const std::string& sDirection, const std::string& sEndpoint, ApiVersion& version);

                std::shared_ptr<SenderBase> GetSender(const std::string& sSenderId);
                std::shared_ptr<ReceiverBase> GetReceiver(const std::string& sSenderId);

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


                void HandleQuerySubscriptionResponse(unsigned short nCode, const Json::Value& jsResponse);
                void HandleSuccessfulQuerySubscription(const Json::Value& jsResponse);

                template<class T> bool RunQuery(std::list<std::shared_ptr<T> >& lstCheck, int nResource);
                template<class T> bool RunQuery(std::list<std::shared_ptr<T> >& lstCheck, const std::string& sQuery);

                bool MeetsQuery(const std::string& sQuery, std::shared_ptr<Resource> pResource);

                void SubscribeToQueryServer();

                void CreateFlow(std::list<std::shared_ptr<Flow>>& lstAdded, const Json::Value& jsData, const std::string& sIpAddress);
                void CreateFlowAudio(std::list<std::shared_ptr<Flow>>& lstAdded, const Json::Value& jsData, const std::string& sIpAddress);
                void CreateFlowAudioCoded(std::list<std::shared_ptr<Flow>>& lstAdded, const Json::Value& jsData, const std::string& sIpAddress);
                void CreateFlowAudioRaw(std::list<std::shared_ptr<Flow>>& lstAdded, const Json::Value& jsData, const std::string& sIpAddress);
                void CreateFlowVideo(std::list<std::shared_ptr<Flow>>& lstAdded, const Json::Value& jsData, const std::string& sIpAddress);
                void CreateFlowVideoRaw(std::list<std::shared_ptr<Flow>>& lstAdded, const Json::Value& jsData, const std::string& sIpAddress);
                void CreateFlowVideoCoded(std::list<std::shared_ptr<Flow>>& lstAdded, const Json::Value& jsData, const std::string& sIpAddress);
                void CreateFlowData(std::list<std::shared_ptr<Flow>>& lstAdded, const Json::Value& jsData, const std::string& sIpAddress);
                void CreateFlowMux(std::list<std::shared_ptr<Flow>>& lstAdded, const Json::Value& jsData, const std::string& sIpAddress);


                enumMode m_eMode;

                ClientHolder<Self> m_nodes;
                ClientHolder<Device> m_devices;
                ClientHolder<Source> m_sources;
                ClientHolder<Flow> m_flows;
                ClientHolder<SenderBase> m_senders;
                ClientHolder<ReceiverBase> m_receivers;


                std::atomic<bool> m_bRun;

                std::shared_ptr<dnsInstance> m_pInstance;
                std::string m_sService;
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
                std::shared_ptr<ClientZCPoster> m_pClientZCPoster;
                std::unique_ptr<CurlRegister> m_pCurl;
                std::multimap<unsigned short, std::shared_ptr<dnsInstance> > m_mmQueryNodes;

                std::map<std::string, std::unique_ptr<ServiceBrowser>> m_mBrowser;

                std::unique_ptr<std::thread> m_pThread;
                bool m_bStarted;
                bool m_bDoneQueryBrowse;
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
    };
};
