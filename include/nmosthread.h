#pragma once
#include "version.h"

namespace pml
{
    namespace nmos
    {
        class NodeThread
        {
            public:
                NodeThread(){}
                static void Main();
                static bool RegisteredOperation(const ApiVersion& version);
                static bool FindRegisterNode();
                static bool HandleHeartbeatResponse(unsigned int nResponse, const ApiVersion& version);

        };
    };
};

