#pragma once
#include <map>
#include "json/json.h"
#include "nmosdlldefine.h"
#include <memory>
#include "version.h"

class Resource;

class NMOS_EXPOSE RegistryHolder
{
    public:
        RegistryHolder();
        ~RegistryHolder();


        bool AddResource(std::shared_ptr<Resource> pResource);
        bool RemoveResource(std::shared_ptr<Resource> pResource);
        bool RemoveResource(std::string sUuid);

        void RemoveAllResources();

        const Json::Value& GetJson(const ApiVersion& version);

        bool ResourceExists(const std::string& sUuid) const;

        std::map<std::string, std::shared_ptr<Resource> >::const_iterator GetResourceBegin() const;
        std::map<std::string, std::shared_ptr<Resource> >::const_iterator GetResourceEnd() const;
        std::map<std::string, std::shared_ptr<Resource> >::const_iterator FindNmosResource(std::string sUuid) const;
        std::map<std::string, std::shared_ptr<Resource> >::iterator GetResource(std::string sUuid);

        size_t GetResourceCount() const;
    protected:


        Json::Value m_json;
        std::map<std::string, std::shared_ptr<Resource> > m_mResource;

};

