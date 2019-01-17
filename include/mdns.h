#pragma once
#include <string>
#include <map>
#include <list>
#include <memory>
#include "dlldefine.h"

struct NMOS_EXPOSE dnsInstance
{
    dnsInstance(){}
    dnsInstance(std::string sN) : sName(sN){}

    std::string sName;
    std::string sHostName;
    std::string sHostIP;
    std::string sService;
    unsigned long nPort;
    std::string sInterface;
    std::map<std::string, std::string> mTxt;

};


struct NMOS_EXPOSE dnsService
{
    dnsService(){}
    dnsService(std::string ss) : sService(ss){}

    ~dnsService()
    {
        lstInstances.clear();
    }

    std::string sService;
    std::list<std::shared_ptr<dnsInstance> > lstInstances;

};

