#pragma once
#include <string>
#include <memory>
#include "nmosdlldefine.h"


class Resource;

class NMOS_EXPOSE Registry
{
    public:
        Registry(){}
        virtual ~Registry(){}


        virtual bool DeleteResource(const std::string& sType, const std::string& sId)=0;
        virtual size_t Heartbeat(const std::string& sId)=0;
        virtual std::shared_ptr<Resource> FindResource(const std::string& sType, const std::string& sId)=0;
        virtual bool AddResource(const std::string& sType, std::shared_ptr<Resource> pResource)=0;
        virtual bool ResourceUpdated(std::shared_ptr<Resource> pResource)=0;

        virtual bool GarbageCollection()=0;

};

