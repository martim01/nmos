#include "sender.h"

const std::string Sender::TRANSPORT[4] = {"urn:x-nmos:transport:rtp", "urn:x-nmos:transport:rtp.ucast", "urn:x-nmos:transport:rtp.mcast","urn:x-nmos:transport:dash"};
const std::string Sender::connection::STR_ACTIVATE[4] = {"", "activate_immediate", "activate_scheduled_absolute", "activate_scheduled_relative"};


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
    UpdateVersionTime();
}

void Sender::RemoveInterfaceBinding(std::string sInterface)
{
    m_setInterfaces.erase(sInterface);
    UpdateVersionTime();
}

void Sender::SetReceiverId(std::string sReceiverId, bool bActive)
{
    m_sReceiverId = sReceiverId;
    m_bReceiverActive = bActive;
    UpdateVersionTime();
}

bool Sender::Commit()
{
    if(Resource::Commit())
    {
        m_json["flow_id"] = m_sFlowId;
        m_json["device_id"] = m_sDeviceId;
        m_json["manifest_href"] = m_sManifest;
        m_json["transport"] = TRANSPORT[m_eTransport];

        m_json["interface_bindings"] = Json::Value(Json::arrayValue);
        for(std::set<std::string>::iterator itInterface = m_setInterfaces.begin(); itInterface != m_setInterfaces.end(); ++itInterface)
        {
            m_json["interface_bindings"].append((*itInterface));
        }

        m_json["subscription"] = Json::Value(Json::objectValue);
        if(m_sReceiverId.empty())
        {
             m_json["subscription"]["receiver_id"] = Json::nullValue;
        }
        else
        {
            m_json["subscription"]["receiver_id"] = m_sReceiverId;
        }

        if(m_bReceiverActive)
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


void Sender::SetTransport(enumTransport eTransport)
{
    m_eTransport = eTransport;
    UpdateVersionTime();
}

void Sender::SetManifestHref(std::string sHref)
{
    m_sManifest = sHref;
    UpdateVersionTime();
}


Json::Value Sender::GetConnectionStagedJson() const
{
    return GetConnectionJson(m_Staged);
}


Json::Value Sender::GetConnectionActiveJson() const
{
    return GetConnectionJson(m_Active);
}

Json::Value Sender::GetConnectionJson(const Sender::connection& con) const
{
    Json::Value jsConnect;
    if(con.sReceiverId.empty())
    {
        jsConnect["receiver_id"] = Json::nullValue;
    }
    else
    {
        jsConnect["receiver_id"] = con.sReceiverId;
    }
    jsConnect["activation"] = Json::objectValue;
    if(con.eActivate == connection::ACT_NULL)
    {
        jsConnect["activation"]["mode"] = Json::nullValue;
        jsConnect["activation"]["requested_time"] = Json::nullValue;

    }
    else
    {
        jsConnect["activation"]["mode"] = connection::STR_ACTIVATE[con.eActivate];
        if(con.eActivate == connection::ACT_NOW)
        {
            jsConnect["activation"]["requested_time"] = Json::nullValue;
        }
        else
        {
            jsConnect["activation"]["requested_time"] = con.sActivationTime;
        }
    }

    jsConnect["master_enable"] = con.bMasterEnable;
    jsConnect["transport_params"] = Json::arrayValue;
    jsConnect["transport_params"].append(con.tpSender.GetJson());
    return jsConnect;
}
