#pragma once
#include "resource.h"
#include <set>
#include <memory>
#include "nmosapiversion.h"
#include "nmosdlldefine.h"
#include "connection.h"
#include "constraint.h"
#include "ioresource.h"
#include "activation.h"

namespace pml
{
    namespace nmos
    {
        class EventPoster;

        class NMOS_EXPOSE Sender : public IOResource
        {
            public:
                Sender(const std::string& sLabel, const std::string& sDescription, const std::string& sFlowId, enumTransport eTransport, const std::string& sDeviceId,
                const std::string& sInterface, TransportParamsRTP::flagsTP flagsTransport=TransportParamsRTP::CORE, const std::optional<std::string>& multicastIp={});

                static std::shared_ptr<Sender> Create(const Json::Value& jsResponse);
                Sender();

                virtual ~Sender();

                /** @brief Set the active destination details to create an SDP. This will be overwritten by IS-05
                **/
                bool UpdateFromJson(const Json::Value& jsData) override;


                void SetManifestHref(const std::string& sHref);
                const std::string& GetManifestHref() const;

                void SetReceiverId(const std::string& sReceiverId, bool bActive);


                std::string GetParentResourceId() const {   return m_sDeviceId;  }
                const std::string& GetFlowId() const    {   return m_sFlowId;   }
                const std::string& GetDeviceId() const { return m_sDeviceId;}

                virtual Json::Value GetConnectionStagedJson(const ApiVersion& version) const;
                virtual Json::Value GetConnectionActiveJson(const ApiVersion& version) const;


                virtual bool CheckConstraints(const connectionSender<activationResponse>& conRequest);

                connectionSender<activationResponse> GetStaged() const;
                connectionSender<activationResponse> GetActive() const;
                const std::string& GetTransportFile() const;

                bool IsLocked();
                bool IsActivateAllowed() const;
                const std::string GetDestinationIp() const {return m_sDestinationIp;}

                bool IsActiveMasterEnabled() const { return (m_Active.GetMasterEnable() && *(m_Active.GetMasterEnable()));}
                bool IsStagedMasterEnabled() const { return (m_Staged.GetMasterEnable() && *(m_Staged.GetMasterEnable()));}


                bool Commit(const ApiVersion& version) override;


                void MasterEnable(bool bEnable);
                void SetTransportFile(const std::string& sSDP);

                void MarkRTPTransmissionAsActive(bool bActive, std::optional<size_t> tp);

                void SetDestinationDetails(const std::string& sDestinationIp, unsigned short nDestinationPort);

            protected:

                friend class IS05Server;
                friend class IS04Server;
                friend class NodeApiPrivate;


                activation::enumActivate Stage(const connectionSender<activationResponse>& conRequest);
                void CommitActivation();
                void Activate(const std::string& sSourceIp);


                void SetupActivation(const std::string& sSourceIp, const std::string& sDestinationIp, const std::string& sSDP);
                void SetStagedActivationTime(const std::string& sTime);
                void SetStagedActivationTimePoint(const std::chrono::time_point<std::chrono::high_resolution_clock>& tp);
                void RemoveStagedActivationTime();
                void SetActivationAllowed(bool bAllowed) { m_bActivateAllowed = bAllowed;}



                void ActualizeUnitialisedActive(const std::string& sSourceIp);

                std::string m_sFlowId;
                std::string m_sDeviceId;
                std::string m_sManifest;
                std::string m_sReceiverId;
                bool m_bSendingToReceiver;


                connectionSender<activationResponse> m_Staged;
                connectionSender<activationResponse> m_Active;

                std::string m_sTransportFile;
                bool m_bActivateAllowed;

                std::string m_sSourceIp;
                std::string m_sDestinationIp;
                std::string m_sSDP;


        };
    };
};


