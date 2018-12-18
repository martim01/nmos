#pragma once
#include "dlldefine.h"
#include <string>


class NMOS_EXPOSE CurlEvent
{
    public:
        CurlEvent(){}
        virtual ~CurlEvent(){}
        virtual void CurlDone(unsigned long nResult, const std::string& sResponse){}

};
