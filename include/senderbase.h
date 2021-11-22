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

        class NMOS_EXPOSE SenderBase : public IOResource
        {
            public:
                SenderBase(const std::string& sLabel, const std::string& sDescription, const std::string& sFlowId, enumTransport eTransport, const std::string& sDeviceId, const std::string& sInterface, TransportParamsRTP::flagsTP flagsTransport=TransportParamsRTP::CORE);
                SenderBase();
                virtual ~SenderBase();

                /** @brief Set the active destination details to create an SDP. This will be overwritten by IS-05
                **/
                bool UpdateFromJson(const Json::Value& jsData) override;



                void SetManifestHref(const std::string& sHref);
                const std::string& GetManifestHref() const;

                void SetReceiverId(const std::string& sReceiverId, bool bActive);


                std::string GetParentResourceId() const {   return m_sDeviceId;  }
                const std::string& GetFlowId() const    {   return m_sFlowId;   }

                Json::Value GetConnectionStagedJson(const ApiVersion& version) const;
                Json::Value GetConnectionActiveJson(const ApiVersion& version) const;
                Json::Value GetConnectionConstraintsJson(const ApiVersion& version) const;

                bool CheckConstraints(const connectionSender& conRequest);
                bool IsLocked();

                connectionSender GetStaged();
                connectionSender GetActive();
                const std::string& GetTransportFile() const;

                bool IsActivateAllowed() const;
                const std::string GetDestinationIp() const {return m_sDestinationIp;}

                bool IsActiveMasterEnabled() const { return m_Active.bMasterEnable;}
                bool IsStageMasterEnabled() const { return m_Active.bMasterEnable;}

                const std::string& GetDeviceId() const { return m_sDeviceId;}

            protected:

                std::string m_sFlowId;
                enumTransport m_eTransport;
                std::string m_sDeviceId;
                std::string m_sManifest;
                std::string m_sReceiverId;
                bool m_bReceiverActive;


                connectionSender m_Staged;
                connectionSender m_Active;
                std::vector<constraintsSender> m_vConstraints;

                std::string m_sTransportFile;
                bool m_bActivateAllowed;

                std::string m_sSourceIp;
                std::string m_sDestinationIp;
                std::string m_sSDP;


        };
    };
};


