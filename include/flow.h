#pragma once
#include "resource.h"
#include <set>
#include "nmosdlldefine.h"

class NMOS_EXPOSE Flow : public Resource
{
    public:
        Flow(std::string sLabel, std::string sDescription, std::string sFormat, std::string sSourceId, std::string sDeviceId);
        virtual ~Flow(){}
        Flow(std::string sFormat);
        virtual bool UpdateFromJson(const Json::Value& jsData);
        void AddParentId(std::string sId);
        void RemoveParentId(std::string sId);

        virtual bool Commit(const ApiVersion& version);

        std::string GetParentResourceId() const
        {
            return m_sDeviceId;
        }
        const std::string& GetSourceId() const
        {
            return m_sSourceId;
        }

        void SetMediaClkOffset(unsigned long nOffset);

        virtual std::string CreateSDPLines(unsigned short nRtpPort) const=0;

    protected:
        unsigned long m_nMediaClkOffset;

    private:
        std::string m_sFormat;
        std::string m_sSourceId;
        std::string m_sDeviceId;
        std::set<std::string> m_setParent;


};

