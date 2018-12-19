#pragma once
#include "self.h"
#include "resourceholder.h"
#include <vector>
#include <string>
#include "dlldefine.h"
#include <mutex>

class ServiceBrowser;
class ServicePublisher;
class ServiceBrowserEvent;
class CurlRegister;
class CurlEvent;
class EventPoster;

class NMOS_EXPOSE NodeApi
{
    public:

        static NodeApi& Get();

        void Init(unsigned short nApiPort, const std::string& sLabel, const std::string& sDescription);
        bool StartServices(EventPoster* pPoster);
        void StopServices();

        bool BrowseForRegistrationNode();

        bool Commit();

        bool FindRegistrationNode();

        Self& GetSelf();
        ResourceHolder& GetSources();
        ResourceHolder& GetDevices();
        ResourceHolder& GetFlows();
        ResourceHolder& GetReceivers();
        ResourceHolder& GetSenders();

        int GetJson(std::string sPath, std::string& sResponse);
        int PutJson(std::string sPath, std::string sJson, std::string& sRepsonse);

        void RegisterThreaded();
        void UnregisterThreaded();

        //These are all called in the thread and so don't need to be called by the user
        int RegisterSimple();
        int UnregisterSimple();
        long RegistrationHeartbeat();




        bool IsRunning();
        void StopRun();


        bool Query(const std::string& sQueryPath);

        enum {REG_FAILED = 0, REG_START, REG_DEVICES, REG_SOURCES, REG_FLOWS, REG_SENDERS, REG_RECEIVERS, REG_DONE};
        enum {CURL_REGISTER=1, CURL_HEARTBEAT, CURL_DELETE, CURL_QUERY};



    private:
        NodeApi();
        ~NodeApi();

        bool StartHttpServers();
        void StopHttpServers();

        bool StartmDNSServer();
        void StopmDNSServer();
        void SetmDNSTxt(bool bSecure);


        void StopRegistrationBrowser();

        void SplitString(std::vector<std::string>& vSplit, std::string str, char cSplit);

        int GetJsonNmos(std::string& sReturn);
        int GetJsonNmosNodeApi(std::string& sReturn);

        Json::Value GetJsonSources();
        Json::Value GetJsonDevices();
        Json::Value GetJsonFlows();
        Json::Value GetJsonReceivers();
        Json::Value GetJsonSenders();

        Json::Value GetJsonError(unsigned long nCode = 404, std::string sError="Resource not found");



        long RegisterResources(ResourceHolder& holder);
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


        std::vector<std::string> m_vPath;
        std::string m_sRegistrationNode;
        std::string m_sQueryNode;

        ServicePublisher* m_pNodeApiPublisher;
        ServiceBrowser* m_pRegistrationBrowser;

        CurlRegister* m_pRegisterCurl;
        unsigned int m_nRegistrationStatus;

        std::mutex m_mutex;

        bool m_bRun;
        EventPoster* m_pPoster;

        enum {BASE=0, NMOS=1, API_TYPE=2,VERSION=3,ENDPOINT=4, RESOURCE=5};
        enum {SZ_BASE=1, SZ_NMOS=2, SZ_API_TYPE=3,SZ_VERSION=4,SZ_ENDPOINT=5};


};
