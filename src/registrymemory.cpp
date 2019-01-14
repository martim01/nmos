#include "registrymemory.h"
#include "registryholder.h"
#include "registryapi.h"
#include "resource.h"
#include "log.h"

using namespace std;

RegistryMemory::RegistryMemory()
{
    for(int i = 0; i < 7; i++)
    {
        m_mRegistryHolder.insert(make_pair(RegistryApi::STR_RESOURCE[i], RegistryHolder()));
    }
}



bool RegistryMemory::DeleteResource(const std::string& sType, const std::string& sId)
{
    map<string, RegistryHolder>::iterator itHolder = m_mRegistryHolder.find(sType);
    if(itHolder != m_mRegistryHolder.end())
    {
        return itHolder->second.RemoveResource(sId);
    }
    return false;
}

size_t RegistryMemory::Heartbeat(const std::string& sId)
{
    map<string, RegistryHolder>::iterator itHolder = m_mRegistryHolder.find("node");
    if(itHolder != m_mRegistryHolder.end())
    {
        map<string, shared_ptr<Resource> >::iterator itResource = itHolder->second.GetResource(sId);
        if(itResource != itHolder->second.GetResourceEnd())
        {
            itResource->second->SetHeartbeat();
            return itResource->second->GetLastHeartbeat();
        }
    }
    return 0;
}

std::shared_ptr<Resource> RegistryMemory::FindResource(const std::string& sType, const std::string& sId)
{
    map<string, RegistryHolder>::iterator itHolder = m_mRegistryHolder.find(sType);
    if(itHolder != m_mRegistryHolder.end())
    {
        map<string, shared_ptr<Resource> >::iterator itResource = itHolder->second.GetResource(sId);
        if(itResource != itHolder->second.GetResourceEnd())
        {
            return itResource->second;
        }
    }
    return shared_ptr<Resource>(0);
}

bool RegistryMemory::AddResource(const std::string& sType, shared_ptr<Resource> pResource)
{
    map<string, RegistryHolder>::iterator itHolder = m_mRegistryHolder.find(sType);
    if(itHolder != m_mRegistryHolder.end())
    {
        if(itHolder->second.AddResource(pResource))
        {
            Log::Get() << "Resource of type '" << sType << "' " << pResource->GetId() << " added to registry." << endl;
            return true;
        }
    }
    Log::Get(Log::ERROR) << "Failed to add resource of type '" << sType << "' " << pResource->GetId() << " added to registry." << endl;
    return false;
}

bool RegistryMemory::ResourceUpdated(shared_ptr<Resource> pResource)
{
    return true;
}
