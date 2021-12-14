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
                static std::shared_ptr<FlowDataJson> Create(const Json::Value& jsResponse);

                virtual bool UpdateFromJson(const Json::Value& jsData);
                virtual bool Commit(const ApiVersion& version);

                std::string CreateSDPMediaLine(unsigned short nPort) const override;
                std::string CreateSDPAttributeLines(std::shared_ptr<const Source> pSource) const override;

            private:
                std::string m_sEventType;


        };
    };
};



