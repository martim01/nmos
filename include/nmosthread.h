#pragma once
#include "version.h"

class NodeThread
{
    public:
        NodeThread(){}
        static void Main();
        static bool RegisteredOperation(const ApiVersion& version);

};


