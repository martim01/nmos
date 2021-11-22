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


void Sender::Activate(const std::string& sSourceIp)
{
    std::lock_guard<std::mutex> lg(m_mutex);

    m_bActivateAllowed = false;
    //move the staged parameters to active parameters
    m_Active = m_Staged;

    if(m_sSourceIp.empty())
    {
        m_sSourceIp = sSourceIp;
    }

    //Change auto settings to what they actually are
    for(size_t i = 0;i < m_vConstraints.size(); i++)
    {
        m_Active.tpSenders[i].Actualize(m_sSourceIp, m_sDestinationIp);
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

}

void Sender::CommitActivation()
{
    //reset the staged activation
    pmlLog(pml::LOG_DEBUG) << "NMOS: " << "ActivateSender: Reset Staged activation parameters..." ;
    m_Staged.eActivate = connection::ACT_NULL;
    m_Staged.sActivationTime.clear();
    m_Staged.sRequestedTime.clear();

    UpdateVersionTime();

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

connection::enumActivate Sender::Stage(const connectionSender& conRequest)
{
    std::lock_guard<std::mutex> lg(m_mutex);
    m_Staged = conRequest;
    return m_Staged.eActivate;
}


void Sender::SetTransportFile(const std::string& sSDP)
{
    m_sTransportFile = sSDP;
}

void Sender::RemoveStagedActivationTime()
{
    m_Staged.sActivationTime.clear();
    m_Staged.tpActivation = std::chrono::time_point<std::chrono::high_resolution_clock>();
}

void Sender::SetStagedActivationTimePoint(const std::chrono::time_point<std::chrono::high_resolution_clock>& tp)
{
    m_Staged.tpActivation = tp;
    m_Staged.sActivationTime = ConvertTimeToString(tp);
}
