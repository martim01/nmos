#include "ioresource.h"

using namespace pml::nmos;

const std::array<std::string,6> IOResource::STR_TRANSPORT = {"urn:x-nmos:transport:rtp", "urn:x-nmos:transport:rtp.ucast", "urn:x-nmos:transport:rtp.mcast","urn:x-nmos:transport:dash", "urn:x-nmos:transport:mqtt", "urn:x-nmos:transport:websocket"};


void IOResource::AddInterfaceBinding(const std::string& sInterface)
{
    m_setInterfaces.insert(sInterface);
    UpdateVersionTime();
}

void IOResource::RemoveInterfaceBinding(const std::string& sInterface)
{
    m_setInterfaces.erase(sInterface);
    UpdateVersionTime();
}


void IOResource::SetTransport(IOResource::enumTransport eTransport)
{
     m_eTransport = eTransport;
     UpdateVersionTime();
}
