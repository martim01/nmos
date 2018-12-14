#include "sender.h"

const std::string Sender::TRANSPORT[4] = {"urn:x-nmos:transport:rtp", "urn:x-nmos:transport:rtp.ucast", "urn:x-nmos:transport:rtp.mcast","urn:x-nmos:transport:dash"};

Sender::Sender(std::string sLabel, std::string sDescription, std::string sFlowId, enumTransport eTransport, std::string sDeviceId, std::string sManifestHref) :
    Resource(sLabel, sDescription),
    m_sFlowId(sFlowId),
    m_eTransport(eTransport),
    m_sDeviceId(sDeviceId),
    m_sManifest(sManifestHref),
    m_sReceiverId(""),
    m_bReceiverActive(false)
{

}

void Sender::AddInterfaceBinding(std::string sInterface)
{
    m_setInterfaces.insert(sInterface);
}

void Sender::RemoveInterfaceBinding(std::string sInterface)
{
    m_setInterfaces.erase(sInterface);
}

void Sender::SetReceiverId(std::string sReceiverId, bool bActive)
{
    m_sReceiverId = sReceiverId;
    m_bReceiverActive = bActive;
}

Json::Value Sender::ToJson() const
{
    Json::Value jsSender(Resource::ToJson());

    jsSender["flow_id"] = m_sFlowId;
    jsSender["device_id"] = m_sDeviceId;
    jsSender["manifest_href"] = m_sManifest;
    jsSender["transport"] = TRANSPORT[m_eTransport];

    jsSender["interface_bindings"] = Json::Value(Json::arrayValue);
    for(std::set<std::string>::iterator itInterface = m_setInterfaces.begin(); itInterface != m_setInterfaces.end(); ++itInterface)
    {
        jsSender["interface_bindings"].append((*itInterface));
    }

    jsSender["subscription"] = Json::Value(Json::objectValue);
    jsSender["subscription"]["receiver_id"] = m_sReceiverId;
    if(m_bReceiverActive)
    {
        jsSender["subscription"]["active"] = true;
    }
    else
    {
        jsSender["subscription"]["active"] = false;
    }
    return jsSender;
}



