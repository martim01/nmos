#include "registry.h"
#include <map>
#include <list>
#include "registryapi.h"

class RegistryHolder;

class RegistryMemory : public Registry
{
    public:
        RegistryMemory();
        virtual ~RegistryMemory(){}


        bool DeleteResource(const std::string& sType, const std::string& sId);
        size_t Heartbeat(const std::string& sId);
        std::shared_ptr<Resource> FindNmosResource(const std::string& sType, const std::string& sId);
        bool AddResource(const std::string& sType, std::shared_ptr<Resource> pResource);
        bool ResourceUpdated(std::shared_ptr<Resource> pResource);
        bool GarbageCollection();


    private:

        std::list<std::string> GarbageCheck(RegistryApi::enumResource eResource, const std::list<std::string>& lstDevice);
        void GarbageDelete(RegistryApi::enumResource eResource, const std::list<std::string>& lstDelete);
        std::map<std::string, RegistryHolder> m_mRegistryHolder;

};
