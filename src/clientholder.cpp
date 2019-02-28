#include "clientholder.h"
#include "resource.h"
#include "log.h"
#include "self.h"
#include "device.h"
#include "source.h"
#include "sender.h"
#include "flow.h"
#include "receiver.h"

using namespace std;

template<class T> ClientHolder<T>::ClientHolder()
{

}

template<class T> ClientHolder<T>::~ClientHolder()
{

}


template<class T> bool ClientHolder<T>::AddResource(const string& sIpAddres, shared_ptr<T> pResource)
{
    if(pResource && m_mResource.insert(make_pair(pResource->GetId(), pResource)).second)
    {
        m_mmAddressResourceId.insert(make_pair(sIpAddres, pResource->GetId()));
        m_mResourceIdAddress.insert(make_pair(pResource->GetId(), sIpAddres));
        return true;
    }
    return false;
}

template<class T> bool ClientHolder<T>::RemoveResource(shared_ptr<T> pResource)
{
    if(pResource)
    {
        return (RemoveResource(pResource->GetId())!=0);
    }
    return false;
}

template<class T> std::shared_ptr<T> ClientHolder<T>::RemoveResource(const string& sUuid)
{
    shared_ptr<T> pResource(0);
    map<string, string>::iterator itAddress = m_mResourceIdAddress.find(sUuid);
    if(itAddress != m_mResourceIdAddress.end())
    {
        m_mmAddressResourceId.erase(itAddress->second);
    }
    m_mResourceIdAddress.erase(sUuid);

    typename map<string,shared_ptr<T> >::iterator itResource = m_mResource.find(sUuid);
    if(itResource != m_mResource.end())
    {
        pResource = itResource->second;
    }

    m_mResource.erase(sUuid);
    return pResource;
}

template<class T> void ClientHolder<T>::RemoveResources(const string& sIpAddres, typename std::list<std::shared_ptr<T> >& lstRemoved)
{
    set<string> setRemoved;

    for(multimap<string, string>::iterator itResource = m_mmAddressResourceId.lower_bound(sIpAddres); itResource != m_mmAddressResourceId.upper_bound(sIpAddres); ++itResource)
    {
        setRemoved.insert(itResource->second);
        typename map<string, shared_ptr<T> >::iterator itRemove = m_mResource.find(itResource->second);
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

template<class T> bool ClientHolder<T>::ResourceExists(const string& sUuid) const
{
    return (m_mResource.find(sUuid) != m_mResource.end());
}

template<class T> typename map<string, shared_ptr<T> >::const_iterator ClientHolder<T>::GetResourceBegin() const
{
    return m_mResource.begin();
}

template<class T> typename map<string, shared_ptr<T> >::const_iterator ClientHolder<T>::GetResourceEnd() const
{
    return m_mResource.end();
}

template<class T> typename map<string, shared_ptr<T> >::const_iterator ClientHolder<T>::FindNmosResource(const string& sUuid) const
{
    return m_mResource.find(sUuid);
}

template<class T> typename map<string, shared_ptr<T> >::iterator ClientHolder<T>::GetNmosResource(const string& sUuid)
{
    return m_mResource.find(sUuid);
}

template<class T> size_t ClientHolder<T>::GetResourceCount() const
{
    return m_mResource.size();
}

template<class T> shared_ptr<T> ClientHolder<T>::UpdateResource(const Json::Value& jsData)
{
    typename map<string, shared_ptr<T> >::iterator itResource = GetNmosResource(jsData["id"].asString());
    if(itResource != GetResourceEnd())
    {
        m_setStored.erase(itResource->first);   //resource still exists so make sure we don't remove it later

        Log::Get() <<  itResource->first << " found already " << endl;
        if(itResource->second->UpdateFromJson(jsData))
        {
            Log::Get() <<  itResource->first << " updated " << endl;
        }
        else
        {
            Log::Get() << "Found node but json data incorrect: " << itResource->second->GetJsonParseError() << endl;
        }
        return itResource->second;
    }
    return 0;
}



template<class T> string ClientHolder<T>::GetResourceIpAddress(const string& sUid)
{
    map<string, string>::const_iterator itResource = m_mResourceIdAddress.find(sUid);
    if(itResource != m_mResourceIdAddress.end())
    {
        return itResource->second;
    }
    return "";
}

template<class T> void ClientHolder<T>::StoreResources(const string& sIpAddress)
{
    for(multimap<string, string>::iterator itResource = m_mmAddressResourceId.lower_bound(sIpAddress); itResource != m_mmAddressResourceId.upper_bound(sIpAddress); ++itResource)
    {
        m_setStored.insert(itResource->second);
    }
}

template<class T> void ClientHolder<T>::RemoveStaleResources(typename std::list<std::shared_ptr<T> >& lstRemoved)
{
    for(set<string>::iterator itResource = m_setStored.begin(); itResource != m_setStored.end(); ++itResource)
    {
        lstRemoved.push_back(RemoveResource((*itResource)));
    }

}





template class ClientHolder<Self>;
template class ClientHolder<Sender>;
template class ClientHolder<Device>;
template class ClientHolder<Source>;
template class ClientHolder<Receiver>;
template class ClientHolder<Flow>;
