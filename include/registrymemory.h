#include "registry.h"
#include <map>

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

    private:
        std::map<std::string, RegistryHolder> m_mRegistryHolder;

};
