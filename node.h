#pragma once
#include "self.h"
#include "resourceholder.h"
#include <vector>

class ServicePublisher;
class MicroServer;

class Node
{
    public:

        static Node& Get();

        void Init(std::string sHostname, std::string sUrl, std::string sLabel, std::string sDescription);

        bool StartServices(unsigned short nPort);
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

    private:
        Node();
        ~Node();

        bool StartHttpServer(unsigned short nPort);
        void StopHttpServer();

        bool StartmDNSServer(unsigned short nPort);
        void StopmDNSServer();
        void SetmDNSTxt();

        void SplitPath(std::string str, char cSplit);

        int GetJsonNmos(std::string& sReturn);
        int GetJsonNmosNodeApi(std::string& sReturn);

        Json::Value GetJsonSources();
        Json::Value GetJsonDevices();
        Json::Value GetJsonFlows();
        Json::Value GetJsonReceivers();
        Json::Value GetJsonSenders();

        Json::Value GetJsonError(unsigned long nCode = 404, std::string sError="Resource not found");

        Self m_self;
        ResourceHolder m_sources;
        ResourceHolder m_devices;
        ResourceHolder m_flows;
        ResourceHolder m_receivers;
        ResourceHolder m_senders;
        std::vector<std::string> m_vPath;


        ServicePublisher* m_pNodeApi;

        enum {BASE=0, NMOS=1, API_TYPE=2,VERSION=3,ENDPOINT=4, RESOURCE=5};
        enum {SZ_BASE=1, SZ_NMOS=2, SZ_API_TYPE=3,SZ_VERSION=4,SZ_ENDPOINT=5};
};
