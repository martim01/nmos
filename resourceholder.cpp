#include "resourceholder.h"
#include "resource.h"
#include <iostream>

ResourceHolder::ResourceHolder() : m_nVersion(0),
    m_json(Json::arrayValue)
{

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


void ResourceHolder::AddResource(Resource* pResource)
{
    if(pResource)
    {
        m_mResourceStaging.insert(make_pair(pResource->GetId(), pResource));
    }
}

void ResourceHolder::RemoveResource(Resource* pResource)
{
    if(pResource)
    {
        RemoveReosource(pResource->GetId());
    }
}

void ResourceHolder::RemoveReosource(std::string sUuid)
{
    std::map<std::string, Resource*>::iterator itResource = m_mResourceStaging.find(sUuid);
    if(itResource != m_mResourceStaging.end())
    {
        delete itResource->second;
        m_mResourceStaging.erase(itResource);
    }
}

bool ResourceHolder::Commit()
{
    std::cout << "ResourceHolder: Commit" << std::endl;
    m_mResource = m_mResourceStaging;

    m_json.clear();
    bool bChanged(false);
    for(std::map<std::string, Resource*>::const_iterator itResource = m_mResource.begin(); itResource != m_mResource.end(); ++itResource)
    {
        std::cout << "Resource: " << itResource->first << " committed" << std::endl;
        bChanged |= itResource->second->Commit();
        m_json.append(itResource->second->GetJson());
    }
    if(bChanged)
    {
        ResourceUpdated();
    }
    return bChanged;
}

const Json::Value& ResourceHolder::GetJson() const
{
    return m_json;
}


std::map<std::string, Resource*>::const_iterator ResourceHolder::GetResourceBegin() const
{
    return m_mResource.begin();
}

std::map<std::string, Resource*>::const_iterator ResourceHolder::GetResourceEnd() const
{
    return m_mResource.end();
}

std::map<std::string, Resource*>::const_iterator ResourceHolder::FindResource(std::string sUuid) const
{
    return m_mResource.find(sUuid);
}
