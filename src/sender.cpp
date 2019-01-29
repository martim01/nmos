#include "sender.h"
#include "eventposter.h"
#include <thread>
#include "sdp.h"
#include "nodeapi.h"
#include "flow.h"

static void ActivationThreadSender(const std::chrono::time_point<std::chrono::high_resolution_clock>& tp, const std::string& sSenderId, std::shared_ptr<EventPoster> pPoster)
{
    std::this_thread::sleep_until(tp);

    if(pPoster)
    {
        pPoster->_ActivateSender(sSenderId);
    }
}

const std::string Sender::STR_TRANSPORT[4] = {"urn:x-nmos:transport:rtp", "urn:x-nmos:transport:rtp.ucast", "urn:x-nmos:transport:rtp.mcast","urn:x-nmos:transport:dash"};


Sender::Sender(std::string sLabel, std::string sDescription, std::string sFlowId, enumTransport eTransport, std::string sDeviceId, std::string sInterface, TransportParamsRTP::flagsTP flagsTransport) :
    Resource("sender", sLabel, sDescription),
    m_sFlowId(sFlowId),
    m_eTransport(eTransport),
    m_sDeviceId(sDeviceId),
    m_sReceiverId(""),
    m_bReceiverActive(false)
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

    //activate the
    Activate("","","");

}

Sender::Sender() : Resource("sender")
{

}

bool Sender::UpdateFromJson(const Json::Value& jsData)
{
    Resource::UpdateFromJson(jsData);
    if(jsData["flow_id"].isString() == false && jsData["flow_id"].isNull() == false)
    {
        m_bIsOk = false;
        m_ssJsonError << "'flow_id' neither a string or null" << std::endl;
    }
    if(jsData["device_id"].isString() == false)
    {
        m_bIsOk = false;
        m_ssJsonError << "'device_id' is not a string" << std::endl;
    }
    if(jsData["manifest_href"].isString() == false)
    {
        m_bIsOk = false;
        m_ssJsonError << "'manifest_href' is not a string" << std::endl;
    }
    if(jsData["transport"].isString() == false)
    {
        m_bIsOk = false;
        m_ssJsonError << "'transport' is not a string" << std::endl;
    }
    if(jsData["interface_bindings"].isArray() == false)
    {
        m_bIsOk = false;
        m_ssJsonError << "'interface_bindings' is not an array" << std::endl;
    }
    if(jsData["subscription"].isObject() == false)
    {
        m_bIsOk = false;
        m_ssJsonError << "'subscription' is not an object" << std::endl;
    }
    if(jsData["subscription"]["receiver_id"].isString() ==false && jsData["subscription"]["receiver_id"].isNull() == false)
    {
        m_bIsOk = false;
        m_ssJsonError << "'subscription' 'receiver_id' is not a string and not null" << std::endl;
    }
    if(jsData["subscription"]["active"].isBool() == false)
    {
        m_bIsOk = false;
        m_ssJsonError << "'subscription' 'active' is not a bool" << std::endl;
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
            m_ssJsonError << "'transport' " <<jsData["transport"].asString() <<" incorrect" << std::endl;
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
                m_ssJsonError << "'interface_bindings' #" << n <<" not a string" << std::endl;
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

bool Sender::Commit(const ApiVersion& version)
{
    if(Resource::Commit(version))
    {
        if(m_sFlowId.empty() == false)
        {
            m_json["flow_id"] = m_sFlowId;
        }
        else
        {
            m_json["flow_id"] = Json::nullValue;
        }
        m_json["device_id"] = m_sDeviceId;
        m_json["manifest_href"] = m_sManifest;
        m_json["transport"] = STR_TRANSPORT[m_eTransport];

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
        CreateSDP();
        return true;
    }
    return false;
}


void Sender::SetTransport(enumTransport eTransport)
{
    m_eTransport = eTransport;

}



Json::Value Sender::GetConnectionStagedJson(const ApiVersion& version) const
{
    return m_Staged.GetJson(version);
}


Json::Value Sender::GetConnectionActiveJson(const ApiVersion& version) const
{
    return m_Active.GetJson(version);
}




Json::Value Sender::GetConnectionConstraintsJson(const ApiVersion& version) const
{
    Json::Value jsArray(Json::arrayValue);
    jsArray.append(m_constraints.GetJson(version));
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
    std::lock_guard<std::mutex> lg(m_mutex);
    return (m_Staged.eActivate == connection::ACT_ABSOLUTE || m_Staged.eActivate == connection::ACT_RELATIVE);
}

bool Sender::Stage(const connectionSender& conRequest, std::shared_ptr<EventPoster> pPoster)
{
    bool bOk(true);

    {
        std::lock_guard<std::mutex> lg(m_mutex);
        m_Staged = conRequest;
    }

    switch(m_Staged.eActivate)
    {
        case connection::ACT_NULL:
            m_Staged.sActivationTime = GetCurrentTime();
            // @todo Cancel any activations that were going to happen
            break;
        case connection::ACT_NOW:
            m_Staged.sActivationTime = GetCurrentTime();
            // Tell the main thread to do it's stuff
            if(pPoster)
            {
                pPoster->_ActivateSender(GetId());
            }
            else
            {
                // @todo no poster so do the activation bits ourself
            }
            break;
        case connection::ACT_ABSOLUTE:
            if(pPoster)
            {
                std::chrono::time_point<std::chrono::high_resolution_clock> tp;
                if(ConvertTaiStringToTimePoint(m_Staged.sRequestedTime, tp))
                {
                    std::thread thActive(ActivationThreadSender, tp, GetId(), pPoster);
                    thActive.detach();
                }
                else
                {
                    bOk = false;
                }
            }
            else
            {
                //nothing to do here -
                bOk = false;
            }
            break;
        case connection::ACT_RELATIVE:
            // start a thread that will sleep for the given time period and then tell the main thread to do its stuff
            if(pPoster)
            {
                std::chrono::time_point<std::chrono::high_resolution_clock> tp(std::chrono::high_resolution_clock::now().time_since_epoch());

                try
                {
                    std::chrono::nanoseconds nano(stoul(m_Staged.sActivationTime));
                    std::thread thActive(ActivationThreadSender, (tp+nano), GetId(), pPoster);
                    thActive.detach();
                }
                catch(std::invalid_argument& ia)
                {
                    bOk = false;
                }
            }
            else
            {
                //nothing to do here
                bOk = false;
            }
            break;
    }
    return bOk;
}

connectionSender Sender::GetStaged()
{
    std::lock_guard<std::mutex> lg(m_mutex);
    return m_Staged;
}

connectionSender Sender::GetActive()
{
    std::lock_guard<std::mutex> lg(m_mutex);
    return m_Active;
}


void Sender::Activate(std::string sSourceIp, std::string sDestinationIp, std::string sSDP)
{
    //move the staged parameters to active parameters
    m_Active = m_Staged;

    if(sSourceIp.empty())
    {
        //get the bound to interface source address
        for(std::set<std::string>::iterator itInteface = m_setInterfaces.begin(); itInteface != m_setInterfaces.end(); ++itInteface)
        {
            std::map<std::string, nodeinterface>::const_iterator itDetails = NodeApi::Get().GetSelf().FindInterface((*itInteface));
            if(itDetails != NodeApi::Get().GetSelf().GetInterfaceEnd())
            {
                sSourceIp = itDetails->second.sMainIpAddress;
                break;
            }
        }
    }

    //Change auto settings to what they actually are
    m_Active.tpSender.Actualize(sSourceIp, sDestinationIp);


    // create the SDP
    if(m_Active.bMasterEnable)
    {
        if(sSDP.empty())
        {
            CreateSDP();
        }
        else
        {
            m_sTransportFile = sSDP;
        }
    }
    else
    {
        m_sTransportFile.clear();
    }
    //@todo Set the flow to be whatever the flow is...

    //activate - set subscription, receiverId and active on master_enable.
    m_sReceiverId = m_Staged.sReceiverId;
    m_bReceiverActive = m_Staged.bMasterEnable;

    //reset the staged activation
    m_Staged.eActivate = connection::ACT_NULL;
    m_Staged.sActivationTime.clear();
    m_Staged.sRequestedTime.clear();
}

void Sender::SetDestinationDetails(const std::string& sDestinationIp, unsigned short nDestinationPort)
{
    m_Active.tpSender.sDestinationIp = sDestinationIp;
    m_Active.tpSender.nDestinationPort = nDestinationPort;
    CreateSDP();
}

void Sender::CreateSDP()
{
    std::stringstream ssSDP;
    ssSDP << "v=0\r\n";
    ssSDP << "o=- " << GetCurrentTime(false) << " " << GetCurrentTime(false) << " IN IP";
    switch(SdpManager::CheckIpAddress(m_Active.tpSender.sSourceIp))
    {
        case SdpManager::IP4_UNI:
        case SdpManager::IP4_MULTI:
            ssSDP << "4 ";
            break;
        case SdpManager::IP6_UNI:
        case SdpManager::IP6_MULTI:
            ssSDP << "6 ";
            break;
        case SdpManager::IP_INVALID:
            ssSDP << " ";
            break;
    }
    ssSDP << m_Active.tpSender.sSourceIp << "\r\n";    // @todo should check here if
    ssSDP << "t=0 0 \r\n";

    std::map<std::string, std::shared_ptr<Resource> >::const_iterator itDevice = NodeApi::Get().GetDevices().FindNmosResource(m_sDeviceId);
    if(itDevice != NodeApi::Get().GetDevices().GetResourceEnd())
    {
        ssSDP << "s=" << itDevice->second->GetLabel() << ":";
    }
    else
    {
        ssSDP << "s=-:";
    }
    ssSDP << GetLabel() << "\r\n";


    //put in the destination unicast/multicast block
    switch(SdpManager::CheckIpAddress(m_Active.tpSender.sDestinationIp))
    {
        case SdpManager::IP4_UNI:
            ssSDP << "c=IN IP4 " << m_Active.tpSender.sDestinationIp << "\r\n";
            ssSDP << "a=type:unicast\r\n";
            break;
        case SdpManager::IP4_MULTI:
            ssSDP << "c=IN IP4 " << m_Active.tpSender.sDestinationIp << "/32\r\n";
            ssSDP << "a=source-filter: incl IN IP4 " << m_Active.tpSender.sDestinationIp << " " << m_Active.tpSender.sSourceIp << "\r\n";
            ssSDP << "a=type:multicast\r\n";
            break;
        case SdpManager::IP6_UNI:
            ssSDP << "c=IN IP6 " << m_Active.tpSender.sDestinationIp << "\r\n";
            ssSDP << "a=type:unicast\r\n";
            break;
        case SdpManager::IP6_MULTI:
            ssSDP << "c=IN IP6 " << m_Active.tpSender.sDestinationIp << "\r\n";
            ssSDP << "a=source-filter: incl IN IP6 " << m_Active.tpSender.sDestinationIp << " " << m_Active.tpSender.sSourceIp << "\r\n";
            ssSDP << "a=type:multicast\r\n";
            break;
        case SdpManager::SdpManager::IP_INVALID:
            break;
    }


    //now put in the flow media information
    std::map<std::string, std::shared_ptr<Resource> >::const_iterator itFlow = NodeApi::Get().GetFlows().FindNmosResource(m_sFlowId);
    if(itFlow != NodeApi::Get().GetFlows().GetResourceEnd())
    {
        std::shared_ptr<Flow> pFlow = std::dynamic_pointer_cast<Flow>(itFlow->second);
        if(pFlow)
        {
            unsigned short nPort(m_Active.tpSender.nDestinationPort);
            if(nPort == 0)
            {
                nPort = 5004;
            }
            ssSDP << pFlow->CreateSDPLines(5004);
        }
    }

    //clock information is probably at the media level
    if(m_setInterfaces.empty())
    {
        ssSDP << NodeApi::Get().GetSelf().CreateClockSdp("");
    }
    else
    {
        ssSDP << NodeApi::Get().GetSelf().CreateClockSdp(*m_setInterfaces.begin()); // @todo should we check all the intefaces for the clock mac address??
    }

    //now put in the RTCP info if we've got any
    if(m_Active.tpSender.bRtcpEnabled)
    {
        switch(SdpManager::CheckIpAddress(m_Active.tpSender.sRtcpDestinationIp))
        {
            case SdpManager::IP4_UNI:
            case SdpManager::IP4_MULTI:
                ssSDP << "a=rtcp:" << m_Active.tpSender.nRtcpDestinationPort << " IN IP4 " << m_Active.tpSender.sRtcpDestinationIp << "\r\n";
                break;
            case SdpManager::IP6_UNI:
            case SdpManager::IP6_MULTI:
                ssSDP << "a=rtcp:" << m_Active.tpSender.nRtcpDestinationPort << " IN IP6 " << m_Active.tpSender.sRtcpDestinationIp << "\r\n";
                break;
            default:
                break;
        }
    }
    m_sTransportFile = ssSDP.str();
}


const std::string& Sender::GetTransportFile() const
{
    return m_sTransportFile;
}


void Sender::SetManifestHref(std::string sHref)
{
    m_sManifest = sHref;
}


const std::string& Sender::GetManifestHref() const
{
    return m_sManifest;
}
