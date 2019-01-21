#pragma once
#include "flowdata.h"
#include "dlldefine.h"
#include <list>

class NMOS_EXPOSE FlowDataSdiAnc : public FlowData
{
    public:
        FlowDataSdiAnc(std::string sLabel, std::string sDescription, std::string sSourceId, std::string sDeviceId);
        FlowDataSdiAnc();
        virtual bool UpdateFromJson(const Json::Value& jsData);
        virtual bool Commit(const ApiVersion& version);

        virtual std::string CreateSDPLines(unsigned short nRtpPort) const;

    private:
        //@TODO identification words
        std::list<std::pair<std::string, std::string> > m_lstWords;
};



