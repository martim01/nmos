#pragma once
#include "dlldefine.h"
#include <string>

struct dnsInstance;

class NMOS_EXPOSE ServiceBrowserEvent
{
    public:
        ServiceBrowserEvent(){}
        virtual ~ServiceBrowserEvent(){}

        virtual void InstanceResolved(dnsInstance* pInstance){}
        virtual void AllForNow(const std::string& sService){}
        virtual void Finished(){}
};
