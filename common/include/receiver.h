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
                friend class NodeApiPrivate;

                void SetSender(const std::string& sSenderId, const std::string& sSdp, const std::string& sInterfaceIp);  //this is the IS-04 way of connecting
                connection::enumActivate Stage(const connectionReceiver& conRequest);
                void CommitActivation();
                void Activate();

                void SetStagedActivationTime(const std::string& sTime);
                void SetStagedActivationTimePoint(const std::chrono::time_point<std::chrono::high_resolution_clock>& tp);
                void RemoveStagedActivationTime();
                void SetActivationAllowed(bool bAllowed) { m_bActivateAllowed = bAllowed;}
        };
    };
};
