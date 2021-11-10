#pragma once
#include "resource.h"

namespace pml
{
    namespace nmos
    {
        class IOResource : public Resource
        {
            public:
                IOResource(const std::string& sType, const std::string& sLabel, const std::string& sDescription) : Resource(sType, sLabel, sDescription){}
                IOResource(const std::string& sType) : Resource(sType){}

                virtual void Activate(bool bImmediate=false){};
        };
    };
};
