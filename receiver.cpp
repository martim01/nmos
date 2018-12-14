#include "receiver.h"

const std::string Receiver::TRANSPORT[4] = {"urn:x-nmos:transport:rtp", "urn:x-nmos:transport:rtp.ucast", "urn:x-nmos:transport:rtp.mcast","urn:x-nmos:transport:dash"};
const std::string Receiver::TYPE[4] = {"urn:x-nmos:format:audio", "urn:x-nmos:format:video", "urn:x-nmos:format:data", "urn:x-nmos:format:mux"};

Receiver::Receiver(std::string sLabel, std::string sDescription, enumTransport eTransport, std::string sDeviceId, enumType eFormat) :
    Resource(sLabel, sDescription),
    m_eTransport(eTransport),
    m_sDeviceId(sDeviceId),
    m_sSenderId("null"),
    m_bSenderActive(false),
    m_eType(eFormat)
{

}

void Receiver::AddInterfaceBinding(std::string sInterface)
{
    m_setInterfaces.insert(sInterface);
}

void Receiver::RemoveInterfaceBinding(std::string sInterface)
{
    m_setInterfaces.erase(sInterface);
}

void Receiver::AddCap(std::string sCap)
{
    m_setCaps.insert(sCap);
}

void Receiver::RemoveCap(std::string sCap)
{
    m_setCaps.erase(sCap);
}

void Receiver::SetSubscription(std::string sSenderId, bool bActive)
{
    m_sSenderId = sSenderId;
    m_bSenderActive = bActive;
}

Json::Value Receiver::ToJson() const
{
    Json::Value jsReceiver(Resource::ToJson());

    jsReceiver["device_id"] = m_sDeviceId;
    jsReceiver["transport"] = TRANSPORT[m_eTransport];
    jsReceiver["format"] = TYPE[m_eType];

    jsReceiver["interface_bindings"] = Json::Value(Json::arrayValue);
    for(std::set<std::string>::iterator itInterface = m_setInterfaces.begin(); itInterface != m_setInterfaces.end(); ++itInterface)
    {
        jsReceiver["interface_bindings"].append((*itInterface));
    }

    jsReceiver["caps"] = Json::Value(Json::objectValue);
    jsReceiver["caps"]["media_types"] = Json::Value(Json::arrayValue);
    for(std::set<std::string>::iterator itCap = m_setCaps.begin(); itCap != m_setCaps.end(); ++itCap)
    {
        jsReceiver["caps"]["media_types"].append((*itCap));
    }


    jsReceiver["subscription"] = Json::Value(Json::objectValue);
    jsReceiver["subscription"]["sender_id"] = m_sSenderId;
    if(m_bSenderActive)
    {
        jsReceiver["subscription"]["active"] = true;
    }
    else
    {
        jsReceiver["subscription"]["active"] = false;
    }
    return jsReceiver;
}



