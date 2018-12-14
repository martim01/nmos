#pragma once
#include "flow.h"


class FlowMux : public Flow
{
    public:
        FlowMux(std::string sLabel, std::string sDescription, std::string sSourceId, std::string sDeviceId, std::string sMediaType);
        virtual Json::Value ToJson() const;

    private:
        std::string m_sMediaType;
};



