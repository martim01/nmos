#include "clientholder.h"
#include "resource.h"
#include "log.h"
#include "self.h"
#include "device.h"
#include "source.h"
#include "senderbase.h"
#include "flow.h"
#include "receiverbase.h"

using namespace pml::nmos;

template<class T> ClientHolder<T>::ClientHolder()
{

}

template<class T> ClientHolder<T>::~ClientHolder()
{

}


template<class T> bool ClientHolder<T>::AddResource(const std::string& sIpAddres, std::shared_ptr<T> pResource)
{
    if(pResource && m_mResource.insert(make_pair(pResource->GetId(), pResource)).second)
    {
        m_mmAddressResourceId.insert(make_pair(sIpAddres, pResource->GetId()));
        m_mResourceIdAddress.insert(make_pair(pResource->GetId(), sIpAddres));
        return true;
    }
    return false;
}

template<class T> bool ClientHolder<T>::RemoveResource(std::shared_ptr<T> pResource)
{
    if(pResource)
    {
        return (RemoveResource(pResource->GetId())!=0);
    }
    return false;
}

template<class T> std::shared_ptr<T> ClientHolder<T>::RemoveResource(const std::string& sUuid)
{
    std::shared_ptr<T> pResource(nullptr);
    auto itAddress = m_mResourceIdAddress.find(sUuid);
    if(itAddress != m_mResourceIdAddress.end())
    {
        for(auto itRes = m_mmAddressResourceId.lower_bound(itAddress->second); itRes != m_mmAddressResourceId.upper_bound(itAddress->second); ++itRes)
        {
            if(itRes->second == sUuid)
            {
                m_mmAddressResourceId.erase(itRes);
                break;
            }
        }
    }
    m_mResourceIdAddress.erase(sUuid);

    auto itResource = m_mResource.find(sUuid);
    if(itResource != m_mResource.end())
    {
        pResource = itResource->second;
    }

    m_mResource.erase(sUuid);
    return pResource;
}

template<class T> void ClientHolder<T>::RemoveResources(const std::string& sIpAddres, typename std::list<std::shared_ptr<T> >& lstRemoved)
{
    std::set<std::string> setRemoved;

    for(auto itResource = m_mmAddressResourceId.lower_bound(sIpAddres); itResource != m_mmAddressResourceId.upper_bound(sIpAddres); ++itResource)
    {
        setRemoved.insert(itResource->second);
        auto itRemove = m_mResource.find(itResource->second);
        if(itRemove != m_mResource.end())
        {
            lstRemoved.push_back(itRemove->second);
        }
        m_mResource.erase(itResource->second);
        m_mResourceIdAddress.erase(itResource->second);
    }
    m_mmAddressResourceId.erase(sIpAddres);
}


template<class T> void ClientHolder<T>::RemoveAllResources()
{
    m_mmAddressResourceId.clear();
    m_mResourceIdAddress.clear();
    m_mResource.clear();
}

template<class T> bool ClientHolder<T>::ResourceExists(const std::string& sUuid) const
{
    return (m_mResource.find(sUuid) != m_mResource.end());
}

template<class T> const typename std::map<std::string, std::shared_ptr<T> >& ClientHolder<T>::GetResources() const
{
    return m_mResource;
}

template<class T> typename std::map<std::string, std::shared_ptr<T> >::const_iterator ClientHolder<T>::FindNmosResource(const std::string& sUuid) const
{
    return m_mResource.find(sUuid);
}

template<class T> typename std::shared_ptr<T> ClientHolder<T>::GetNmosResource(const std::string& sUuid)
{
    auto itResource = m_mResource.find(sUuid);
    if(itResource != m_mResource.end())
    {
        return itResource->second;
    }
    return nullptr;
}

template<class T> size_t ClientHolder<T>::GetResourceCount() const
{
    return m_mResource.size();
}

template<class T> std::shared_ptr<T> ClientHolder<T>::UpdateResource(const Json::Value& jsData)
{
    auto pResource = GetNmosResource(jsData["id"].asString());
    if(pResource)
    {
        m_setStored.erase(jsData["id"].asString());   //resource still exists so make sure we don't remove it later

        pmlLog(pml::LOG_INFO) << "NMOS: " << jsData["id"].asString() << " found already " ;
        if(pResource->UpdateFromJson(jsData))
        {
            pmlLog(pml::LOG_INFO) <<  "NMOS: " << jsData["id"].asString() << " updated " ;
        }
        else
        {
            pmlLog(pml::LOG_INFO) << "NMOS: " << "Found node but json data incorrect: " << pResource->GetJsonParseError() ;
        }
    }
    return pResource;
}



template<class T> std::string ClientHolder<T>::GetResourceIpAddress(const std::string& sUid)
{
    auto itResource = m_mResourceIdAddress.find(sUid);
    if(itResource != m_mResourceIdAddress.end())
    {
        return itResource->second;
    }
    return "";
}

template<class T> void ClientHolder<T>::StoreResources(const std::string& sIpAddress)
{
    for(auto itResource = m_mmAddressResourceId.lower_bound(sIpAddress); itResource != m_mmAddressResourceId.upper_bound(sIpAddress); ++itResource)
    {
        m_setStored.insert(itResource->second);
    }
}

template<class T> void ClientHolder<T>::RemoveStaleResources(typename std::list<std::shared_ptr<T> >& lstRemoved)
{
    for(auto pResource : m_setStored)
    {
        lstRemoved.push_back(RemoveResource(pResource));
    }
}

template<class T> void ClientHolder<T>::GetResourcesAsList(typename std::list<std::shared_ptr<T> >& lstResources)
{
    for(auto pairResource : m_mResource)
    {
        lstResources.push_back(pairResource.second);
    }
}



template class ClientHolder<Self>;
template class ClientHolder<SenderBase>;
template class ClientHolder<Device>;
template class ClientHolder<Source>;
template class ClientHolder<ReceiverBase>;
template class ClientHolder<Flow>;
