#pragma once
#include "resource.h"
#include <set>
#include <memory>

#include "dlldefine.h"
#include "connection.h"
#include "constraint.h"

class EventPoster;

class NMOS_EXPOSE Sender : public Resource
{
    public:
        enum enumTransport {RTP, RTP_UCAST, RTP_MCAST, DASH};
        Sender(std::string sLabel, std::string sDescription, std::string sFlowId, enumTransport eTransport, std::string sDeviceId, std::string sManifestHref);

        Sender(const Json::Value& jsData);


        void AddInterfaceBinding(std::string sInterface);
        void RemoveInterfaceBinding(std::string sInterface);

        void SetTransport(enumTransport eTransport);
        void SetManifestHref(std::string sHref);

        void SetReceiverId(std::string sReceiverId, bool bActive);
        virtual bool Commit();

        const std::string& GetDeviceId() const
        {
            return m_sDeviceId;
        }

        const std::string& GetFlowId() const
        {
            return m_sFlowId;
        }

        Json::Value GetConnectionStagedJson() const;
        Json::Value GetConnectionActiveJson() const;
        Json::Value GetConnectionConstraintsJson() const;

        bool CheckConstraints(const connectionSender& conRequest);
        bool IsLocked();

        bool Stage(const connectionSender& conRequest, std::shared_ptr<EventPoster> pPoster);
        connectionSender GetStaged();

        // called by the main thread as a reply to the eventposter
        void Activate();


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






        static const std::string TRANSPORT[4];
};



