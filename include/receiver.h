#pragma once
#include "receiverbase.h"

namespace pml
{
    namespace nmos
    {
        class NodeApiPrivate;
        class Receiver : public ReceiverBase
        {
            public:
                Receiver(const std::string& sLabel, const std::string& sDescription, enumTransport eTransport, const std::string& sDeviceId, enumType eType, int flagsTransport=TransportParamsRTP::CORE);

                Receiver();

                void SetupActivation(const std::string& sInterfaceIp);
                void MasterEnable(bool bEnable);


            private:
                friend class IS05Server;
                friend class IS04Server;
                void SetSender(const std::string& sSenderId, const std::string& sSdp, const std::string& sInterfaceIp, NodeApiPrivate& api);  //this is the IS-04 way of connecting
                bool Stage(const connectionReceiver& conRequest, NodeApiPrivate& api);
                void CommitActivation(NodeApiPrivate& api);
                void Activate(bool bImmediate, NodeApiPrivate& api);
        };
    };
};
