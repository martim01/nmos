#include "resourceholder.h"
#include "resource.h"
#include <iostream>
#include "log.h"
#include "self.h"
#include "device.h"
#include "source.h"
#include "sender.h"
#include "flow.h"
#include "receiver.h"


using namespace std;

template<class T> ResourceHolder<T>::ResourceHolder(const string& sType) :
    m_sType(sType),
    m_nVersion(0),
    m_json(Json::arrayValue)
{

}

template<class T> ResourceHolder<T>::~ResourceHolder()
{
    RemoveAllResources();
}

template<class T> void ResourceHolder<T>::ResourceUpdated()
{
    if(m_nVersion < 255)
    {
        m_nVersion++;
    }
    else
    {
        m_nVersion = 0;
    }
}

template<class T> unsigned char ResourceHolder<T>::GetVersion() const
{
    return m_nVersion;
}


template<class T> bool ResourceHolder<T>::AddResource(shared_ptr<T> pResource)
{
    if(pResource)
    {
        return m_mResourceStaging.insert(make_pair(pResource->GetId(), pResource)).second;
    }
    return false;
}

template<class T> void ResourceHolder<T>::RemoveResource(shared_ptr<T> pResource)
{
    if(pResource)
    {
        RemoveResource(pResource->GetId());
    }
}

template<class T> void ResourceHolder<T>::RemoveResource(string sUuid)
{
    m_mResourceStaging.erase(sUuid);
}

template<class T> bool ResourceHolder<T>::Commit(const set<ApiVersion>& setVersion)
{
    m_mResource = m_mResourceStaging;
    m_mResourceChanged.clear();

    for(auto version : setVersion)
    {
        m_json.clear();

        for(auto pairResource : m_mResource)
        {
            if(pairResource.second->Commit(version))
            {
                m_mResourceChanged.insert(pairResource);
            }
            m_json.append(pairResource.second->GetJson(version));
        }
    }
    if(m_mResourceChanged.empty() == false)
    {
        ResourceUpdated();
        return true;
    }
    return false;
}

template<class T> const Json::Value& ResourceHolder<T>::GetJson(const ApiVersion& version) const
{
    return m_json;
}

template<class T> Json::Value ResourceHolder<T>::GetConnectionJson(const ApiVersion& version) const
{
    Json::Value jsArray(Json::arrayValue);
    for(auto pairResource : m_mResource)
    {
        jsArray.append(pairResource.first+"/");
    }
    return jsArray;
}

template<class T> typename map<string, shared_ptr<T> >::const_iterator ResourceHolder<T>::GetResourceBegin() const
{
    return m_mResource.begin();
}

template<class T> typename map<string, shared_ptr<T> >::const_iterator ResourceHolder<T>::GetResourceEnd() const
{
    return m_mResource.end();
}

template<class T> typename map<string, shared_ptr<T> >::const_iterator ResourceHolder<T>::FindNmosResource(string sUuid) const
{
    return m_mResource.find(sUuid);
}

template<class T> typename map<string, shared_ptr<T> >::iterator ResourceHolder<T>::GetResource(string sUuid, bool bCommited)
{
    if(bCommited)
    {
        return m_mResource.find(sUuid);
    }
    return m_mResourceStaging.find(sUuid);
}

template<class T> typename map<string, shared_ptr<T> >::const_iterator ResourceHolder<T>::GetStagedResourceBegin() const
{
    return m_mResourceStaging.begin();
}

template<class T> typename map<string, shared_ptr<T> >::const_iterator ResourceHolder<T>::GetStagedResourceEnd() const
{
    return m_mResourceStaging.end();
}

template<class T> typename map<string, shared_ptr<T> >::const_iterator ResourceHolder<T>::FindStagedResource(string sUuid) const
{
    return m_mResourceStaging.find(sUuid);
}

template<class T> typename map<string, shared_ptr<T> >::const_iterator ResourceHolder<T>::GetChangedResourceBegin() const
{
    return m_mResourceChanged.begin();
}

template<class T> typename map<string, shared_ptr<T> >::const_iterator ResourceHolder<T>::GetChangedResourceEnd() const
{
    return m_mResourceChanged.end();
}


template<class T> void ResourceHolder<T>::RemoveAllResources()
{
    m_mResource.clear();
}

template<class T> size_t ResourceHolder<T>::GetResourceCount() const
{
    return m_mResource.size();
}


template<class T> bool ResourceHolder<T>::ResourceExists(const string& sUuid) const
{
    return (m_mResource.find(sUuid) != m_mResource.end() || m_mResourceStaging.find(sUuid) != m_mResourceStaging.end());
}


template class ResourceHolder<Self>;
template class ResourceHolder<Sender>;
template class ResourceHolder<Receiver>;

template class ResourceHolder<Device>;
template class ResourceHolder<Source>;
template class ResourceHolder<Flow>;
