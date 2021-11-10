#include "sender.h"
#include "nodeapi.h"
#include "utils.h"
#include "log.h"
#include "activator.h"
#include "sdp.h"
#include "device.h"
#include "flow.h"
#include "source.h"


Sender::Sender(const std::string& sLabel, const std::string& sDescription, const std::string& sFlowId, enumTransport eTransport, const std::string& sDeviceId, const std::string& sInterface, TransportParamsRTP::flagsTP flagsTransport) :
   SenderBase(sLabel, sDescription, sFlowId, eTransport, sDeviceId, sInterface, flagsTransport)
{
    SetupActivation("", "239.220.1.1","");  // @todo choose some defaults - get the ori
    Activate();
}

Sender::Sender() : SenderBase()
{
}

void Sender::CreateSDP()
{
    CreateSDP(m_Active);
}

void Sender::CreateSDP(const connectionSender& state)
{
    std::stringstream ssSDP;
    ssSDP << "v=0\r\n";
    ssSDP << "o=- " << GetCurrentTaiTime(false) << " " << GetCurrentTaiTime(false) << " IN IP";
    switch(SdpManager::CheckIpAddress(state.tpSender.sSourceIp))
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
    ssSDP << state.tpSender.sSourceIp << "\r\n";    // @todo should check here if sSourceIp is not set to auto
    ssSDP << "t=0 0 \r\n";

    std::map<std::string, std::shared_ptr<Device> >::const_iterator itDevice = NodeApi::Get().GetDevices().FindNmosResource(m_sDeviceId);
    if(itDevice != NodeApi::Get().GetDevices().GetResourceEnd())
    {
        ssSDP << "s=" << itDevice->second->GetLabel() << ":";
    }
    else
    {
        ssSDP << "s=-:";
    }
    ssSDP << GetLabel() << "\r\n";


    std::stringstream ssc;
    //put in the destination unicast/multicast block
    switch(SdpManager::CheckIpAddress(state.tpSender.sDestinationIp))
    {
        case SdpManager::IP4_UNI:
            ssc << "c=IN IP4 " << state.tpSender.sDestinationIp << "\r\n";
            ssSDP << ssc.str();
            ssSDP << "a=type:unicast\r\n";
            break;
        case SdpManager::IP4_MULTI:
            ssc << "c=IN IP4 " << state.tpSender.sDestinationIp << "/32\r\n";
            ssSDP << ssc.str();
            ssSDP << "a=source-filter:incl IN IP4 " << state.tpSender.sDestinationIp << " " << state.tpSender.sSourceIp << "\r\n";
            ssSDP << "a=type:multicast\r\n";
            break;
        case SdpManager::IP6_UNI:
            ssc << "c=IN IP6 " << state.tpSender.sDestinationIp << "\r\n";
            ssSDP << ssc.str();
            ssSDP << "a=type:unicast\r\n";
            break;
        case SdpManager::IP6_MULTI:
            ssc << "c=IN IP6 " << state.tpSender.sDestinationIp << "\r\n";
            ssSDP << ssc.str();
            ssSDP << "a=source-filter:incl IN IP6 " << state.tpSender.sDestinationIp << " " << state.tpSender.sSourceIp << "\r\n";
            ssSDP << "a=type:multicast\r\n";
            break;
        case SdpManager::SdpManager::IP_INVALID:
            pmlLog(pml::LOG_WARN) << "NMOS: Sender can't create SDP - destination IP invalid '" << state.tpSender.sDestinationIp << "'";
            break;
    }


    //now put in the flow media information
    auto itFlow = NodeApi::Get().GetFlows().FindNmosResource(m_sFlowId);
    if(itFlow != NodeApi::Get().GetFlows().GetResourceEnd())
    {
        auto pFlow = std::dynamic_pointer_cast<Flow>(itFlow->second);
        if(pFlow)
        {
            pmlLog(pml::LOG_DEBUG) << "NMOS: " << "CreateSDP Destination Port = " << state.tpSender.nDestinationPort ;
            unsigned short nPort(state.tpSender.nDestinationPort);
            if(nPort == 0)
            {
                nPort = 5004;
            }
            ssSDP << pFlow->CreateSDPLines(nPort);
            ssSDP << ssc.str();

            //get clock name from source
            auto itSource = NodeApi::Get().GetSources().FindNmosResource(pFlow->GetSourceId());
            if(itSource != NodeApi::Get().GetSources().GetResourceEnd())
            {
                auto pSource = std::dynamic_pointer_cast<Source>(itSource->second);
                auto sClock = pSource->GetClock();

                //clock information is probably at the media level
                if(m_setInterfaces.empty() || sClock.empty())
                {
                    ssSDP << NodeApi::Get().GetSelf().CreateClockSdp(sClock, "");
                }
                else
                {
                    ssSDP << NodeApi::Get().GetSelf().CreateClockSdp(sClock, *(m_setInterfaces.begin())); // @todo should we check all the intefaces for the clock mac address??
                }
            }
        }
    }
    //now put in the RTCP info if we've got any
    if(state.tpSender.bRtcpEnabled)
    {
        switch(SdpManager::CheckIpAddress(state.tpSender.sRtcpDestinationIp))
        {
            case SdpManager::IP4_UNI:
            case SdpManager::IP4_MULTI:
                ssSDP << "a=rtcp:" << state.tpSender.nRtcpDestinationPort << " IN IP4 " << state.tpSender.sRtcpDestinationIp << "\r\n";
                break;
            case SdpManager::IP6_UNI:
            case SdpManager::IP6_MULTI:
                ssSDP << "a=rtcp:" << state.tpSender.nRtcpDestinationPort << " IN IP6 " << state.tpSender.sRtcpDestinationIp << "\r\n";
                break;
            default:
                break;
        }
    }
    m_sTransportFile = ssSDP.str();

    pmlLog(pml::LOG_DEBUG) << "NMOS: " << "CreateSDP= " << m_sTransportFile ;
}


void Sender::Activate(bool bImmediate)
{
    std::lock_guard<std::mutex> lg(m_mutex);

    m_bActivateAllowed = false;
    //move the staged parameters to active parameters
    m_Active = m_Staged;

    if(m_sSourceIp.empty())
    {
        //get the bound to interface source address
        for(std::set<std::string>::iterator itInteface = m_setInterfaces.begin(); itInteface != m_setInterfaces.end(); ++itInteface)
        {
            std::map<std::string, nodeinterface>::const_iterator itDetails = NodeApi::Get().GetSelf().FindInterface((*itInteface));
            if(itDetails != NodeApi::Get().GetSelf().GetInterfaceEnd())
            {
                m_sSourceIp = itDetails->second.sMainIpAddress;
                break;
            }
        }
    }

    //Change auto settings to what they actually are
    m_Active.tpSender.Actualize(m_sSourceIp, m_sDestinationIp);


    // create the SDP
    if(m_Active.bMasterEnable)
    {
        CreateSDP(m_Active);    // @TODO need to update SDP but keep stuff
        if(m_sSDP.empty())
        {
            CreateSDP(m_Active);
        }
        else
        {
            m_sTransportFile = m_sSDP;
        }
    }
    else
    {
        m_sTransportFile.clear();
    }
    //@todo Set the flow to be whatever the flow is...

    //activate - set subscription, receiverId and active on master_enable.
    if(m_Staged.bMasterEnable)
    {
        m_sReceiverId = m_Staged.sReceiverId;
    }
    else
    {
        m_sReceiverId.clear();
    }
    m_bReceiverActive = m_Staged.bMasterEnable;

    if(!bImmediate)
    {
        CommitActivation();
    }
}

void Sender::CommitActivation()
{
    //reset the staged activation
    pmlLog(pml::LOG_DEBUG) << "NMOS: " << "ActivateSender: Reset Staged activation parameters..." ;
    m_Staged.eActivate = connection::ACT_NULL;
    m_Staged.sActivationTime.clear();
    m_Staged.sRequestedTime.clear();

    UpdateVersionTime();

    NodeApi::Get().SenderActivated(GetId());
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
        CreateSDP(m_Active);
        return true;
    }
    return false;
}

void Sender::SetupActivation(const std::string& sSourceIp, const std::string& sDestinationIp, const std::string& sSDP)
{
    m_sSourceIp = sSourceIp;
    m_sDestinationIp = sDestinationIp;
    m_sSDP = sSDP;
}


void Sender::SetDestinationDetails(const std::string& sDestinationIp, unsigned short nDestinationPort)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_Active.tpSender.sDestinationIp = sDestinationIp;
    m_Active.tpSender.nDestinationPort = nDestinationPort;
    CreateSDP(m_Active);
    UpdateVersionTime();
}

void Sender::MasterEnable(bool bEnable)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_Active.bMasterEnable = bEnable;
    m_Active.tpSender.bRtpEnabled = bEnable;
    m_Staged.bMasterEnable = bEnable;
    m_Staged.tpSender.bRtpEnabled = bEnable;
    UpdateVersionTime();
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
            if(m_Staged.sActivationTime.empty() == false)
            {
                Activator::Get().RemoveActivationSender(m_Staged.tpActivation, GetId());
                m_Staged.sActivationTime.clear();
                m_Staged.tpActivation = std::chrono::time_point<std::chrono::high_resolution_clock>();
            }
            m_bActivateAllowed = false;
            break;
        case connection::ACT_NOW:
            m_Staged.sActivationTime = GetCurrentTaiTime();
            m_bActivateAllowed = true;
            Activate(true);
            break;
        case connection::ACT_ABSOLUTE:
            {
                std::chrono::time_point<std::chrono::high_resolution_clock> tp;
                if(ConvertTaiStringToTimePoint(m_Staged.sRequestedTime, tp))
                {
                    m_bActivateAllowed = true;

                    m_Staged.sActivationTime = m_Staged.sRequestedTime;
                    m_Staged.tpActivation = tp;
                    Activator::Get().AddActivationSender(tp, GetId());
                }
                else
                {
                    pmlLog(pml::LOG_ERROR) << "NMOS: " << "Stage Sender: Invalid absolute time" ;
                    bOk = false;
                }
            }
            break;
        case connection::ACT_RELATIVE:
            // start a thread that will sleep for the given time period and then tell the main thread to do its stuff
            try
            {
                std::chrono::time_point<std::chrono::high_resolution_clock> tp(std::chrono::high_resolution_clock::now().time_since_epoch());
                std::vector<std::string> vTime;
                SplitString(vTime, m_Staged.sRequestedTime, ':');
                if(vTime.size() != 2)
                {
                    throw std::invalid_argument("invalid time");
                }
                m_bActivateAllowed = true;

                std::chrono::nanoseconds nano((static_cast<long long int>(std::stoul(vTime[0]))*1000000000)+stoul(vTime[1]));
                std::chrono::time_point<std::chrono::high_resolution_clock> tpAbs(tp+nano-LEAP_SECONDS);

                m_Staged.sActivationTime = ConvertTimeToString(tpAbs, true);
                m_Staged.tpActivation = tpAbs;
                Activator::Get().AddActivationSender(tpAbs, GetId());
            }
            catch(std::invalid_argument& ia)
            {
                pmlLog(pml::LOG_ERROR) << "NMOS: " << "Stage Sender: Invalid time" ;
                bOk = false;
            }
            break;
    }
    return bOk;
}
