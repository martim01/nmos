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

class ClientApiPrivate
{
    public:
        enum enumMode {MODE_P2P, MODE_REGISTRY};
        enum enumSignal {CLIENT_SIG_NONE=0, CLIENT_SIG_INSTANCE_RESOLVED, CLIENT_SIG_INSTANCE_REMOVED, CLIENT_SIG_NODE_BROWSED, CLIENT_SIG_CURL_DONE};

        ClientApiPrivate();
        ~ClientApiPrivate();

        void SetPoster(std::shared_ptr<ClientApiPoster> pPoster);

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

        void StoreDevices(const std::string& sIpAddress);
        void StoreSources(const std::string& sIpAddress);
        void StoreFlows(const std::string& sIpAddress);
        void StoreSenders(const std::string& sIpAddress);
        void StoreReceivers(const std::string& sIpAddress);

        void RemoveStaleDevices();
        void RemoveStaleSources();
        void RemoveStaleFlows();
        void RemoveStaleSenders();
        void RemoveStaleReceivers();


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

        enumMode m_eMode;

        ClientHolder<Self> m_nodes;
        ClientHolder<Device> m_devices;
        ClientHolder<Source> m_sources;
        ClientHolder<Flow> m_flows;
        ClientHolder<Sender> m_senders;
        ClientHolder<Receiver> m_receivers;

        ServiceBrowser* m_pBrowser;
        CurlRegister* m_pCurl;
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
};

