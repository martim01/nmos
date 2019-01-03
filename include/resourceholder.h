#pragma once
#include "json/json.h"
#include "dlldefine.h"

class Resource;

class NMOS_EXPOSE ResourceHolder
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

        void AddResource(Resource* pResource);
        void RemoveResource(Resource* pResource);
        void RemoveResource(std::string sUuid);

        void RemoveAllResources();

        virtual bool Commit();

        const Json::Value& GetJson() const;
        Json::Value GetConnectionJson() const;

        bool ResourceExists(const std::string& sUuid) const;

        std::map<std::string, Resource*>::const_iterator GetResourceBegin() const;
        std::map<std::string, Resource*>::const_iterator GetResourceEnd() const;
        std::map<std::string, Resource*>::const_iterator FindResource(std::string sUuid) const;
        std::map<std::string, Resource*>::iterator GetResource(std::string sUuid);

        std::map<std::string, Resource*>::const_iterator GetChangedResourceBegin() const;
        std::map<std::string, Resource*>::const_iterator GetChangedResourceEnd() const;

        std::map<std::string, Resource*>::const_iterator GetStagedResourceBegin() const;
        std::map<std::string, Resource*>::const_iterator GetStagedResourceEnd() const;
        std::map<std::string, Resource*>::const_iterator FindStagedResource(std::string sUuid) const;

        size_t GetResourceCount() const;
    protected:



        void ResourceUpdated();

        std::string m_sType;
        unsigned char m_nVersion;
        Json::Value m_json;
        std::map<std::string, Resource*> m_mResource;
        std::map<std::string, Resource*> m_mResourceStaging;
        std::map<std::string, Resource*> m_mResourceChanged;


};
