#pragma once
#include "flow.h"
#include "nmosdlldefine.h"

namespace pml
{
    namespace nmos
    {
        class NMOS_EXPOSE FlowMux : public Flow
        {
            public:
                FlowMux(const std::string& sLabel, const std::string& sDescription, const std::string& sSourceId, const std::string& sDeviceId, const std::string& sMediaType);
                FlowMux();
                virtual bool UpdateFromJson(const Json::Value& jsData);
                virtual bool Commit(const ApiVersion& version);
                void SetMediaType(const std::string& sMediaType);

            private:
                std::string m_sMediaType;
        };
    };
};


