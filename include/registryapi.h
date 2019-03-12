#pragma once
#include <string>
#include "json/json.h"
#include "nmosdlldefine.h"
#include "registryholder.h"
#include <mutex>
class Registry;
class ServicePublisher;
class RegistryServer;

class NMOS_EXPOSE RegistryApi
{
    public:
        ///< @brief Gets the singleton RegistryApi object
        static RegistryApi& Get();

        /** @brief Starts the mDNS-SD and HTTP servers
        *   @param pRegistry a shared_ptr to the derivce registry object that actually stores the resource details
        *   @param nPriority the priority to assign to this Registry Node.
        *   @param nPort the http port to use to connect to the Registry Node.
        *   @param bSecure whether to use https or http for connections
        *   @return <i>bool</i> true if both servers start okay.
        **/
        bool Start(std::shared_ptr<Registry> pRegistry, unsigned short nPriority, const std::string& sInterface, unsigned short nPort, bool bSecure);

        ///< @brief Stops the mDNS-SD and HTTP servers
        void Stop();

        /** @brief Attempts to find a resource of the given type and id
        *   @param sType the type of resource. This should be one of STR_RESOURCE
        *   @param sId the id of the resource
        *   @return <i>const shared_ptr<Resource></i> Pointer to the resource if found. Else 0
        **/
        const std::shared_ptr<Resource> FindNmosResource(const std::string& sType, const std::string& sId);

        /** @brief Attempts to add or update a resource of the given type with the Json data
        *   @param sType the type of resource. This should be one of STR_RESOURCE
        *   @param jsData Json data that describes the resource. The function will parse this data and check for validity before adding the resource
        *   @return <i>unsigned short</i> 201 if the resource was added. 200 if the resource was updated. 404 for failure
        **/
        unsigned short AddUpdateResource(const std::string& sType, const Json::Value& jsData, std::string& sError);

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

        /** @brief Checks whether all the registy services are running
        *   @return <i>bool</i>
        **/
        bool Running();

        ///< @brief Checks all nodes to see if they are still heartbeating and removes those that aren't
        void GarbageCollection();

        static const std::string STR_RESOURCE[6];
        enum enumResource{NODE, DEVICE, SOURCE, FLOW, SENDER, RECEIVER};
    private:
        RegistryApi();
        ~RegistryApi();

        bool StartPublisher(unsigned short nPriority, const std::string& sInterface, unsigned short nPort, bool bSecure);
        void StopPublisher();

        bool StartServer(unsigned short nPort);
        void StopServer();

        unsigned short AddResource(const std::string& sType, const Json::Value& jsData, std::string& sError);

        bool AddResource(const std::string& sType, std::shared_ptr<Resource> pResource);
        bool UpdateNmosResource(const Json::Value& jsData, std::shared_ptr<Resource> pResource, std::string& sError);

        ServicePublisher* m_pRegistryApiPublisher;
        RegistryServer* m_pRegistryServer;

        std::shared_ptr<Registry> m_pRegistry;

        std::mutex m_mutex;
        bool m_bRunning;

};
