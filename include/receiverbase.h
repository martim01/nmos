#pragma once
#include "ioresource.h"
#include <set>
#include <memory>
#include "nmosdlldefine.h"
#include "constraint.h"
#include "connection.h"
#include "activation.h"
#include "nmosapiversion.h"

namespace pml
{
    namespace nmos
    {

        class NMOS_EXPOSE Receiver : public IOResource
        {
            public:

                enum enumType {AUDIO, VIDEO, DATA, MUX};

                Receiver(const std::string& sLabel, const std::string& sDescription, enumTransport eTransport, const std::string& sDeviceId, enumType eType,
                TransportParamsRTP::flagsTP flagsTransport=TransportParamsRTP::CORE);
                virtual ~Receiver();
                //Receiver();

                bool UpdateFromJson(const Json::Value& jsData) override;

                void SetType(enumType eType);


                bool AddCap(const std::string& sCap);
                void RemoveCap(const std::string& sCap);


                virtual bool Commit(const ApiVersion& version);

                std::string GetParentResourceId() const {   return m_sDeviceId; }

                Json::Value GetConnectionStagedJson(const ApiVersion& version) const;
                Json::Value GetConnectionActiveJson(const ApiVersion& version) const;

                const std::string& GetSender() const;


                bool CheckConstraints(const connectionReceiver<activationResponse>& conRequest);
                bool IsLocked() const;
                connectionReceiver<activationResponse> GetStaged() const;



                bool IsMasterEnabled() const;
                bool IsActivateAllowed() const;

                void SetupActivation(const std::string& sInterfaceIp);
                void MasterEnable(bool bEnable);


            private:
                friend class IS05Server;
                friend class IS04Server;
                friend class NodeApiPrivate;

                void SubscribeToSender(const std::string& sSenderId, const std::string& sSdp, const std::string& sInterfaceIp);  //this is the IS-04 way of connecting
                activation::enumActivate Stage(const connectionReceiver<activationResponse>& conRequest);
                void CommitActivation();
                void Activate();

                void SetStagedActivationTime(const std::string& sTime);
                void SetStagedActivationTimePoint(const std::chrono::time_point<std::chrono::high_resolution_clock>& tp);
                void RemoveStagedActivationTime();
                void SetActivationAllowed(bool bAllowed) { m_bActivateAllowed = bAllowed;}

                void ActualizeUnitialisedActive(const std::string& sInterfaceIp);

            protected:

                std::string m_sDeviceId;
                std::string m_sManifest;
                std::string m_sSenderId;
                bool m_bSenderActive;
                enumType m_eType;

                std::set<std::string> m_setCaps;


                connectionReceiver<activationResponse> m_Staged;
                connectionReceiver<activationResponse> m_Active;

                bool m_bActivateAllowed;
                //std::string m_sSenderId;
                std::string m_sInterfaceIp;


                static const std::array<std::string,4> STR_TYPE;
        };
    };
};



