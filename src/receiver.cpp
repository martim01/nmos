#include "receiver.h"

const std::string Receiver::TRANSPORT[4] = {"urn:x-nmos:transport:rtp", "urn:x-nmos:transport:rtp.ucast", "urn:x-nmos:transport:rtp.mcast","urn:x-nmos:transport:dash"};
const std::string Receiver::TYPE[4] = {"urn:x-nmos:format:audio", "urn:x-nmos:format:video", "urn:x-nmos:format:data", "urn:x-nmos:format:mux"};

Receiver::Receiver(std::string sLabel, std::string sDescription, enumTransport eTransport, std::string sDeviceId, enumType eFormat) :
    Resource(sLabel, sDescription),
    m_eTransport(eTransport),
    m_sDeviceId(sDeviceId),
    m_sSenderId(""),
    m_bSenderActive(false),
    m_eType(eFormat)
{

}

void Receiver::AddInterfaceBinding(std::string sInterface)
{
    m_setInterfaces.insert(sInterface);
    UpdateVersionTime();
}

void Receiver::RemoveInterfaceBinding(std::string sInterface)
{
    m_setInterfaces.erase(sInterface);
    UpdateVersionTime();
}

void Receiver::AddCap(std::string sCap)
{
    m_setCaps.insert(sCap);
    UpdateVersionTime();
}

void Receiver::RemoveCap(std::string sCap)
{
    m_setCaps.erase(sCap);
    UpdateVersionTime();
}

void Receiver::SetSubscription(std::string sSenderId, bool bActive)
{
    m_sSenderId = sSenderId;
    m_bSenderActive = bActive;
    UpdateVersionTime();
}

void Receiver::SetTransport(enumTransport eTransport)
{
    m_eTransport = eTransport;
    UpdateVersionTime();
}

void Receiver::SetType(enumType eType)
{
    m_eType = eType;
    UpdateVersionTime();
}

bool Receiver::Commit()
{
    if(Resource::Commit())
    {

        m_json["device_id"] = m_sDeviceId;
        m_json["transport"] = TRANSPORT[m_eTransport];
        m_json["format"] = TYPE[m_eType];

        m_json["interface_bindings"] = Json::Value(Json::arrayValue);
        for(std::set<std::string>::iterator itInterface = m_setInterfaces.begin(); itInterface != m_setInterfaces.end(); ++itInterface)
        {
            m_json["interface_bindings"].append((*itInterface));
        }

        m_json["caps"] = Json::Value(Json::objectValue);
        m_json["caps"]["media_types"] = Json::Value(Json::arrayValue);
        for(std::set<std::string>::iterator itCap = m_setCaps.begin(); itCap != m_setCaps.end(); ++itCap)
        {
            m_json["caps"]["media_types"].append((*itCap));
        }


        m_json["subscription"] = Json::Value(Json::objectValue);

        if(m_sSenderId.empty())
        {
             m_json["subscription"]["sender_id"] = Json::nullValue;
        }
        else
        {
            m_json["subscription"]["sender_id"] = m_sSenderId;
        }

        if(m_bSenderActive)
        {
            m_json["subscription"]["active"] = true;
        }
        else
        {
            m_json["subscription"]["active"] = false;
        }
        return true;
    }
    return false;
}



