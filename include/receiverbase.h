#pragma once
#include "ioresource.h"
#include <set>
#include <memory>
#include "nmosdlldefine.h"
#include "constraint.h"
#include "connection.h"
#include "nmosapiversion.h"

namespace pml
{
    namespace nmos
    {

        class NMOS_EXPOSE ReceiverBase : public IOResource
        {
            public:
                enum enumTransport {RTP, RPT_UCAST, RTP_MCAST, DASH};
                enum enumType {AUDIO, VIDEO, DATA, MUX};

                ReceiverBase(const std::string& sLabel, const std::string& sDescription, enumTransport eTransport, const std::string& sDeviceId, enumType eType, int flagsTransport=TransportParamsRTP::CORE);
                virtual ~ReceiverBase();
                ReceiverBase();

                bool UpdateFromJson(const Json::Value& jsData) override;

                void SetTransport(enumTransport eTransport);
                void SetType(enumType eType);

                void AddInterfaceBinding(const std::string& sInterface);
                void RemoveInterfaceBinding(const std::string& sInterface);

                bool AddCap(const std::string& sCap);
                void RemoveCap(const std::string& sCap);


                virtual bool Commit(const ApiVersion& version);

                std::string GetParentResourceId() const {   return m_sDeviceId; }

                Json::Value GetConnectionStagedJson(const ApiVersion& version) const;
                Json::Value GetConnectionActiveJson(const ApiVersion& version) const;
                Json::Value GetConnectionConstraintsJson(const ApiVersion& version) const;

                const std::string& GetSender() const;


                bool CheckConstraints(const connectionReceiver& conRequest);
                bool IsLocked() const;
                connectionReceiver GetStaged() const;



                bool IsMasterEnabled() const;
                bool IsActivateAllowed() const;


                const std::string& GetTransportType() const { return STR_TRANSPORT[m_eTransport]; }

            protected:


                enumTransport m_eTransport;
                std::string m_sDeviceId;
                std::string m_sManifest;
                std::string m_sSenderId;
                bool m_bSenderActive;
                enumType m_eType;
                std::set<std::string> m_setInterfaces;
                std::set<std::string> m_setCaps;


                connectionReceiver m_Staged;
                connectionReceiver m_Active;
                constraintsReceiver m_constraints;  // @todo constraints should be same parameters as connection

                bool m_bActivateAllowed;
                //std::string m_sSenderId;
                std::string m_sInterfaceIp;

                static const std::string STR_TRANSPORT[4];
                static const std::string STR_TYPE[4];
        };
    };
};



