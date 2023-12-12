#pragma once
#include "source.h"
#include "nmosdlldefine.h"

namespace pml
{
    namespace nmos
    {
        class NMOS_EXPOSE SourceGeneric : public Source
        {
            public:
                SourceGeneric(const std::string& sLabel, const std::string& sDescription, const std::string& sDeviceId, enumFormat eFormat);
                SourceGeneric();
                static std::shared_ptr<SourceGeneric> Create(const Json::Value& jsResponse);
                virtual bool UpdateFromJson(const Json::Value& jsData);
            private:


        };
    };
};
