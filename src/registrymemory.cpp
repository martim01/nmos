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


bool RegistryMemory::GarbageCollection()
{
    map<string, RegistryHolder>::iterator itNode = m_mRegistryHolder.find(RegistryApi::STR_RESOURCE[RegistryApi::NODE]);
    map<string, RegistryHolder>::iterator itDevice = m_mRegistryHolder.find(RegistryApi::STR_RESOURCE[RegistryApi::DEVICE]);

    if(itNode != m_mRegistryHolder.end() && itDevice != m_mRegistryHolder.end())
    {
        auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        size_t nNow = nanos/1000000000;

        list<string> lstDeleteNode;
        for(map<string, shared_ptr<Resource> >::const_iterator itResource = itNode->second.GetResourceBegin(); itResource != itNode->second.GetResourceEnd(); ++itResource)
        {
            if(nNow-itResource->second->GetLastHeartbeat() > 30)    //@todo should be able to set the garbage collection time
            {
                lstDeleteNode.push_back(itResource->first);
            }
        }

        list<string> lstDevice(GarbageCheck(RegistryApi::DEVICE, lstDeleteNode));

        //now delete them all
        GarbageDelete(RegistryApi::RECEIVER, GarbageCheck(RegistryApi::RECEIVER, lstDevice));
        GarbageDelete(RegistryApi::SENDER, GarbageCheck(RegistryApi::SENDER, lstDevice));
        GarbageDelete(RegistryApi::FLOW, GarbageCheck(RegistryApi::FLOW, lstDevice));
        GarbageDelete(RegistryApi::SOURCE, GarbageCheck(RegistryApi::SOURCE, lstDevice));
        GarbageDelete(RegistryApi::DEVICE, lstDevice);
        GarbageDelete(RegistryApi::NODE, lstDeleteNode);

        return (lstDeleteNode.empty() == false);
    }
    return false;
}

list<string> RegistryMemory::GarbageCheck(RegistryApi::enumResource eResource, const list<string>& lstDevice)
{
    //need to delete all sources, flows, senders and receivers of the devices to be deleted...
    list<string> lstDelete;
    map<string, RegistryHolder>::iterator itHolder = m_mRegistryHolder.find(RegistryApi::STR_RESOURCE[eResource]);
    if(itHolder != m_mRegistryHolder.end())
    {
        for(map<string, shared_ptr<Resource> >::const_iterator itResource = itHolder->second.GetResourceBegin(); itResource != itHolder->second.GetResourceEnd(); ++itResource)
        {
            for(list<string>::const_iterator itDelete = lstDevice.begin(); itDelete != lstDevice.end(); ++itDelete)
            {
                if(itResource->second->GetParentResourceId() == (*itDelete))
                {
                    lstDelete.push_back(itResource->second->GetId());
                }
            }
        }
    }
    return lstDelete;
}

void RegistryMemory::GarbageDelete(RegistryApi::enumResource eResource, const std::list<std::string>& lstDelete)
{
    map<string, RegistryHolder>::iterator itHolder = m_mRegistryHolder.find(RegistryApi::STR_RESOURCE[eResource]);
    if(itHolder != m_mRegistryHolder.end())
    {
        for(list<string>::const_iterator itDelete = lstDelete.begin(); itDelete != lstDelete.end(); ++itDelete)
        {
            Log::Get() << "Garbage Collect '" << RegistryApi::STR_RESOURCE[eResource] << "' " << (*itDelete) << endl;
            itHolder->second.RemoveResource(*itDelete);
        }
    }
}
