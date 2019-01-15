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


        virtual bool DeleteResource(const std::string& sType, const std::string& sId);
        virtual size_t Heartbeat(const std::string& sId);
        virtual std::shared_ptr<Resource> FindResource(const std::string& sType, const std::string& sId);
        virtual bool AddResource(const std::string& sType, std::shared_ptr<Resource> pResource);
        virtual bool ResourceUpdated(std::shared_ptr<Resource> pResource);
        virtual bool GarbageCollection();


    private:

        std::list<std::string> GarbageCheck(RegistryApi::enumResource eResource, const std::list<std::string>& lstDevice);
        void GarbageDelete(RegistryApi::enumResource eResource, const std::list<std::string>& lstDelete);
        std::map<std::string, RegistryHolder> m_mRegistryHolder;

};
