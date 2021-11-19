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

using namespace pml::nmos;



SenderBase::SenderBase(const std::string& sLabel, const std::string& sDescription, const std::string& sFlowId, enumTransport eTransport, const std::string& sDeviceId, const std::string& sInterface, TransportParamsRTP::flagsTP flagsTransport) :
    IOResource("sender", sLabel, sDescription, eTransport),
    m_sFlowId(sFlowId),
    m_sDeviceId(sDeviceId),
    m_sReceiverId(""),
    m_bReceiverActive(false),
    m_vConstraints(1),
    m_bActivateAllowed(false)
{
    AddInterfaceBinding(sInterface);

    if(flagsTransport & TransportParamsRTP::REDUNDANT)
    {
        m_vConstraints.resize(2);
        m_Staged.tpSenders.resize(2);
        m_Active.tpSenders.resize(2);
    }

    for(size_t i = 0; i < m_vConstraints.size(); i++)
    {
        if(flagsTransport & TransportParamsRTP::FEC)
        {
            m_Staged.tpSenders[i].eFec = TransportParamsRTP::TP_SUPPORTED;
            m_Active.tpSenders[i].eFec = TransportParamsRTP::TP_SUPPORTED;
        }
        else
        {
            m_Staged.tpSenders[i].eFec = TransportParamsRTP::TP_NOT_SUPPORTED;
            m_Active.tpSenders[i].eFec = TransportParamsRTP::TP_NOT_SUPPORTED;
        }

        if(flagsTransport & TransportParamsRTP::RTCP)
        {
            m_Staged.tpSenders[i].eRTCP = TransportParamsRTP::TP_SUPPORTED;
            m_Active.tpSenders[i].eRTCP = TransportParamsRTP::TP_SUPPORTED;
        }
        else
        {
            m_Staged.tpSenders[i].eRTCP = TransportParamsRTP::TP_NOT_SUPPORTED;
            m_Active.tpSenders[i].eRTCP = TransportParamsRTP::TP_NOT_SUPPORTED;
        }
        if(flagsTransport & TransportParamsRTP::MULTICAST)
        {
            m_Staged.tpSenders[i].eMulticast = TransportParamsRTP::TP_SUPPORTED;
            m_Active.tpSenders[i].eMulticast = TransportParamsRTP::TP_SUPPORTED;
        }
        else
        {
            m_Staged.tpSenders[i].eMulticast = TransportParamsRTP::TP_NOT_SUPPORTED;
            m_Active.tpSenders[i].eMulticast = TransportParamsRTP::TP_NOT_SUPPORTED;
        }

        m_vConstraints[i].nParamsSupported = flagsTransport;

    }

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
    if(jsData["manifest_href"].isString() == false  && JsonMemberExistsAndIsNotNull(jsData, "manifest_href"))
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

        m_eTransport = enumTransport::UNKNOWN_TRANSPORT;
        for(int i = 0; i < STR_TRANSPORT.size(); i++)
        {
            if(jsData["transport"].asString().find(STR_TRANSPORT[i]) != std::string::npos)
            {
                m_eTransport = (enumTransport)i;
                break;
            }
        }

        if(m_eTransport == enumTransport::UNKNOWN_TRANSPORT)
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


void SenderBase::SetReceiverId(const std::string& sReceiverId, bool bActive)
{
    m_sReceiverId = sReceiverId;
    m_bReceiverActive = bActive;

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
    for(const auto& con :m_vConstraints)
    {
        jsArray.append(con.GetJson(version));
    }
    return jsArray;
}



bool SenderBase::CheckConstraints(const connectionSender& conRequest)
{
    bool bMeets = (m_vConstraints.size() == conRequest.tpSenders.size());
    if(bMeets)
    {
        for(size_t i = 0; i < m_vConstraints.size(); i++)
        {
            bMeets &= m_vConstraints[i].destination_port.MeetsConstraint(conRequest.tpSenders[i].nDestinationPort);
            bMeets &= m_vConstraints[i].fec_destination_ip.MeetsConstraint(conRequest.tpSenders[i].sFecDestinationIp);
            bMeets &= m_vConstraints[i].fec_enabled.MeetsConstraint(conRequest.tpSenders[i].bFecEnabled);
            bMeets &= m_vConstraints[i].fec_mode.MeetsConstraint(TransportParamsRTP::STR_FEC_MODE[conRequest.tpSenders[i].eFecMode]);
            bMeets &= m_vConstraints[i].fec1D_destination_port.MeetsConstraint(conRequest.tpSenders[i].nFec1DDestinationPort);
            bMeets &= m_vConstraints[i].fec2D_destination_port.MeetsConstraint(conRequest.tpSenders[i].nFec2DDestinationPort);
            bMeets &= m_vConstraints[i].rtcp_destination_ip.MeetsConstraint(conRequest.tpSenders[i].sRtcpDestinationIp);
            bMeets &= m_vConstraints[i].rtcp_destination_port.MeetsConstraint(conRequest.tpSenders[i].nRtcpDestinationPort);

            bMeets &= m_vConstraints[i].destination_ip.MeetsConstraint(conRequest.tpSenders[i].sDestinationIp);
            bMeets &= m_vConstraints[i].source_ip.MeetsConstraint(conRequest.tpSenders[i].sSourceIp);
            bMeets &= m_vConstraints[i].source_port.MeetsConstraint(conRequest.tpSenders[i].nSourcePort);
            bMeets &= m_vConstraints[i].fec_type.MeetsConstraint(TransportParamsRTPSender::STR_FEC_TYPE[conRequest.tpSenders[i].eFecType]);
            bMeets &= m_vConstraints[i].fec_block_width.MeetsConstraint(conRequest.tpSenders[i].nFecBlockWidth);
            bMeets &= m_vConstraints[i].fec_block_height.MeetsConstraint(conRequest.tpSenders[i].nFecBlockHeight);
            bMeets &= m_vConstraints[i].fec1D_source_port.MeetsConstraint(conRequest.tpSenders[i].nFec1DSourcePort);
            bMeets &= m_vConstraints[i].fec2D_source_port.MeetsConstraint(conRequest.tpSenders[i].nFec2DSourcePort);
            bMeets &= m_vConstraints[i].rtcp_enabled.MeetsConstraint(conRequest.tpSenders[i].bRtcpEnabled);
            bMeets &= m_vConstraints[i].rtcp_source_port.MeetsConstraint(conRequest.tpSenders[i].nRtcpSourcePort);
            bMeets &= m_vConstraints[i].rtp_enabled.MeetsConstraint(conRequest.tpSenders[i].bRtpEnabled);
        }
    }
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
