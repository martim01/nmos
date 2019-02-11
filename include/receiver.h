#pragma once
#include "resource.h"
#include <set>
#include <memory>
#include "nmosdlldefine.h"
#include "constraint.h"
#include "connection.h"
#include "version.h"

class Sender;
class EventPoster;

class NMOS_EXPOSE Receiver : public Resource
{
    public:
        enum enumTransport {RTP, RPT_UCAST, RTP_MCAST, DASH};
        enum enumType {AUDIO, VIDEO, DATA, MUX};
        Receiver(std::string sLabel, std::string sDescription, enumTransport eTransport, std::string sDeviceId, enumType eType, TransportParamsRTP::flagsTP flagsTransport=TransportParamsRTP::CORE);
        ~Receiver();
        Receiver();
        virtual bool UpdateFromJson(const Json::Value& jsData);

        void SetTransport(enumTransport eTransport);
        void SetType(enumType eType);

        void AddInterfaceBinding(std::string sInterface);
        void RemoveInterfaceBinding(std::string sInterface);

        bool AddCap(std::string sCap);
        void RemoveCap(std::string sCap);


        virtual bool Commit(const ApiVersion& version);

        std::string GetParentResourceId() const
        {
            return m_sDeviceId;
        }

        Json::Value GetConnectionStagedJson(const ApiVersion& version) const;
        Json::Value GetConnectionActiveJson(const ApiVersion& version) const;
        Json::Value GetConnectionConstraintsJson(const ApiVersion& version) const;

        const std::string& GetSender() const;
        void SetSender(const std::string& sSenderId, const std::string& sSdp, const std::string& sInterfaceIp);  //this is the IS-04 way of connecting

        bool CheckConstraints(const connectionReceiver& conRequest);
        bool IsLocked() const;
        bool Stage(const connectionReceiver& conRequest, std::shared_ptr<EventPoster> pPoster);
        connectionReceiver GetStaged() const;

        void Activate(const std::string& sInterfaceIp);

        bool IsMasterEnabled() const;

    private:
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
        constraintsReceiver m_constraints;


        //std::string m_sSenderId;

        static const std::string STR_TRANSPORT[4];
        static const std::string STR_TYPE[4];
};




