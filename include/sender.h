#pragma once
#include "senderbase.h"

namespace pml
{
    namespace nmos
    {
        class Sender : public SenderBase
        {
            public:
                Sender(const std::string& sLabel, const std::string& sDescription, const std::string& sFlowId, enumTransport eTransport, const std::string& sDeviceId, const std::string& sInterface, TransportParamsRTP::flagsTP flagsTransport=TransportParamsRTP::CORE);
                Sender();
                virtual ~Sender(){}

                void CreateSDP();

                void Activate(bool bImmediate=false);
                void CommitActivation();

                bool Commit(const ApiVersion& version) override;

                void SetupActivation(const std::string& sSourceIp, const std::string& sDestinationIp, const std::string& sSDP);
                void SetDestinationDetails(const std::string& sDestinationIp, unsigned short nDestinationPort);
                void MasterEnable(bool bEnable);

                bool Stage(const connectionSender& conRequest, std::shared_ptr<EventPoster> pPoster);

            protected:
                void CreateSDP(const connectionSender& state);
        };
    };
};
