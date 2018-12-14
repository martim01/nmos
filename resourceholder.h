#pragma once
#include "json/json.h"
class Resource;

class ResourceHolder
{
    public:
        ResourceHolder();


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
        unsigned char m_nVersion;
        Json::Value m_json;
        std::map<std::string, Resource*> m_mResource;
        std::map<std::string, Resource*> m_mResourceStaging;


};
