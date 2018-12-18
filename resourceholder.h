#pragma once
#include "json/json.h"
#include "dlldefine.h"

class Resource;

class NMOS_EXPOSE ResourceHolder
{
    public:
        ResourceHolder(const std::string& sType);
        const std::string& GetType() const
        {
            return m_sType;
        }

        unsigned char GetVersion() const;

        void AddResource(Resource* pResource);
        void RemoveResource(Resource* pResource);
        void RemoveReosource(std::string sUuid);

        virtual bool Commit();

        const Json::Value& GetJson() const;

        std::map<std::string, Resource*>::const_iterator GetResourceBegin() const;
        std::map<std::string, Resource*>::const_iterator GetResourceEnd() const;
        std::map<std::string, Resource*>::const_iterator FindResource(std::string sUuid) const;

    protected:
        void ResourceUpdated();
        std::string m_sType;
        unsigned char m_nVersion;
        Json::Value m_json;
        std::map<std::string, Resource*> m_mResource;
        std::map<std::string, Resource*> m_mResourceStaging;


};
