#pragma once
#include "resource.h"
#include <array>
#include <set>
#include <string>

namespace pml
{
    namespace nmos
    {
        class NodeApiPrivate;
        class IOResource : public Resource
        {
            public:
                enum enumTransport {RTP, RTP_UCAST, RTP_MCAST, DASH, MQTT, WEBSOCKET, UNKNOWN_TRANSPORT};

                IOResource(const std::string& sType, const std::string& sLabel, const std::string& sDescription, enumTransport eTransport) : Resource(sType, sLabel, sDescription), m_eTransport(eTransport){}
                IOResource(const std::string& sType) : Resource(sType){}

                virtual void Activate(bool bImmediate, NodeApiPrivate& api){};

                std::string GetTransportType() const
                {
                    if(m_eTransport < STR_TRANSPORT.size())
                    {
                        return STR_TRANSPORT[m_eTransport];
                    }
                    else
                    {
                        return "Unknown";
                    }
                }
                void SetTransport(enumTransport eTransport);

                void AddInterfaceBinding(const std::string& sInterface);
                void RemoveInterfaceBinding(const std::string& sInterface);


            protected:
                enumTransport m_eTransport;
                std::set<std::string> m_setInterfaces;


                static const std::array<std::string,6> STR_TRANSPORT;
        };
    };
};
