#pragma once
#include "resource.h"
#include <set>
#include <memory>
#include "nmosapiversion.h"
#include "nmosdlldefine.h"
#include "connection.h"
#include "constraint.h"
#include "ioresource.h"

namespace pml
{
    namespace nmos
    {
        class EventPoster;

        class NMOS_EXPOSE Sender : public IOResource
        {
            public:
                Sender(const std::string& sLabel, const std::string& sDescription, const std::string& sFlowId, enumTransport eTransport, const std::string& sDeviceId, const std::string& sInterface,
                           TransportParamsRTP::flagsTP flagsTransport=TransportParamsRTP::CORE);
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
                virtual Json::Value GetConnectionConstraintsJson(const ApiVersion& version) const;

                virtual bool CheckConstraints(const connectionSender& conRequest);

                connectionSender GetStaged();
                connectionSender GetActive();
                const std::string& GetTransportFile() const;

                bool IsLocked();
                bool IsActivateAllowed() const;
                const std::string GetDestinationIp() const {return m_sDestinationIp;}

                bool IsActiveMasterEnabled() const { return m_Active.bMasterEnable;}
                bool IsStageMasterEnabled() const { return m_Active.bMasterEnable;}


                bool Commit(const ApiVersion& version) override;


                void MasterEnable(bool bEnable);
                void SetTransportFile(const std::string& sSDP);

            protected:

                friend class IS05Server;
                friend class IS04Server;
                friend class NodeApiPrivate;

                connection::enumActivate Stage(const connectionSender& conRequest);
                void CommitActivation();
                void Activate(const std::string& sSourceIp);

                void SetupActivation(const std::string& sSourceIp, const std::string& sDestinationIp, const std::string& sSDP);
                void SetStagedActivationTime(const std::string& sTime);
                void SetStagedActivationTimePoint(const std::chrono::time_point<std::chrono::high_resolution_clock>& tp);
                void RemoveStagedActivationTime();
                void SetActivationAllowed(bool bAllowed) { m_bActivateAllowed = bAllowed;}

                void SetDestinationDetails(const std::string& sDestinationIp, unsigned short nDestinationPort);

                std::string m_sFlowId;
                enumTransport m_eTransport;
                std::string m_sDeviceId;
                std::string m_sManifest;
                std::string m_sReceiverId;
                bool m_bReceiverActive;


                connectionSender m_Staged;
                connectionSender m_Active;
                std::vector<ConstraintsSender> m_vConstraints;

                std::string m_sTransportFile;
                bool m_bActivateAllowed;

                std::string m_sSourceIp;
                std::string m_sDestinationIp;
                std::string m_sSDP;


        };
    };
};


