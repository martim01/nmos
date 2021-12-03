#pragma once
#include "json/json.h"
#include "nmosdlldefine.h"
#include <memory>
#include "nmosapiversion.h"
#include <set>
#include <list>
namespace pml
{
    namespace nmos
    {
        class Resource;


        template<class T> class NMOS_EXPOSE ResourceHolder
        {
            public:
                ResourceHolder(const std::string& sType);
                ~ResourceHolder();
                const std::string& GetType() const
                {
                    return m_sType;
                }
                void SetType(const std::string& sType)
                {
                    m_sType = sType;
                }

                unsigned char GetVersion() const;

                bool AddResource(std::shared_ptr<T> pResource);
                void RemoveResource(std::shared_ptr<T> pResource);
                void RemoveResource(std::string sUuid);

                void RemoveAllResources();

                virtual typename std::list<std::shared_ptr<Resource>> Commit(const std::set<ApiVersion>& setVersion);

                const Json::Value& GetJson(const ApiVersion& version) const;
                Json::Value GetConnectionJson(const ApiVersion& version) const;

                bool ResourceExists(const std::string& sUuid) const;

                const typename std::map<std::string, std::shared_ptr<T> >& GetResources() const { return m_mResource; }
                typename std::map<std::string, std::shared_ptr<T> >::const_iterator GetResourceBegin() const;
                typename std::map<std::string, std::shared_ptr<T> >::const_iterator GetResourceEnd() const;
                typename std::map<std::string, std::shared_ptr<T> >::const_iterator FindNmosResource(std::string sUuid) const;
                typename std::map<std::string, std::shared_ptr<T> >::iterator GetResource(std::string sUuid,bool bCommited=true);

                typename std::map<std::string, std::shared_ptr<T> >::const_iterator GetStagedResourceBegin() const;
                typename std::map<std::string, std::shared_ptr<T> >::const_iterator GetStagedResourceEnd() const;
                typename std::map<std::string, std::shared_ptr<T> >::const_iterator FindStagedResource(std::string sUuid) const;

                size_t GetResourceCount() const;
            protected:



                void ResourceUpdated();

                std::string m_sType;
                unsigned char m_nVersion;
                Json::Value m_json;
                typename std::map<std::string, std::shared_ptr<T> > m_mResource;
                typename std::map<std::string, std::shared_ptr<T> > m_mResourceStaging;



        };
    };
};
