#include "receiver.h"

class ReceiverNode : public Receiver
{
    public:
        ReceiverNode(const std::string& sLabel, const std::string& sDescription, enumTransport eTransport, const std::string& sDeviceId, enumType eType, int flagsTransport=TransportParamsRTP::CORE);

        ReceiverNode();

        void Activate(bool bImmediate);
        void CommitActivation();

        bool Stage(const connectionReceiver& conRequest);
};
