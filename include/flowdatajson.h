#pragma once
#include "flowdata.h"
#include "nmosdlldefine.h"
#include <list>

namespace pml
{
    namespace nmos
    {
        class NMOS_EXPOSE FlowDataJson : public FlowData
        {
            public:
                FlowDataJson(const std::string& sLabel, const std::string& sDescription, const std::string& sSourceId, const std::string& sDeviceId);
                FlowDataJson();
                virtual bool UpdateFromJson(const Json::Value& jsData);
                virtual bool Commit(const ApiVersion& version);

            private:
                std::string m_sEventType;

        };
    };
};



