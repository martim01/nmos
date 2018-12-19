#pragma once
#include "dlldefine.h"
#include <string>
struct dnsInstance;

class NMOS_EXPOSE EventPoster
{
    public:
        EventPoster(){}
        virtual ~EventPoster(){}
        virtual void CurlDone(unsigned long nResult, const std::string& sResponse, long nType){}
        virtual void InstanceResolved(dnsInstance* pInstance){}
        virtual void AllForNow(const std::string& sService){}
        virtual void Finished(){}
        virtual void RegistrationNodeError(){}
        virtual void InstanceRemoved(const std::string& sInstance){}
};
