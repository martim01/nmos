#pragma once
#include "resource.h"
#include <set>

class Sender : public Resource
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
    private:
        std::string m_sFlowId;
        enumTransport m_eTransport;
        std::string m_sDeviceId;
        std::string m_sManifest;
        std::string m_sReceiverId;
        bool m_bReceiverActive;
        std::set<std::string> m_setInterfaces;

        static const std::string TRANSPORT[4];
};



