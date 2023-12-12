#pragma once
#include <map>
#include "json/json.h"
#include "nmosdlldefine.h"
#include <memory>
#include "nmosapiversion.h"
#include <string>
#include <set>
#include <list>
#include "clientapiposter.h"

namespace pml
{
    namespace nmos
    {
        class Resource;

        template<class T> class NMOS_EXPOSE ClientHolder
        {
            public:
                ClientHolder();
                ~ClientHolder();


                bool AddResource(const std::string& sIpAddres, std::shared_ptr<T> pResource);
                resourcechanges<T> RemoveResources(const std::string& sIpAddres);

                void StoreResources(const std::string& sIpAddress);
                typename std::list<std::shared_ptr<const T> > RemoveStaleResources();


                void RemoveAllResources();

                bool ResourceExists(const std::string& sUuid) const;

                const typename std::map<std::string, std::shared_ptr<T> >& GetResources() const;
                typename std::map<std::string, std::shared_ptr<T> >::const_iterator FindNmosResource(const std::string& sUid) const;
                typename std::shared_ptr<T> GetNmosResource(const std::string& sUid);

                std::string GetResourceIpAddress(const std::string& sUid);

                std::shared_ptr<T> UpdateResource(const Json::Value& jsData);
                size_t GetResourceCount() const;

                resourcechanges<T> GetResourcesAsList();

                std::shared_ptr<T> RemoveResource(const std::string& sUid);

            protected:

                bool RemoveResource(std::shared_ptr<T> pResource);


                std::map<std::string, std::shared_ptr<T> > m_mResource;
                std::multimap<std::string, std::string > m_mmAddressResourceId;
                std::multimap<std::string, std::string > m_mResourceIdAddress;

                std::set<std::string> m_setStored;
                std::list<std::shared_ptr<const T>> m_lstRemoved;
        };
    };
};

