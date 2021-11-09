#pragma once
#include <string>

class FlowSdpCreator
{
    public:
        FlowSdpCreator(){}
        virtual ~FlowSdpCreator(){}

        virtual std::string CreateLines(unsigned short nRtpPort)   { return ""; }
};
