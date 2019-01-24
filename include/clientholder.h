#pragma once
#include <map>
#include "json/json.h"
#include "nmosdlldefine.h"
#include <memory>
#include "version.h"

class Resource;

class NMOS_EXPOSE ClientHolder
{
    public:
        ClientHolder();
        ~ClientHolder();


        bool AddResource(const std::string& sIpAddres, std::shared_ptr<Resource> pResource);
        bool RemoveResource(std::shared_ptr<Resource> pResource);
        bool RemoveResource(std::string sUuid);
        size_t RemoveResources(const std::string& sIpAddres);

        void RemoveAllResources();

        bool ResourceExists(const std::string& sUuid) const;

        std::map<std::string, std::shared_ptr<Resource> >::const_iterator GetResourceBegin() const;
        std::map<std::string, std::shared_ptr<Resource> >::const_iterator GetResourceEnd() const;
        std::map<std::string, std::shared_ptr<Resource> >::const_iterator FindNmosResource(std::string sUuid) const;
        std::map<std::string, std::shared_ptr<Resource> >::iterator GetNmosResource(std::string sUuid);

        size_t GetResourceCount() const;
    protected:

        std::map<std::string, std::shared_ptr<Resource> > m_mResource;
        std::multimap<std::string, std::string > m_mmAddressResourceId;

};


