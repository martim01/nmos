#pragma once
#include "resource.h"
#include <set>
#include "nmosdlldefine.h"

#include "flowsdpcreator.h"

namespace pml
{
    namespace nmos
    {
        class NMOS_EXPOSE Flow : public Resource
        {
            public:
                Flow(const std::string& sLabel, const std::string& sDescription, const std::string& sFormat, const std::string& sSourceId, const std::string& sDeviceId);
                virtual ~Flow(){}
                Flow(const std::string& sFormat);



                virtual bool UpdateFromJson(const Json::Value& jsData);
                void AddParentId(const std::string& sId);
                void RemoveParentId(const std::string& sId);
                const std::string& GetFormat() const;

                virtual bool Commit(const ApiVersion& version);

                std::string GetParentResourceId() const
                {
                    return m_sDeviceId;
                }
                const std::string& GetSourceId() const
                {
                    return m_sSourceId;
                }

                unsigned long GetMediaClkOffset() const { return m_nMediaClkOffset;}
                void SetMediaClkOffset(unsigned long nOffset);

                std::string CreateSDPLines(unsigned short nRtpPort) const;

                void SetSdpCreator(std::unique_ptr<FlowSdpCreator> pCreator);

            protected:
                unsigned long m_nMediaClkOffset;
                std::unique_ptr<FlowSdpCreator> m_pSdpCreator;

            private:
                std::string m_sFormat;
                std::string m_sSourceId;
                std::string m_sDeviceId;
                std::set<std::string> m_setParent;


        };
    };
};
