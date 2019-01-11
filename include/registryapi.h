#pragma once
#include <string>
#include "json/json.h"
#include "dlldefine.h"
#include "registryholder.h"


class ServicePublisher;
class RegistryServer;

class NMOS_EXPOSE RegistryApi
{
    public:
        ///< @brief Gets the singleton RegistryApi object
        static RegistryApi& Get();

        /** @brief Starts the mDNS-SD and HTTP servers
        *   @param nPriority the priority to assign to this Registry Node.
        *   @param nPort the http port to use to connect to the Registry Node.
        *   @param bSecure whether to use https or http for connections
        *   @return <i>bool</i> true if both servers start okay.
        **/
        bool Start(unsigned short nPriority, unsigned short nPort, bool bSecure);

        ///< @brief Stops the mDNS-SD and HTTP servers
        void Stop();

        /** @brief Attempts to find a resource of the given type and id
        *   @param sType the type of resource. This should be one of STR_RESOURCE
        *   @param sId the id of the resource
        *   @return <i>const shared_ptr<Resource></i> Pointer to the resource if found. Else 0
        **/
        const std::shared_ptr<Resource> FindResource(const std::string& sType, const std::string& sId);

        /** @brief Attempts to add or update a resource of the given type with the Json data
        *   @param sType the type of resource. This should be one of STR_RESOURCE
        *   @param jsData Json data that describes the resource. The function will parse this data and check for validity before adding the resource
        *   @return <i>unsigned short</i> 201 if the resource was added. 200 if the resource was updated. 404 for failure
        **/
        unsigned short AddUpdateResource(const std::string& sType, const Json::Value& jsData);

        /** @brief Attempts to addelete resource of the given type and id
        *   @param sType the type of resource. This should be one of STR_RESOURCE
        *   @param sId the id of the resource
        *   @return <i>bool</i> true if a resource of the given type and id was found and deleted.
        **/
        bool DeleteResource(const std::string& sType, const std::string& sId);

        /** @brief Sets the heartbeat time for a node with the given id
        *   @param sId the id of the node
        *   @return <i>size_t</i> If the node is found then the current heartbeat time. If the node is not found then 0
        **/
        size_t Heartbeat(const std::string& sId);

        static const std::string STR_RESOURCE[6];
        enum{NODE, DEVICE, SOURCE, FLOW, SENDER, RECEIVER};
    private:
        RegistryApi();
        ~RegistryApi();

        bool StartPublisher(unsigned short nPriority, unsigned short nPort, bool bSecure);
        void StopPublisher();

        bool StartServer(unsigned short nPort);
        void StopServer();

        bool AddResource(const std::string& sType, const Json::Value& jsData);

        bool AddResource(const std::string& sType, std::shared_ptr<Resource> pResource);
        bool UpdateResource(const Json::Value& jsData, std::shared_ptr<Resource> pResource);

        ServicePublisher* m_pRegistryApiPublisher;
        RegistryServer* m_pRegistryServer;

        std::map<std::string, RegistryHolder> m_mRegistryHolder;

};
