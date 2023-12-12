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
#include "response.h"
#include <thread>
#include <functional>
#include "clientapiposter.h"
#include <optional>
#include "clientemum.h"


class WebSocketClient;
namespace pml
{
    namespace dnssd
    {
        class Browser;
        struct dnsInstance;
    };
    namespace restgoose
    {
        class WebSocketClient;
    }
    namespace nmos
    {

        class EventPoster;
        class ClientApiPoster;
        class ClientZCPoster;
        class ZCPoster;
                class activationRequest;
        class activationResponse;
        template<typename T> class ConnectionSender;
        template<typename T> class ConnectionReceiver;
        class Constraints;

        class ClientApiImpl
        {
            public:
                enum enumMode {MODE_P2P=0, MODE_REGISTRY};
                enum enumSignal {CLIENT_SIG_NONE=0, CLIENT_SIG_INSTANCE_RESOLVED, CLIENT_SIG_INSTANCE_REMOVED, CLIENT_SIG_NODE_BROWSED, CLIENT_SIG_HTTP_DONE, CLIENT_SIG_BROWSE_DONE, THREAD_EXIT};

                enum flagResource {NODES=0, DEVICES=1, SOURCES=2, FLOWS=3, SENDERS=4, RECEIVERS=5};
                static const std::array<std::string, 7> STR_CONNECTION;

                ClientApiImpl();
                ~ClientApiImpl();

                void SetPoster(std::shared_ptr<ClientApiPoster> pPoster);
                std::shared_ptr<ClientApiPoster> GetPoster();

                void Start();
                void Stop();

                enumMode GetMode();

                resourcechanges<Self> AddNode(const std::string& sIpAddress, const Json::Value& jsData);
                resourcechanges<Device> AddDevices(const std::string& sIpAddress, const Json::Value& jsData);
                resourcechanges<Source> AddSources(const std::string& sIpAddress, const Json::Value& jsData);
                resourcechanges<Flow> AddFlows(const std::string& sIpAddress, const Json::Value& jsData);
                resourcechanges<Sender> AddSenders(const std::string& sIpAddress, const Json::Value& jsData);
                resourcechanges<Receiver> AddReceivers(const std::string& sIpAddress, const Json::Value& jsData);

                void StoreDevices(const std::string& sIpAddress);
                void StoreSources(const std::string& sIpAddress);
                void StoreFlows(const std::string& sIpAddress);
                void StoreSenders(const std::string& sIpAddress);
                void StoreReceivers(const std::string& sIpAddress);

                resourcechanges<Device> RemoveStaleDevices();
                resourcechanges<Source> RemoveStaleSources();
                resourcechanges<Flow> RemoveStaleFlows();
                resourcechanges<Sender> RemoveStaleSenders();
                resourcechanges<Receiver> RemoveStaleReceivers();

                template<class T> bool RunQuery(resourcechanges<T>& changed, int nResourceType);


                static const std::array<std::string,6> STR_RESOURCE;
                enum enumResource{NODE, DEVICE, SOURCE, FLOW, SENDER, RECEIVER};

                void Signal(enumSignal eSignal);


                void DeleteServiceBrowser();

                bool Wait(unsigned long nMilliseconds);
                bool IsRunning();
                void StopRun();
                void SetInstanceResolved(std::shared_ptr<pml::dnssd::dnsInstance> pInstance);
                void SetInstanceRemoved(std::shared_ptr<pml::dnssd::dnsInstance> pInstance);
                void SetAllForNow(const std::string& sService);
                void SetHttpRequestDone(const pml::restgoose::clientResponse& resp, unsigned long nType, const std::string& sResourceId);
                enumSignal GetSignal();
                void HandleInstanceResolved();
                void HandleInstanceRemoved();
                void HandleHttpRequestDone();
                void HandleBrowseDone();
                void NodeDetailsDone();
                void GetNodeDetails();


                bool Subscribe(const std::string& sSenderId, const std::string& sReceiverId);
                bool Unsubscribe(const std::string& sReceiverId);

                std::pair<pml::restgoose::clientResponse, std::optional<connectionSender<activationResponse>>> RequestSenderStaged(const std::string& sSenderId, bool bAsync);
                std::pair<pml::restgoose::clientResponse, std::optional<connectionSender<activationResponse>>> RequestSenderActive(const std::string& sSenderId, bool bAsync);
                std::pair<pml::restgoose::clientResponse, std::optional<std::string>> RequestSenderTransportFile(const std::string& sSenderId, bool bAsync);
                std::pair<pml::restgoose::clientResponse, std::vector<Constraints>> RequestSenderConstraints(const std::string& sSenderId, bool bAsync);

                std::pair<pml::restgoose::clientResponse, std::optional<connectionReceiver<activationResponse>>>  RequestReceiverStaged(const std::string& sReceiverId, bool bAsync);
                std::pair<pml::restgoose::clientResponse, std::optional<connectionReceiver<activationResponse>>>  RequestReceiverActive(const std::string& sReceiverId, bool bAsync);
                std::pair<pml::restgoose::clientResponse, std::vector<Constraints>> RequestReceiverConstraints(const std::string& sReceiverId, bool bAsync);

                std::pair<pml::restgoose::clientResponse, std::optional<connectionSender<activationResponse>>> PatchSenderStaged(const std::string& sSenderId, const connectionSender<activationRequest>& aConnection, bool bAsync);
                std::pair<pml::restgoose::clientResponse, std::optional<connectionReceiver<activationResponse>>> PatchReceiverStaged(const std::string& sReceiverId, const connectionReceiver<activationRequest>& aConnection, bool bAsync);

                bool Connect(const std::string& sSenderId, const std::string& sReceiverId);
                bool Disconnect(const std::string& sReceiverId);

                const std::map<std::string, std::shared_ptr<Self> >& GetNodes();
                std::shared_ptr<const Self> FindNode(const std::string& sUid);

                const std::map<std::string, std::shared_ptr<Device> >& GetDevices();
                std::shared_ptr<const Device> FindDevice(const std::string& sUid);

                const std::map<std::string, std::shared_ptr<Source> >& GetSources();
                std::shared_ptr<const Source> FindSource(const std::string& sUid);

                const std::map<std::string, std::shared_ptr<Flow> >& GetFlows();
                std::shared_ptr<const Flow> FindFlow(const std::string& sUid);

                const std::map<std::string, std::shared_ptr<Sender> >& GetSenders();
                std::shared_ptr<const Sender> FindSender(const std::string& sUid);

                const std::map<std::string, std::shared_ptr<Receiver> >& GetReceivers();
                std::shared_ptr<const Receiver>FindReceiver(const std::string& sUid);


                void HandleConnect(const std::string& sSenderId, const std::string& sReceiverId, bool bSuccess, const std::string& sResponse);



                bool AddQuerySubscription(int nResource, const std::string& sQuery, unsigned long nUpdateRate);
                bool RemoveQuerySubscription(const std::string& sSubscriptionId);

                void AddBrowseServices();
                void StartBrowsers();

                bool AddBrowseDomain(const std::string& sDomain);
                bool RemoveBrowseDomain(const std::string& sDomain);

                bool WebsocketConnected(const endpoint& theEndpoint);
                bool WebsocketMessage(const endpoint& theEndpoint, const std::string& sMessage);

                const ApiVersion& GetVersion() const { return m_version;}

            private:


                resourcechanges<Device> AddDevice(const std::string& sIpAddress, const Json::Value& jsData);
                resourcechanges<Source> AddSource(const std::string& sIpAddress, const Json::Value& jsData);
                resourcechanges<Flow> AddFlow(const std::string& sIpAddress, const Json::Value& jsData);
                resourcechanges<Sender> AddSender(const std::string& sIpAddress, const Json::Value& jsData);
                resourcechanges<Receiver> AddReceiver(const std::string& sIpAddress, const Json::Value& jsData);


                void ConnectToQueryServer();
                void RemoveResources(const std::string& sIpAddress);
                void StoreNodeVersion(const std::string& sIpAddress, std::shared_ptr<pml::dnssd::dnsInstance> pInstance);

                std::string GetTargetUrl(std::shared_ptr<Receiver> pReceiver, ApiVersion& version);
                std::string GetConnectionUrlSingle(std::shared_ptr<Resource> pResource, const std::string& sDirection, const std::string& sEndpoint);
                std::string GetConnectionUrl(std::shared_ptr<Resource> pResource);

                std::shared_ptr<Sender> GetSender(const std::string& sSenderId);
                std::shared_ptr<Receiver> GetReceiver(const std::string& sSenderId);

                std::pair<pml::restgoose::clientResponse, std::optional<connectionSender<activationResponse>>> RequestSenderConnection(const std::string& sSenderId, enumConnection eType,bool bAsync);
                std::pair<pml::restgoose::clientResponse, std::optional<connectionReceiver<activationResponse>>> RequestReceiverConnection(const std::string& sSenderId, enumConnection eType, bool bAsync);

                pml::restgoose::clientResponse RequestSender(const std::string& sSenderId, enumConnection eType, bool bAsync = true, bool bJson=true);
                pml::restgoose::clientResponse RequestReceiver(const std::string& sReceiverId, enumConnection eType, bool bAsync = true, bool bJson=true);

                void HandleHttpRequestDoneTarget();
                void HandleHttpRequestDonePatchSender();
                void HandleHttpRequestDonePatchReceiver();
                void HandleHttpRequestDoneGetSenderStaged();
                void HandleHttpRequestDoneGetSenderActive();
                void HandleHttpRequestDoneGetSenderConstraints();
                void HandlHttpRequestDoneGetSenderTransportFile();
                void HandleHttpRequestDoneGetReceiverStaged();
                void HandleHttpRequestDoneGetReceiverActive();
                void HandleHttpRequestDoneGetReceiverConstraints();

                bool AddQuerySubscriptionRegistry(int nResource, const std::string& sQuery, unsigned long nUpdateRate);
                bool RemoveQuerySubscriptionRegistry(const std::string& sSubscriptionId);

                bool AddQuerySubscriptionP2P(int nResource, const std::string& sQuery);
                bool RemoveQuerySubscriptionP2P(const std::string& sSubscriptionId);



                template<class T> bool RunQuery(std::list<std::shared_ptr<const T> >& lstCheck, int nResource);
                template<class T> bool RunQuery(std::list<std::shared_ptr<const T> >& lstCheck, const std::string& sQuery);

                bool MeetsQuery(const std::string& sQuery, std::shared_ptr<const Resource> pResource);

                void SubscribeToQueryServer();


                std::shared_ptr<const Flow> CreateFlow(const Json::Value& jsData, const std::string& sIpAddress);
                std::shared_ptr<const Flow> CreateFlowAudio(const Json::Value& jsData, const std::string& sIpAddress);
                std::shared_ptr<const Flow> CreateFlowAudioCoded(const Json::Value& jsData, const std::string& sIpAddress);
                std::shared_ptr<const Flow> CreateFlowAudioRaw(const Json::Value& jsData, const std::string& sIpAddress);
                std::shared_ptr<const Flow> CreateFlowVideo(const Json::Value& jsData, const std::string& sIpAddress);
                std::shared_ptr<const Flow> CreateFlowVideoRaw(const Json::Value& jsData, const std::string& sIpAddress);
                std::shared_ptr<const Flow> CreateFlowVideoCoded(const Json::Value& jsData, const std::string& sIpAddress);
                std::shared_ptr<const Flow> CreateFlowData(const Json::Value& jsData, const std::string& sIpAddress);
                std::shared_ptr<const Flow> CreateFlowMux(const Json::Value& jsData, const std::string& sIpAddress);


                void HandleGrainEvent(const std::string& sSourceId, const Json::Value& jsGrain);

                void GrainUpdateNode(const std::string& sSourceId, const Json::Value& jsData);
                void GrainUpdateDevice(const std::string& sSourceId, const Json::Value& jsData);
                void GrainUpdateSource(const std::string& sSourceId, const Json::Value& jsData);
                void GrainUpdateFlow(const std::string& sSourceId, const Json::Value& jsData);
                void GrainUpdateSender(const std::string& sSourceId, const Json::Value& jsData);
                void GrainUpdateReceiver(const std::string& sSourceId, const Json::Value& jsData);

                enum class enumGrain {ADD, UPDATE, DELETE, UNKNOWN};


                enumGrain WorkoutAction(const Json::Value& jsData);

                ApiVersion m_version;
                enumMode m_eMode;

                ClientHolder<Self> m_nodes;
                ClientHolder<Device> m_devices;
                ClientHolder<Source> m_sources;
                ClientHolder<Flow> m_flows;
                ClientHolder<Sender> m_senders;
                ClientHolder<Receiver> m_receivers;


                std::atomic<bool> m_bRun;

                std::shared_ptr<pml::dnssd::dnsInstance> m_pInstance;
                std::string m_sService;
                std::list<std::shared_ptr<pml::dnssd::dnsInstance> > m_lstResolve;

                std::mutex m_mutex;
                std::condition_variable m_cvBrowse; //sync between nmos thread and ServiceBrowser thread
                enumSignal m_eSignal;
                

                std::shared_ptr<ClientApiPoster> m_pPoster;

                pml::restgoose::clientResponse m_asyncResponse;
                unsigned long m_nHttpRequestType;
                std::string m_sHttpRequestResourceId;

                std::shared_ptr<ClientZCPoster> m_pClientZCPoster;

                std::multimap<unsigned short, std::shared_ptr<pml::dnssd::dnsInstance> > m_mmQueryNodes;

                std::map<std::string, std::unique_ptr<pml::dnssd::Browser>> m_mBrowser;

                std::unique_ptr<std::thread> m_pThread;
                bool m_bStarted;
                bool m_bDoneQueryBrowse;
                struct query
                {
                    query() : nResource(0), nRefreshRate(0){}
                    std::string sId;
                    std::string sHref;
                    int nResource;
                    std::string sQuery;
                    unsigned long nRefreshRate;
                    Json::Value jsSubscription;
                };

                endpoint GetQueryServer(const ApiVersion& version=ApiVersion(1,2));
                bool QueryQueryServer(const endpoint& theEndpoint, query& theQuery);
                void HandleQuerySubscriptionResponse(unsigned short nCode, const ClientApiImpl::query& theQuery);
                void HandleSuccessfulQuerySubscription(const ClientApiImpl::query& theQuery);

                void GetSubnetMasks();

                std::multimap<int, query> m_mmQuery;


                std::unique_ptr<pml::restgoose::WebSocketClient> m_pWebSocket;

                std::map<std::string, std::function<void(const std::string&, const Json::Value&)>> m_mGrainUpdate;

                std::map<ipAddress, unsigned long> m_mSubnetMasks;
                //std::map<std::string, ClientHolder<T>&> m_mGrainRemove;


        };
    };
};
