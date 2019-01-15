#pragma once
#include "resource.h"
#include <set>
#include <memory>
#include "dlldefine.h"
#include "constraint.h"
#include "connection.h"

class Sender;
class EventPoster;

class NMOS_EXPOSE Receiver : public Resource
{
    public:
        enum enumTransport {RTP, RPT_UCAST, RTP_MCAST, DASH};
        enum enumType {AUDIO, VIDEO, DATA, MUX};
        Receiver(std::string sLabel, std::string sDescription, enumTransport eTransport, std::string sDeviceId, enumType eType);
        ~Receiver();
        Receiver();
        virtual bool UpdateFromJson(const Json::Value& jsData);

        void SetTransport(enumTransport eTransport);
        void SetType(enumType eType);

        void AddInterfaceBinding(std::string sInterface);
        void RemoveInterfaceBinding(std::string sInterface);

        bool AddCap(std::string sCap);
        void RemoveCap(std::string sCap);


        virtual bool Commit();

        std::string GetParentResourceId() const
        {
            return m_sDeviceId;
        }

        Json::Value GetConnectionStagedJson() const;
        Json::Value GetConnectionActiveJson() const;
        Json::Value GetConnectionConstraintsJson() const;

        std::shared_ptr<Sender> GetSender() const;
        void SetSender(std::shared_ptr<Sender>);  //this is the IS-04 way of connecting

        bool CheckConstraints(const connectionReceiver& conRequest);
        bool IsLocked() const;
        bool Stage(const connectionReceiver& conRequest, std::shared_ptr<EventPoster> pPoster);
        connectionReceiver GetStaged() const;

        void Activate(const std::string& sInterfaceIp);

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


        std::shared_ptr<Sender> m_pSender;

        static const std::string STR_TRANSPORT[4];
        static const std::string STR_TYPE[4];
};




