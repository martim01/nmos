#pragma once
#include "self.h"
#include "resourceholder.h"
#include <vector>
#include <string>
#include "dlldefine.h"

class ServiceBrowser;
class ServicePublisher;
class MicroServer;
class ServiceBrowserEvent;
class CurlRegister;
class CurlEvent;

class NMOS_EXPOSE NodeApi
{
    public:

        static NodeApi& Get();

        void Init(std::string sHostname, std::string sUrl, std::string sLabel, std::string sDescription);

        bool StartServices(unsigned short nPort, ServiceBrowserEvent* pPoster, CurlEvent* pCurlPoster);
        void StopServices();

        bool Commit();

        Self& GetSelf();
        ResourceHolder& GetSources();
        ResourceHolder& GetDevices();
        ResourceHolder& GetFlows();
        ResourceHolder& GetReceivers();
        ResourceHolder& GetSenders();

        int GetJson(std::string sPath, std::string& sResponse);
        int PutJson(std::string sPath, std::string sJson, std::string& sRepsonse);

        int Register();
        bool RegistrationHeartbeat();

        enum {REG_FAILED = 0, REG_START, REG_DEVICES, REG_SOURCES, REG_FLOWS, REG_SENDERS, REG_RECEIVERS, REG_DONE};

        int GetRegistrationStatus() const
        {
            return m_nRegisterNext;
        }

    private:
        NodeApi();
        ~NodeApi();

        bool StartHttpServer(unsigned short nPort);
        void StopHttpServer();

        bool StartmDNSServer(unsigned short nPort);
        void StopmDNSServer();
        void SetmDNSTxt();

        bool BrowseForRegistrationNode(ServiceBrowserEvent* pPoster);
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


        bool StartRegistration();
        void RegisterResources(ResourceHolder& holder, ResourceHolder& next);

        bool RegisterSelf();
        bool RegisterDevice(const std::string& sId);
        bool RegisterSender(const std::string& sId);
        bool RegisterReceiver(const std::string& sId);
        bool RegisterSource(const std::string& sId);
        bool RegisterFlow(const std::string& sId);

        bool RegisterResource(const std::string& sType, const Json::Value& json);

        Self m_self;

        ResourceHolder m_devices;
        ResourceHolder m_senders;
        ResourceHolder m_receivers;
        ResourceHolder m_sources;
        ResourceHolder m_flows;


        std::vector<std::string> m_vPath;
        std::string m_sRegistrationNode;

        ServicePublisher* m_pNodeApiPublisher;
        ServiceBrowser* m_pRegistrationBrowser;

        CurlRegister* m_pRegisterCurl;
        unsigned int m_nRegisterNext;
        std::map<std::string, Resource*>::const_iterator m_itRegisterNext;

        enum {BASE=0, NMOS=1, API_TYPE=2,VERSION=3,ENDPOINT=4, RESOURCE=5};
        enum {SZ_BASE=1, SZ_NMOS=2, SZ_API_TYPE=3,SZ_VERSION=4,SZ_ENDPOINT=5};


};
