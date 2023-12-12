#include "resourceholder.h"
#include "resource.h"
#include <iostream>
#include "log.h"
#include "self.h"
#include "device.h"
#include "source.h"
#include "senderbase.h"
#include "flow.h"
#include "receiverbase.h"


using namespace pml::nmos;

template<class T> ResourceHolder<T>::ResourceHolder(const std::string& sType) :
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


template<class T> bool ResourceHolder<T>::AddResource(std::shared_ptr<T> pResource)
{
    if(pResource)
    {
        return m_mResourceStaging.insert(make_pair(pResource->GetId(), pResource)).second;
    }
    return false;
}

template<class T> void ResourceHolder<T>::RemoveResource(std::shared_ptr<T> pResource)
{
    if(pResource)
    {
        RemoveResource(pResource->GetId());
    }
}

template<class T> void ResourceHolder<T>::RemoveResource(std::string sUuid)
{
    m_mResourceStaging.erase(sUuid);
}

template<class T> std::list<std::shared_ptr<Resource>> ResourceHolder<T>::Commit(const std::set<ApiVersion>& setVersion)
{
    m_mResource = m_mResourceStaging;
    std::list<std::shared_ptr<Resource>> lstChanged;

    for(auto version : setVersion)
    {
        m_json.clear();

        for(auto pairResource : m_mResource)
        {
            if(pairResource.second->Commit(version))
            {
                pmlLog(pml::LOG_DEBUG, "pml::nmos") << "++++++++++ Commit " << m_sType << ": " << pairResource.first << " +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++";
                lstChanged.push_back(pairResource.second);
            }
            m_json.append(pairResource.second->GetJson(version));
        }
    }
    if(lstChanged.empty() == false)
    {
        ResourceUpdated();
    }
    return lstChanged;
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

template<class T> typename std::map<std::string, std::shared_ptr<T> >::const_iterator ResourceHolder<T>::GetResourceBegin() const
{
    return m_mResource.begin();
}

template<class T> typename std::map<std::string, std::shared_ptr<T> >::const_iterator ResourceHolder<T>::GetResourceEnd() const
{
    return m_mResource.end();
}

template<class T> typename std::map<std::string, std::shared_ptr<T> >::const_iterator ResourceHolder<T>::FindNmosResource(std::string sUuid) const
{
    return m_mResource.find(sUuid);
}

template<class T> typename std::map<std::string, std::shared_ptr<T> >::iterator ResourceHolder<T>::GetResource(std::string sUuid, bool bCommited)
{
    if(bCommited)
    {
        return m_mResource.find(sUuid);
    }
    return m_mResourceStaging.find(sUuid);
}

template<class T> typename std::shared_ptr<T> ResourceHolder<T>::GetStagedResource(const std::string& sUuid) const
{
    auto itResource =  m_mResourceStaging.find(sUuid);
    if(itResource != m_mResourceStaging.end())
    {
        return itResource->second;
    }
    return nullptr;
}

template<class T> typename std::map<std::string, std::shared_ptr<T> >::const_iterator ResourceHolder<T>::GetStagedResourceBegin() const
{
    return m_mResourceStaging.begin();
}

template<class T> typename std::map<std::string, std::shared_ptr<T> >::const_iterator ResourceHolder<T>::GetStagedResourceEnd() const
{
    return m_mResourceStaging.end();
}

template<class T> typename std::map<std::string, std::shared_ptr<T> >::const_iterator ResourceHolder<T>::FindStagedResource(std::string sUuid) const
{
    return m_mResourceStaging.find(sUuid);
}



template<class T> void ResourceHolder<T>::RemoveAllResources()
{
    m_mResource.clear();
    m_mResourceStaging.clear();
}

template<class T> size_t ResourceHolder<T>::GetResourceCount() const
{
    return m_mResource.size();
}


template<class T> bool ResourceHolder<T>::ResourceExists(const std::string& sUuid) const
{
    return (m_mResource.find(sUuid) != m_mResource.end() || m_mResourceStaging.find(sUuid) != m_mResourceStaging.end());
}


template class ResourceHolder<Self>;
template class ResourceHolder<Sender>;
template class ResourceHolder<Receiver>;

template class ResourceHolder<Device>;
template class ResourceHolder<Source>;
template class ResourceHolder<Flow>;
