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
                void CreateSDP(NodeApiPrivate& api);
                void CreateSDP(NodeApiPrivate& api, const connectionSender& state);



                void Activate(bool bImmediate, NodeApiPrivate& api) override;
                void CommitActivation(NodeApiPrivate& api);
                bool Stage(const connectionSender& conRequest, std::shared_ptr<EventPoster> pPoster, NodeApiPrivate& api);
        };
    };
};
