#pragma once
#include <string>

namespace pml
{
    namespace nmos
    {
        class FlowSdpCreator
        {
            public:
                FlowSdpCreator();
                virtual ~FlowSdpCreator();

                virtual std::string CreateLines(unsigned short nRtpPort) ;
        };
    };
};
