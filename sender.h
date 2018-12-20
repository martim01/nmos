#pragma once
#include "resource.h"
#include <set>
#include "dlldefine.h"
#include "transportparams.h"

class NMOS_EXPOSE Sender : public Resource
{
    public:
        enum enumTransport {RTP, RPT_UCAST, RTP_MCAST, DASH};
        Sender(std::string sLabel, std::string sDescription, std::string sFlowId, enumTransport eTransport, std::string sDeviceId, std::string sManifestHref);

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

    private:
        std::string m_sFlowId;
        enumTransport m_eTransport;
        std::string m_sDeviceId;
        std::string m_sManifest;
        std::string m_sReceiverId;
        bool m_bReceiverActive;
        std::set<std::string> m_setInterfaces;


        //Connection API
        struct connection
        {
            enum enumActivate {ACT_NULL, ACT_NOW, ACT_ABSOLUTE, ACT_RELATIVE};
            connection() : bMasterEnable(true), eActivate(ACT_NULL){}
            TransportParamsRTPSender tpSender;
            std::string sReceiverId;
            bool bMasterEnable;
            enumActivate eActivate;
            std::string sActivationTime;
            static const std::string STR_ACTIVATE[4];

        };
        connection m_Staged;
        connection m_Active;

        Json::Value GetConnectionJson(const connection& con) const;




        static const std::string TRANSPORT[4];
};



