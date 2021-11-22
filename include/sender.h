#pragma once
#include "senderbase.h"

namespace pml
{
    namespace nmos
    {
        class NodeApiPrivate;

        class Sender : public SenderBase
        {
            friend class IS04Server;
            friend class IS05Server;
            friend class NodeApiPrivate;


            public:
                Sender(const std::string& sLabel, const std::string& sDescription, const std::string& sFlowId, enumTransport eTransport, const std::string& sDeviceId, const std::string& sInterface, TransportParamsRTP::flagsTP flagsTransport=TransportParamsRTP::CORE);
                Sender();
                virtual ~Sender(){}





            private:

        };
    };
};
