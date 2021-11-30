#pragma once
#include "resource.h"
#include <set>
#include "nmosdlldefine.h"


namespace pml
{
    namespace nmos
    {
        class Source;
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

                virtual std::string CreateSDPMediaLine(unsigned short nPort) const=0;
                virtual std::string CreateSDPAttributeLines(std::shared_ptr<const Source> pSource) const=0;


            protected:
                unsigned long m_nMediaClkOffset;

            private:
                std::string m_sFormat;
                std::string m_sSourceId;
                std::string m_sDeviceId;
                std::set<std::string> m_setParent;


        };
    };
};
