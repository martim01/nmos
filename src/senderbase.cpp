#include "senderbase.h"
#include "eventposter.h"
#include <thread>

#include "flow.h"
#include "device.h"
#include "utils.h"
#include "log.h"
#include <algorithm>
#include <vector>
#include "source.h"

const std::string SenderBase::STR_TRANSPORT[4] = {"urn:x-nmos:transport:rtp", "urn:x-nmos:transport:rtp.ucast", "urn:x-nmos:transport:rtp.mcast","urn:x-nmos:transport:dash"};


SenderBase::SenderBase(const std::string& sLabel, const std::string& sDescription, const std::string& sFlowId, enumTransport eTransport, const std::string& sDeviceId, const std::string& sInterface, TransportParamsRTP::flagsTP flagsTransport) :
    IOResource("sender", sLabel, sDescription),
    m_sFlowId(sFlowId),
    m_eTransport(eTransport),
    m_sDeviceId(sDeviceId),
    m_sReceiverId(""),
    m_bReceiverActive(false),
    m_bActivateAllowed(false)
{
    AddInterfaceBinding(sInterface);

    if(flagsTransport & TransportParamsRTP::FEC)
    {
        m_Staged.tpSender.eFec = TransportParamsRTP::TP_SUPPORTED;
        m_Active.tpSender.eFec = TransportParamsRTP::TP_SUPPORTED;
    }
    else
    {
        m_Staged.tpSender.eFec = TransportParamsRTP::TP_NOT_SUPPORTED;
        m_Active.tpSender.eFec = TransportParamsRTP::TP_NOT_SUPPORTED;
    }

    if(flagsTransport & TransportParamsRTP::RTCP)
    {
        m_Staged.tpSender.eRTCP = TransportParamsRTP::TP_SUPPORTED;
        m_Active.tpSender.eRTCP = TransportParamsRTP::TP_SUPPORTED;
    }
    else
    {
        m_Staged.tpSender.eRTCP = TransportParamsRTP::TP_NOT_SUPPORTED;
        m_Active.tpSender.eRTCP = TransportParamsRTP::TP_NOT_SUPPORTED;
    }
    if(flagsTransport & TransportParamsRTP::MULTICAST)
    {
        m_Staged.tpSender.eMulticast = TransportParamsRTP::TP_SUPPORTED;
        m_Active.tpSender.eMulticast = TransportParamsRTP::TP_SUPPORTED;
    }
    else
    {
        m_Staged.tpSender.eMulticast = TransportParamsRTP::TP_NOT_SUPPORTED;
        m_Active.tpSender.eMulticast = TransportParamsRTP::TP_NOT_SUPPORTED;
    }

    m_constraints.nParamsSupported = flagsTransport;



}

SenderBase::SenderBase() : IOResource("sender")
{

}

SenderBase::~SenderBase()
{

}

bool SenderBase::UpdateFromJson(const Json::Value& jsData)
{
    Resource::UpdateFromJson(jsData);
    if(jsData["flow_id"].isString() == false && JsonMemberExistsAndIsNotNull(jsData, "flow_id"))
    {
        m_bIsOk = false;
        m_ssJsonError << "'flow_id' neither a string or null" ;
    }
    if(jsData["device_id"].isString() == false)
    {
        m_bIsOk = false;
        m_ssJsonError << "'device_id' is not a string" ;
    }
    if(jsData["manifest_href"].isString() == false)
    {
        m_bIsOk = false;
        m_ssJsonError << "'manifest_href' is not a string" ;
    }
    if(jsData["transport"].isString() == false)
    {
        m_bIsOk = false;
        m_ssJsonError << "'transport' is not a string" ;
    }
    if(jsData["interface_bindings"].isArray() == false)
    {
        m_bIsOk = false;
        m_ssJsonError << "'interface_bindings' is not an array" ;
    }
    if(jsData["subscription"].isObject() == false)
    {
        m_bIsOk = false;
        m_ssJsonError << "'subscription' is not an object" ;
    }
    if(jsData["subscription"]["receiver_id"].isString() ==false && JsonMemberExistsAndIsNotNull(jsData["subscription"], "receiver_id"))
    {
        m_bIsOk = false;
        m_ssJsonError << "'subscription' 'receiver_id' is not a string and not null" ;
    }
    if(jsData["subscription"]["active"].isBool() == false)
    {
        m_bIsOk = false;
        m_ssJsonError << "'subscription' 'active' is not a bool" ;
    }

    if(m_bIsOk)
    {
        if(jsData["flow_id"].isString())
        {
            m_sFlowId = jsData["flow_id"].asString();
        }
        m_sDeviceId = jsData["device_id"].asString();
        m_sManifest = jsData["manifest_href"].asString();


        if(jsData["transport"].asString().find(STR_TRANSPORT[RTP_MCAST]) != std::string::npos)
        {
            m_eTransport = RTP_MCAST;
        }
        else if(jsData["transport"].asString().find(STR_TRANSPORT[RTP_UCAST]) != std::string::npos)
        {
            m_eTransport = RTP_UCAST;
        }

        else if(jsData["transport"].asString().find(STR_TRANSPORT[RTP]) != std::string::npos)
        {
            m_eTransport = RTP;
        }
        else if(jsData["transport"].asString().find(STR_TRANSPORT[DASH]) != std::string::npos)
        {
            m_eTransport = DASH;
        }
        else
        {
            m_bIsOk = false;
            m_ssJsonError << "'transport' " <<jsData["transport"].asString() <<" incorrect" ;
        }
        for(Json::ArrayIndex n = 0; n < jsData["interface_bindings"].size(); n++)
        {
            if(jsData["interface_bindings"][n].isString())
            {
                m_setInterfaces.insert(jsData["interface_bindings"][n].asString());
            }
            else
            {
                m_bIsOk = false;
                m_ssJsonError << "'interface_bindings' #" << n <<" not a string" ;
                break;
            }
        }

        if(jsData["subscription"]["receiver_id"].isString())
        {
            m_sReceiverId = jsData["subscription"]["receiver_id"].asString();
        }
        m_bReceiverActive = jsData["subscription"]["active"].asBool();
    }
    return m_bIsOk;
}

void SenderBase::AddInterfaceBinding(const std::string& sInterface)
{
    m_setInterfaces.insert(sInterface);
}

void SenderBase::RemoveInterfaceBinding(const std::string& sInterface)
{
    m_setInterfaces.erase(sInterface);
}

void SenderBase::SetReceiverId(const std::string& sReceiverId, bool bActive)
{
    m_sReceiverId = sReceiverId;
    m_bReceiverActive = bActive;

}



void SenderBase::SetTransport(enumTransport eTransport)
{
    m_eTransport = eTransport;

}



Json::Value SenderBase::GetConnectionStagedJson(const ApiVersion& version) const
{
    return m_Staged.GetJson(version);
}


Json::Value SenderBase::GetConnectionActiveJson(const ApiVersion& version) const
{
    return m_Active.GetJson(version);
}




Json::Value SenderBase::GetConnectionConstraintsJson(const ApiVersion& version) const
{
    Json::Value jsArray(Json::arrayValue);
    jsArray.append(m_constraints.GetJson(version));
    return jsArray;
}



bool SenderBase::CheckConstraints(const connectionSender& conRequest)
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

bool SenderBase::IsLocked()
{
    std::lock_guard<std::mutex> lg(m_mutex);
    return (m_Staged.eActivate == connection::ACT_ABSOLUTE || m_Staged.eActivate == connection::ACT_RELATIVE);
}


connectionSender SenderBase::GetStaged()
{
    std::lock_guard<std::mutex> lg(m_mutex);
    return m_Staged;
}

connectionSender SenderBase::GetActive()
{
    std::lock_guard<std::mutex> lg(m_mutex);
    return m_Active;
}




const std::string& SenderBase::GetTransportFile() const
{
    return m_sTransportFile;
}


void SenderBase::SetManifestHref(const std::string& sHref)
{
    m_sManifest = sHref;
}


const std::string& SenderBase::GetManifestHref() const
{
    return m_sManifest;
}


bool SenderBase::IsActivateAllowed() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_bActivateAllowed;
}
