#include "sender.h"
#include "nodeapiprivate.h"
#include "utils.h"
#include "log.h"
#include "activator.h"
#include "sdp.h"
#include "device.h"
#include "flow.h"
#include "source.h"

using namespace pml::nmos;

Sender::Sender(const std::string& sLabel, const std::string& sDescription, const std::string& sFlowId, enumTransport eTransport, const std::string& sDeviceId, const std::string& sInterface, TransportParamsRTP::flagsTP flagsTransport) :
   SenderBase(sLabel, sDescription, sFlowId, eTransport, sDeviceId, sInterface, flagsTransport)
{
    SetupActivation("", "239.220.1.1","");  // @todo choose some defaults - get the ori
    //Activate();
}

Sender::Sender() : SenderBase()
{
}

void Sender::CreateSDP(NodeApiPrivate& api)
{
    CreateSDP(api, m_Active);
}

void Sender::CreateSDP(NodeApiPrivate& api, const connectionSender& state)
{
    std::stringstream ssSDP;
    ssSDP << "v=0\r\n";
    ssSDP << "o=- " << GetCurrentTaiTime(false) << " " << GetCurrentTaiTime(false) << " IN IP";
    switch(SdpManager::CheckIpAddress(state.tpSenders[0].sSourceIp))
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
    ssSDP << state.tpSenders[0].sSourceIp << "\r\n";    // @todo should check here if sSourceIp is not set to auto
    ssSDP << "t=0 0 \r\n";

    std::map<std::string, std::shared_ptr<Device> >::const_iterator itDevice = api.GetDevices().FindNmosResource(m_sDeviceId);
    if(itDevice != api.GetDevices().GetResourceEnd())
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
    switch(SdpManager::CheckIpAddress(state.tpSenders[0].sDestinationIp))
    {
        case SdpManager::IP4_UNI:
            ssc << "c=IN IP4 " << state.tpSenders[0].sDestinationIp << "\r\n";
            ssSDP << ssc.str();
            ssSDP << "a=type:unicast\r\n";
            break;
        case SdpManager::IP4_MULTI:
            ssc << "c=IN IP4 " << state.tpSenders[0].sDestinationIp << "/32\r\n";
            ssSDP << ssc.str();
            ssSDP << "a=source-filter:incl IN IP4 " << state.tpSenders[0].sDestinationIp << " " << state.tpSenders[0].sSourceIp << "\r\n";
            ssSDP << "a=type:multicast\r\n";
            break;
        case SdpManager::IP6_UNI:
            ssc << "c=IN IP6 " << state.tpSenders[0].sDestinationIp << "\r\n";
            ssSDP << ssc.str();
            ssSDP << "a=type:unicast\r\n";
            break;
        case SdpManager::IP6_MULTI:
            ssc << "c=IN IP6 " << state.tpSenders[0].sDestinationIp << "\r\n";
            ssSDP << ssc.str();
            ssSDP << "a=source-filter:incl IN IP6 " << state.tpSenders[0].sDestinationIp << " " << state.tpSenders[0].sSourceIp << "\r\n";
            ssSDP << "a=type:multicast\r\n";
            break;
        case SdpManager::SdpManager::IP_INVALID:
            pmlLog(pml::LOG_WARN) << "NMOS: Sender can't create SDP - destination IP invalid '" << state.tpSenders[0].sDestinationIp << "'";
            break;
    }

    ssSDP << api.CreateFlowSdp(m_sFlowId, state, ssc.str(), m_setInterfaces);

    if(state.tpSenders[0].bRtcpEnabled)
    {
        switch(SdpManager::CheckIpAddress(state.tpSenders[0].sRtcpDestinationIp))
        {
            case SdpManager::IP4_UNI:
            case SdpManager::IP4_MULTI:
                ssSDP << "a=rtcp:" << state.tpSenders[0].nRtcpDestinationPort << " IN IP4 " << state.tpSenders[0].sRtcpDestinationIp << "\r\n";
                break;
            case SdpManager::IP6_UNI:
            case SdpManager::IP6_MULTI:
                ssSDP << "a=rtcp:" << state.tpSenders[0].nRtcpDestinationPort << " IN IP6 " << state.tpSenders[0].sRtcpDestinationIp << "\r\n";
                break;
            default:
                break;
        }
    }
    m_sTransportFile = ssSDP.str();

    pmlLog(pml::LOG_DEBUG) << "NMOS: " << "CreateSDP= " << m_sTransportFile ;
}


void Sender::Activate(bool bImmediate, NodeApiPrivate& api)
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
            std::map<std::string, nodeinterface>::const_iterator itDetails = api.GetSelf().FindInterface((*itInteface));
            if(itDetails != api.GetSelf().GetInterfaceEnd())
            {
                m_sSourceIp = itDetails->second.sMainIpAddress;
                break;
            }
        }
    }

    //Change auto settings to what they actually are
    for(size_t i = 0;i < m_vConstraints.size(); i++)
    {
        m_Active.tpSenders[i].Actualize(m_sSourceIp, m_sDestinationIp);
    }


    // create the SDP
    if(m_Active.bMasterEnable)
    {
        CreateSDP(api, m_Active);    // @TODO need to update SDP but keep stuff
        if(m_sSDP.empty())
        {
            CreateSDP(api, m_Active);
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
        CommitActivation(api);
    }
}

void Sender::CommitActivation(NodeApiPrivate& api)
{
    //reset the staged activation
    pmlLog(pml::LOG_DEBUG) << "NMOS: " << "ActivateSender: Reset Staged activation parameters..." ;
    m_Staged.eActivate = connection::ACT_NULL;
    m_Staged.sActivationTime.clear();
    m_Staged.sRequestedTime.clear();

    UpdateVersionTime();

    api.SenderActivated(GetId());
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
        //@todo create the SDP somehow         CreateSDP(api, m_Active);
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
    for(size_t i = 0;i < m_vConstraints.size(); i++)
    {
        m_Active.tpSenders[i].sDestinationIp = sDestinationIp;
        m_Active.tpSenders[i].nDestinationPort = nDestinationPort;
    }
    //@todo create the SDP somehow         CreateSDP(api, m_Active);

    UpdateVersionTime();
}

void Sender::MasterEnable(bool bEnable)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_Active.bMasterEnable = bEnable;
    m_Staged.bMasterEnable = bEnable;
    for(size_t i = 0;i < m_vConstraints.size(); i++)
    {
        m_Active.tpSenders[i].bRtpEnabled = bEnable;
        m_Staged.tpSenders[i].bRtpEnabled = bEnable;
    }
    UpdateVersionTime();
}

bool Sender::Stage(const connectionSender& conRequest, std::shared_ptr<EventPoster> pPoster, NodeApiPrivate& api)
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
                api.RemoveActivationSender(m_Staged.tpActivation, GetId());
                m_Staged.sActivationTime.clear();
                m_Staged.tpActivation = std::chrono::time_point<std::chrono::high_resolution_clock>();
            }
            m_bActivateAllowed = false;
            break;
        case connection::ACT_NOW:
            m_Staged.sActivationTime = GetCurrentTaiTime();
            m_bActivateAllowed = true;
            Activate(true, api);
            break;
        case connection::ACT_ABSOLUTE:
            {
                std::chrono::time_point<std::chrono::high_resolution_clock> tp;
                if(ConvertTaiStringToTimePoint(m_Staged.sRequestedTime, tp))
                {
                    m_bActivateAllowed = true;

                    m_Staged.sActivationTime = m_Staged.sRequestedTime;
                    m_Staged.tpActivation = tp;

                    pmlLog(pml::LOG_DEBUG) << "NMOS: Sender  - add absolute activation";
                    api.AddActivationSender(tp, GetId());
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
                pmlLog(pml::LOG_DEBUG) << "NMOS: Sender  - add relative activation";
                api.AddActivationSender(tpAbs, GetId());
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
