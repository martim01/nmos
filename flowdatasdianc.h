#pragma once
#include "flowdata.h"


class FlowDataSdiAnc : public FlowData
{
    public:
        FlowDataSdiAnc(std::string sLabel, std::string sDescription, std::string sSourceId, std::string sDeviceId, std::string sMediaType);
        virtual Json::Value ToJson() const;

    private:
        //@TODO identification words
};



