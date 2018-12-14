#pragma once
#include "resource.h"
#include <set>

class Receiver : public Resource
{
    public:
        enum enumTransport {RTP, RPT_UCAST, RTP_MCAST, DASH};
        enum enumType {AUDIO, VIDEO, DATA, MUX};
        Receiver(std::string sLabel, std::string sDescription, enumTransport eTransport, std::string sDeviceId, enumType eType);

        void SetTransport(enumTransport eTransport);
        void SetType(enumType eType);

        void AddInterfaceBinding(std::string sInterface);
        void RemoveInterfaceBinding(std::string sInterface);

        void AddCap(std::string sCap);
        void RemoveCap(std::string sCap);

        void SetSubscription(std::string sSenderId, bool bActive);
        virtual bool Commit();
    private:
        std::string m_sFlowId;
        enumTransport m_eTransport;
        std::string m_sDeviceId;
        std::string m_sManifest;
        std::string m_sSenderId;
        bool m_bSenderActive;
        enumType m_eType;
        std::set<std::string> m_setInterfaces;
        std::set<std::string> m_setCaps;

        static const std::string TRANSPORT[4];
        static const std::string TYPE[4];
};




