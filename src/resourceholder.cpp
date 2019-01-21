#include "resourceholder.h"
#include "resource.h"
#include <iostream>
#include "log.h"


ResourceHolder::ResourceHolder(const std::string& sType) :
    m_sType(sType),
    m_nVersion(0),
    m_json(Json::arrayValue)
{

}

ResourceHolder::~ResourceHolder()
{
    RemoveAllResources();
}

void ResourceHolder::ResourceUpdated()
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

unsigned char ResourceHolder::GetVersion() const
{
    return m_nVersion;
}


bool ResourceHolder::AddResource(std::shared_ptr<Resource> pResource)
{
    if(pResource)
    {
        return m_mResourceStaging.insert(make_pair(pResource->GetId(), pResource)).second;
    }
    return false;
}

void ResourceHolder::RemoveResource(std::shared_ptr<Resource> pResource)
{
    if(pResource)
    {
        RemoveResource(pResource->GetId());
    }
}

void ResourceHolder::RemoveResource(std::string sUuid)
{
    std::map<std::string, std::shared_ptr<Resource> >::iterator itResource = m_mResourceStaging.find(sUuid);
    if(itResource != m_mResourceStaging.end())
    {
        m_mResourceStaging.erase(itResource);
    }
}

bool ResourceHolder::Commit(const std::set<ApiVersion>& setVersion)
{
    m_mResource = m_mResourceStaging;
    m_mResourceChanged.clear();

    for(std::set<ApiVersion>::const_iterator itVersion = setVersion.begin(); itVersion != setVersion.end(); ++itVersion)
    {
        m_json.clear();

        for(std::map<std::string, std::shared_ptr<Resource> >::const_iterator itResource = m_mResource.begin(); itResource != m_mResource.end(); ++itResource)
        {
            if(itResource->second->Commit((*itVersion)))
            {
                m_mResourceChanged.insert(make_pair(itResource->first, itResource->second));
            }
            m_json.append(itResource->second->GetJson((*itVersion)));
        }
    }
    if(m_mResourceChanged.empty() == false)
    {
        ResourceUpdated();
        return true;
    }
    return false;
}

const Json::Value& ResourceHolder::GetJson(const ApiVersion& version) const
{
    return m_json;
}

Json::Value ResourceHolder::GetConnectionJson(const ApiVersion& version) const
{
    Json::Value jsArray(Json::arrayValue);
    for(std::map<std::string, std::shared_ptr<Resource> >::const_iterator itResource = m_mResource.begin(); itResource != m_mResource.end(); ++itResource)
    {
        jsArray.append(itResource->first+"/");
    }
    return jsArray;
}

std::map<std::string, std::shared_ptr<Resource> >::const_iterator ResourceHolder::GetResourceBegin() const
{
    return m_mResource.begin();
}

std::map<std::string, std::shared_ptr<Resource> >::const_iterator ResourceHolder::GetResourceEnd() const
{
    return m_mResource.end();
}

std::map<std::string, std::shared_ptr<Resource> >::const_iterator ResourceHolder::FindNmosResource(std::string sUuid) const
{
    return m_mResource.find(sUuid);
}

std::map<std::string, std::shared_ptr<Resource> >::iterator ResourceHolder::GetResource(std::string sUuid)
{
    return m_mResource.find(sUuid);
}

std::map<std::string, std::shared_ptr<Resource> >::const_iterator ResourceHolder::GetStagedResourceBegin() const
{
    return m_mResourceStaging.begin();
}

std::map<std::string, std::shared_ptr<Resource> >::const_iterator ResourceHolder::GetStagedResourceEnd() const
{
    return m_mResourceStaging.end();
}

std::map<std::string, std::shared_ptr<Resource> >::const_iterator ResourceHolder::FindStagedResource(std::string sUuid) const
{
    return m_mResourceStaging.find(sUuid);
}

std::map<std::string, std::shared_ptr<Resource> >::const_iterator ResourceHolder::GetChangedResourceBegin() const
{
    return m_mResourceChanged.begin();
}

std::map<std::string, std::shared_ptr<Resource> >::const_iterator ResourceHolder::GetChangedResourceEnd() const
{
    return m_mResourceChanged.end();
}


void ResourceHolder::RemoveAllResources()
{
    m_mResource.clear();
}

size_t ResourceHolder::GetResourceCount() const
{
    return m_mResource.size();
}


bool ResourceHolder::ResourceExists(const std::string& sUuid) const
{
    return (m_mResource.find(sUuid) != m_mResource.end() || m_mResourceStaging.find(sUuid) != m_mResourceStaging.end());
}
