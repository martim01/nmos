#pragma once
#include "flowdata.h"
#include "nmosdlldefine.h"
#include <list>

namespace pml
{
    namespace nmos
    {
        class NMOS_EXPOSE FlowDataSdiAnc : public FlowData
        {
            public:
                FlowDataSdiAnc(const std::string& sLabel, const std::string& sDescription, const std::string& sSourceId, const std::string& sDeviceId);
                static std::shared_ptr<FlowDataSdiAnc> Create(const Json::Value& jsResponse);
                FlowDataSdiAnc();
                virtual bool UpdateFromJson(const Json::Value& jsData);
                virtual bool Commit(const ApiVersion& version);

                std::string CreateSDPMediaLine(unsigned short nPort) const override;
                std::string CreateSDPAttributeLines(std::shared_ptr<const Source> pSource) const override;

            private:

                //@TODO identification words
                std::list<std::pair<std::string, std::string> > m_lstWords;
        };
    };
};


