#pragma once
#include "receiverbase.h"

namespace pml
{
    namespace nmos
    {
        class Receiver : public ReceiverBase
        {
            public:
                Receiver(const std::string& sLabel, const std::string& sDescription, enumTransport eTransport, const std::string& sDeviceId, enumType eType, int flagsTransport=TransportParamsRTP::CORE);

                Receiver();

                void Activate(bool bImmediate);
                void CommitActivation();

                bool Stage(const connectionReceiver& conRequest);
        };
    };
};
