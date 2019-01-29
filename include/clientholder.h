#pragma once
#include <map>
#include "json/json.h"
#include "nmosdlldefine.h"
#include <memory>
#include "version.h"
#include <string>
#include <set>


class Resource;

template<class T> class NMOS_EXPOSE ClientHolder
{
    public:
        ClientHolder();
        ~ClientHolder();


        bool AddResource(const std::string& sIpAddres, std::shared_ptr<T> pResource);
        bool RemoveResource(std::shared_ptr<T> pResource);
        bool RemoveResource(const std::string& sUid);
        std::set<std::string> RemoveResources(const std::string& sIpAddres);

        void StoreResources(const std::string& sIpAddress);
        std::set<std::string> RemoveStaleResources();

        void RemoveAllResources();

        bool ResourceExists(const std::string& sUuid) const;

        typename std::map<std::string, std::shared_ptr<T> >::const_iterator GetResourceBegin() const;
        typename std::map<std::string, std::shared_ptr<T> >::const_iterator GetResourceEnd() const;
        typename std::map<std::string, std::shared_ptr<T> >::const_iterator FindNmosResource(const std::string& sUid) const;
        typename std::map<std::string, std::shared_ptr<T> >::iterator GetNmosResource(const std::string& sUid);

        std::string GetResourceIpAddress(const std::string& sUid);

        std::shared_ptr<T> UpdateResource(const Json::Value& jsData);
        size_t GetResourceCount() const;
    protected:

        std::map<std::string, std::shared_ptr<T> > m_mResource;
        std::multimap<std::string, std::string > m_mmAddressResourceId;
        std::multimap<std::string, std::string > m_mResourceIdAddress;

        std::set<std::string> m_setStored;
};


