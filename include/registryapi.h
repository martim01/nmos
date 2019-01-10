#pragma once
#include <string>
#include "json/json.h"
#include "dlldefine.h"
#include "resourceholder.h"


class ServicePublisher;
class RegistryServer;

class NMOS_EXPOSE RegistryApi
{
    public:
        static RegistryApi& Get();

        bool Start(unsigned short nPriority, unsigned short nPort, bool bSecure);
        void Stop();

        const ResourceHolder& GetResources();
        std::shared_ptr<Resource> GetResource(const std::string& sId);

        bool AddResource(const std::string& sType, const Json::Value& jsData);
        bool UpdateResource(const Json::Value& jsData, std::shared_ptr<Resource> pResource);
        void DeleteResource(const std::string& sId);

    private:
        RegistryApi();
        ~RegistryApi();

        bool StartPublisher(unsigned short nPriority, unsigned short nPort, bool bSecure);
        void StopPublisher();

        bool StartServer(unsigned short nPort);
        void StopServer();

        ServicePublisher* m_pRegistryApiPublisher;
        RegistryServer* m_pRegistryServer;

        ResourceHolder m_resources;
};
