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
                static std::shared_ptr<FlowMux> Create(const Json::Value& jsResponse);
                virtual bool UpdateFromJson(const Json::Value& jsData);
                virtual bool Commit(const ApiVersion& version);
                void SetMediaType(const std::string& sMediaType);
                FlowMux();
            private:
                std::string m_sMediaType;

        };
    };
};


