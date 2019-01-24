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
        return true;
    }
    return false;
}

template<class T> bool ClientHolder<T>::RemoveResource(shared_ptr<T> pResource)
{
    if(pResource)
    {
        return RemoveResource(pResource->GetId());
    }
    return false;
}

template<class T> bool ClientHolder<T>::RemoveResource(string sUuid)
{
    for(multimap<string, string>::iterator itAddress = m_mmAddressResourceId.begin(); itAddress != m_mmAddressResourceId.end(); ++itAddress)
    {
        if(itAddress->second == sUuid)
        {
            m_mmAddressResourceId.erase(itAddress);
            break;
        }
    }
    m_mResource.erase(sUuid);
    return true;
}

template<class T> size_t ClientHolder<T>::RemoveResources(const string& sIpAddres)
{
    size_t nResources = m_mResource.size();
    for(multimap<string, string>::iterator itResource = m_mmAddressResourceId.lower_bound(sIpAddres); itResource != m_mmAddressResourceId.upper_bound(sIpAddres); ++itResource)
    {
        m_mResource.erase(itResource->second);
    }
    m_mmAddressResourceId.erase(sIpAddres);
    return (nResources-m_mResource.size());
}

template<class T> void ClientHolder<T>::RemoveAllResources()
{
    m_mmAddressResourceId.clear();
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

template<class T> typename map<string, shared_ptr<T> >::const_iterator ClientHolder<T>::FindNmosResource(string sUuid) const
{
    return m_mResource.find(sUuid);
}

template<class T> typename map<string, shared_ptr<T> >::iterator ClientHolder<T>::GetNmosResource(string sUuid)
{
    return m_mResource.find(sUuid);
}

template<class T> size_t ClientHolder<T>::GetResourceCount() const
{
    return m_mResource.size();
}

template<class T> bool ClientHolder<T>::UpdateResource(const Json::Value& jsData)
{
    typename map<string, shared_ptr<T> >::iterator itResource = GetNmosResource(jsData["id"].asString());
    if(itResource != GetResourceEnd())
    {
        Log::Get() <<  itResource->first << " found already " << endl;
        if(itResource->second->UpdateFromJson(jsData))
        {
            Log::Get() <<  itResource->first << " updated " << endl;
        }
        else
        {
            Log::Get() << "Found node but json data incorrect: " << itResource->second->GetJsonParseError() << endl;
        }
        return true;
    }
    return false;
}


template class ClientHolder<Self>;
template class ClientHolder<Sender>;
template class ClientHolder<Device>;
template class ClientHolder<Source>;
template class ClientHolder<Receiver>;
template class ClientHolder<Flow>;
