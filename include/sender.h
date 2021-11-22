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


                void SetupActivation(const std::string& sSourceIp, const std::string& sDestinationIp, const std::string& sSDP);
                void SetDestinationDetails(const std::string& sDestinationIp, unsigned short nDestinationPort);
                void MasterEnable(bool bEnable);

                bool Commit(const ApiVersion& version) override;



            private:
                void SetTransportFile(const std::string& sSDP);

                void SetStagedActivationTime(const std::string& sTime);
                void SetStagedActivationTimePoint(const std::chrono::time_point<std::chrono::high_resolution_clock>& tp);
                void RemoveStagedActivationTime();
                void SetActivationAllowed(bool bAllowed) { m_bActivateAllowed = bAllowed;}

                void Activate(const std::string& sSourceIp);
                void CommitActivation();
                connection::enumActivate Stage(const connectionSender& conRequest);
        };
    };
};
