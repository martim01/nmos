#include "registryholder.h"
#include "resource.h"
#include <iostream>
#include "log.h"


RegistryHolder::RegistryHolder() :
    m_json(Json::arrayValue)
{

}

RegistryHolder::~RegistryHolder()
{
    RemoveAllResources();
}



bool RegistryHolder::AddResource(std::shared_ptr<Resource> pResource)
{
    if(pResource)
    {
        return m_mResource.insert(make_pair(pResource->GetId(), pResource)).second;
    }
    return false;
}

bool RegistryHolder::RemoveResource(std::shared_ptr<Resource> pResource)
{
    if(pResource)
    {
        return RemoveResource(pResource->GetId());
    }
    return false;
}

bool RegistryHolder::RemoveResource(std::string sUuid)
{
    return (m_mResource.erase(sUuid)!=0);
}


const Json::Value& RegistryHolder::GetJson()
{
    m_json.clear();

    for(std::map<std::string, std::shared_ptr<Resource> >::const_iterator itResource = m_mResource.begin(); itResource != m_mResource.end(); ++itResource)
    {
        itResource->second->Commit();
        m_json.append(itResource->second->GetJson());
    }
    return m_json;
}


std::map<std::string, std::shared_ptr<Resource> >::const_iterator RegistryHolder::GetResourceBegin() const
{
    return m_mResource.begin();
}

std::map<std::string, std::shared_ptr<Resource> >::const_iterator RegistryHolder::GetResourceEnd() const
{
    return m_mResource.end();
}

std::map<std::string, std::shared_ptr<Resource> >::const_iterator RegistryHolder::FindResource(std::string sUuid) const
{
    return m_mResource.find(sUuid);
}

std::map<std::string, std::shared_ptr<Resource> >::iterator RegistryHolder::GetResource(std::string sUuid)
{
    return m_mResource.find(sUuid);
}

void RegistryHolder::RemoveAllResources()
{
    m_mResource.clear();
}

size_t RegistryHolder::GetResourceCount() const
{
    return m_mResource.size();
}


bool RegistryHolder::ResourceExists(const std::string& sUuid) const
{
    return (m_mResource.find(sUuid) != m_mResource.end());
}

