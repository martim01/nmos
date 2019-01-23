#include "clientholder.h"
#include "resource.h"
using namespace std;

ClientHolder::ClientHolder()
{

}

ClientHolder::~ClientHolder()
{

}


bool ClientHolder::AddResource(const string& sIpAddres, shared_ptr<Resource> pResource)
{
    if(pResource && m_mResource.insert(make_pair(pResource->GetId(), pResource)).second)
    {
        m_mmAddressResourceId.insert(make_pair(sIpAddres, pResource->GetId()));
        return true;
    }
    return false;
}

bool ClientHolder::RemoveResource(shared_ptr<Resource> pResource)
{
    if(pResource)
    {
        return RemoveResource(pResource->GetId());
    }
    return false;
}

bool ClientHolder::RemoveResource(string sUuid)
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

size_t ClientHolder::RemoveResources(const string& sIpAddres)
{
    size_t nResources = m_mResource.size();
    for(multimap<string, string>::iterator itResource = m_mmAddressResourceId.lower_bound(sIpAddres); itResource != m_mmAddressResourceId.upper_bound(sIpAddres); ++itResource)
    {
        m_mResource.erase(itResource->second);
    }
    m_mmAddressResourceId.erase(sIpAddres);
    return (nResources-m_mResource.size());
}

void ClientHolder::RemoveAllResources()
{
    m_mmAddressResourceId.clear();
    m_mResource.clear();
}

bool ClientHolder::ResourceExists(const string& sUuid) const
{
    return (m_mResource.find(sUuid) != m_mResource.end());
}

map<string, shared_ptr<Resource> >::const_iterator ClientHolder::GetResourceBegin() const
{
    return m_mResource.begin();
}

map<string, shared_ptr<Resource> >::const_iterator ClientHolder::GetResourceEnd() const
{
    return m_mResource.end();
}

map<string, shared_ptr<Resource> >::const_iterator ClientHolder::FindNmosResource(string sUuid) const
{
    return m_mResource.find(sUuid);
}

size_t ClientHolder::GetResourceCount() const
{
    return m_mResource.size();
}



