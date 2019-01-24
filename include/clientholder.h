#pragma once
#include <map>
#include "json/json.h"
#include "nmosdlldefine.h"
#include <memory>
#include "version.h"
#include <string>



class Resource;

template<class T> class NMOS_EXPOSE ClientHolder
{
    public:
        ClientHolder();
        ~ClientHolder();


        bool AddResource(const std::string& sIpAddres, std::shared_ptr<T> pResource);
        bool RemoveResource(std::shared_ptr<T> pResource);
        bool RemoveResource(std::string sUuid);
        size_t RemoveResources(const std::string& sIpAddres);

        void RemoveAllResources();

        bool ResourceExists(const std::string& sUuid) const;

        typename std::map<std::string, std::shared_ptr<T> >::const_iterator GetResourceBegin() const;
        typename std::map<std::string, std::shared_ptr<T> >::const_iterator GetResourceEnd() const;
        typename std::map<std::string, std::shared_ptr<T> >::const_iterator FindNmosResource(std::string sUuid) const;
        typename std::map<std::string, std::shared_ptr<T> >::iterator GetNmosResource(std::string sUuid);

        bool UpdateResource(const Json::Value& jsData);
        size_t GetResourceCount() const;
    protected:

        std::map<std::string, std::shared_ptr<T> > m_mResource;
        std::multimap<std::string, std::string > m_mmAddressResourceId;

};


