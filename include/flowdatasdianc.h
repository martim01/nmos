#pragma once
#include "flowdata.h"
#include "dlldefine.h"

class NMOS_EXPOSE FlowDataSdiAnc : public FlowData
{
    public:
        FlowDataSdiAnc(std::string sLabel, std::string sDescription, std::string sSourceId, std::string sDeviceId, std::string sMediaType);
        virtual bool Commit();

        virtual std::string CreateSDPLines(unsigned short nRtpPort) const;

    private:
        //@TODO identification words
};



