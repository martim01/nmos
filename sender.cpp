#include "sender.h"
#include "eventposter.h"

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


Sender::Sender(const Json::Value& jsData) : Resource(jsData)
{
    m_bIsOk = m_bIsOk && (m_json["flow_id"].isString() && m_json["device_id"].isString() && m_json["manifest_href"].isString() &&  m_json["transport"].isString()
                   && m_json["interface_bindings"].isArray() && m_json["subscription"].isObject() && m_json["subscription"]["receiver_id"].isString() && m_json["subscription"]["active"].isBool());

    if(m_bIsOk)
    {
        m_sFlowId = m_json["flow_id"].asString();
        m_sDeviceId = m_json["device_id"].asString();
        m_sManifest = m_json["manifest_href"].asString();

        if(m_json["transport"].asString() == TRANSPORT[RTP])
        {
            m_eTransport = RTP;
        }
        else if(m_json["transport"].asString() == TRANSPORT[RTP_UCAST])
        {
            m_eTransport = RTP_UCAST;
        }
        else if(m_json["transport"].asString() == TRANSPORT[RTP_MCAST])
        {
            m_eTransport = RTP_MCAST;
        }
        else if(m_json["transport"].asString() == TRANSPORT[DASH])
        {
            m_eTransport = DASH;
        }
        else
        {
            m_bIsOk = false;
        }
        for(Json::ArrayIndex n = 0; n < m_json["interface_bindings"].size(); n++)
        {
            if(m_json["interface_bindings"][n].isString())
            {
                m_setInterfaces.insert(m_json["interface_bindings"][n].asString());
            }
            else
            {
                m_bIsOk = false;
                break;
            }
        }

        m_sReceiverId = m_json["subscription"]["receiver_id"].asString();
        m_bReceiverActive = m_json["subscription"]["active"].asBool();
    }
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
    return m_Staged.GetJson();
}


Json::Value Sender::GetConnectionActiveJson() const
{
    return m_Active.GetJson();
}




Json::Value Sender::GetConnectionConstraintsJson() const
{
    Json::Value jsArray(Json::arrayValue);
    jsArray.append(m_constraints.GetJson());
    return jsArray;
}



bool Sender::CheckConstraints(const connectionSender& conRequest)
{
    bool bMeets = m_constraints.destination_port.MeetsConstraint(conRequest.tpSender.nDestinationPort);
    bMeets &= m_constraints.fec_destination_ip.MeetsConstraint(conRequest.tpSender.sFecDestinationIp);
    bMeets &= m_constraints.fec_enabled.MeetsConstraint(conRequest.tpSender.bFecEnabled);
    bMeets &= m_constraints.fec_mode.MeetsConstraint(TransportParamsRTP::STR_FEC_MODE[conRequest.tpSender.eFecMode]);
    bMeets &= m_constraints.fec1D_destination_port.MeetsConstraint(conRequest.tpSender.nFec1DDestinationPort);
    bMeets &= m_constraints.fec2D_destination_port.MeetsConstraint(conRequest.tpSender.nFec2DDestinationPort);
    bMeets &= m_constraints.rtcp_destination_ip.MeetsConstraint(conRequest.tpSender.sRtcpDestinationIp);
    bMeets &= m_constraints.rtcp_destination_port.MeetsConstraint(conRequest.tpSender.nRtcpDestinationPort);

    bMeets &= m_constraints.destination_ip.MeetsConstraint(conRequest.tpSender.sDestinationIp);
    bMeets &= m_constraints.source_ip.MeetsConstraint(conRequest.tpSender.sSourceIp);
    bMeets &= m_constraints.source_port.MeetsConstraint(conRequest.tpSender.nSourcePort);
    bMeets &= m_constraints.fec_type.MeetsConstraint(TransportParamsRTPSender::STR_FEC_TYPE[conRequest.tpSender.eFecType]);
    bMeets &= m_constraints.fec_block_width.MeetsConstraint(conRequest.tpSender.nFecBlockWidth);
    bMeets &= m_constraints.fec_block_height.MeetsConstraint(conRequest.tpSender.nFecBlockHeight);
    bMeets &= m_constraints.fec1D_source_port.MeetsConstraint(conRequest.tpSender.nFec1DSourcePort);
    bMeets &= m_constraints.fec2D_source_port.MeetsConstraint(conRequest.tpSender.nFec2DSourcePort);
    bMeets &= m_constraints.rtcp_enabled.MeetsConstraint(conRequest.tpSender.bRtcpEnabled);
    bMeets &= m_constraints.rtcp_source_port.MeetsConstraint(conRequest.tpSender.nRtcpSourcePort);
    bMeets &= m_constraints.rtp_enabled.MeetsConstraint(conRequest.tpSender.bRtpEnabled);
    return bMeets;
}

bool Sender::IsLocked()
{
    return (m_Staged.eActivate == connection::ACT_ABSOLUTE || m_Staged.eActivate == connection::ACT_RELATIVE);
}

void Sender::Stage(const connectionSender& conRequest)
{
    m_Staged = conRequest;

    switch(m_Staged.eActivate)
    {
        case connection::ACT_NOW:
            m_Staged.sActivationTime = GetCurrentTime();
            // @todo Tell the main thread to do it's stuff
            break;
        case connection::ACT_ABSOLUTE:
            // @todo start a thread that will sleep until the given time and then tell the main thread to do it's stuff
            break;
        case connection::ACT_RELATIVE:
            // @todo start a thread that will sleep for the given time period and then tell the main thread to do its stuff
            break;
    }
}

const connectionSender& Sender::GetStaged()
{
    return m_Staged;
}
