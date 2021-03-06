#pragma once
#include "resource.h"
#include <set>
#include <memory>
#include "version.h"
#include "nmosdlldefine.h"
#include "connection.h"
#include "constraint.h"
#include "ioresource.h"

class EventPoster;

class NMOS_EXPOSE Sender : public IOResource
{
    public:
        enum enumTransport {RTP, RTP_UCAST, RTP_MCAST, DASH};

        Sender(const std::string& sLabel, const std::string& sDescription, const std::string& sFlowId, enumTransport eTransport, const std::string& sDeviceId, const std::string& sInterface, TransportParamsRTP::flagsTP flagsTransport=TransportParamsRTP::CORE);
        Sender();

        /** @brief Set the active destination details to create an SDP. This will be overwritten by IS-05
        **/
        void SetDestinationDetails(const std::string& sDestinationIp, unsigned short nDestinationPort);

        virtual bool UpdateFromJson(const Json::Value& jsData);

        void AddInterfaceBinding(const std::string& sInterface);
        void RemoveInterfaceBinding(const std::string& sInterface);

        void SetTransport(enumTransport eTransport);
        void SetManifestHref(const std::string& sHref);
        const std::string& GetManifestHref() const;

        void SetReceiverId(const std::string& sReceiverId, bool bActive);
        virtual bool Commit(const ApiVersion& version);

        std::string GetParentResourceId() const
        {
            return m_sDeviceId;
        }

        const std::string& GetFlowId() const
        {
            return m_sFlowId;
        }

        Json::Value GetConnectionStagedJson(const ApiVersion& version) const;
        Json::Value GetConnectionActiveJson(const ApiVersion& version) const;
        Json::Value GetConnectionConstraintsJson(const ApiVersion& version) const;

        bool CheckConstraints(const connectionSender& conRequest);
        bool IsLocked();

        bool Stage(const connectionSender& conRequest, std::shared_ptr<EventPoster> pPoster);
        connectionSender GetStaged();
        connectionSender GetActive();

        // called by the main thread as a reply to the eventposter
        void SetupActivation(const std::string& sSourceIp="", const std::string& sDestinationIp="", const std::string& sSDP="");
        void Activate(bool bImmediate=false);

        void MasterEnable(bool bEnable);


        const std::string& GetTransportFile() const;
        void CreateSDP();

        bool IsActivateAllowed() const;
        void CommitActivation();
    private:



        std::string m_sFlowId;
        enumTransport m_eTransport;
        std::string m_sDeviceId;
        std::string m_sManifest;
        std::string m_sReceiverId;
        bool m_bReceiverActive;
        std::set<std::string> m_setInterfaces;

        connectionSender m_Staged;
        connectionSender m_Active;
        constraintsSender m_constraints;

        std::string m_sTransportFile;
        bool m_bActivateAllowed;

        std::string m_sSourceIp;
        std::string m_sDestinationIp;
        std::string m_sSDP;

        static const std::string STR_TRANSPORT[4];
};



