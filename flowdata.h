#pragma once
#include "flow.h"


class FlowData : public Flow
{
    public:
        FlowData(std::string sLabel, std::string sDescription, std::string sSourceId, std::string sDeviceId, std::string sMediaType);
        virtual bool Commit();
        void SetMediaType(std::string sMediaType);

    private:
        std::string m_sMediaType;
};


