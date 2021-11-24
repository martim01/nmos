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



Sender::Sender(const std::string& sLabel, const std::string& sDescription, const std::string& sFlowId, enumTransport eTransport, const std::string& sDeviceId, const std::string& sInterface, TransportParamsRTP::flagsTP flagsTransport) :
    IOResource("sender", sLabel, sDescription, eTransport),
    m_sFlowId(sFlowId),
    m_sDeviceId(sDeviceId),
    m_sReceiverId(""),
    m_bReceiverActive(false),
    m_bActivateAllowed(false)
{
    m_Active.bClient = false;
    m_Staged.bClient = false;

    AddInterfaceBinding(sInterface);

    m_vConstraints.push_back(ConstraintsSender(flagsTransport));

    if(flagsTransport & TransportParamsRTP::REDUNDANT)
    {
        m_vConstraints.push_back(ConstraintsSender(flagsTransport));
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


    }

}

Sender::Sender() : IOResource("sender")
{

}

Sender::~Sender()
{

}

bool Sender::UpdateFromJson(const Json::Value& jsData)
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


void Sender::SetReceiverId(const std::string& sReceiverId, bool bActive)
{
    m_sReceiverId = sReceiverId;
    m_bReceiverActive = bActive;

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
    for(const auto& con :m_vConstraints)
    {
        jsArray.append(con.GetJson(version));
    }
    return jsArray;
}



bool Sender::CheckConstraints(const connectionSender& conRequest)
{
    bool bMeets = (m_vConstraints.size() == conRequest.tpSenders.size());
    if(bMeets)
    {
        for(size_t i = 0; i < m_vConstraints.size(); i++)
        {
            bMeets &= m_vConstraints[i].MeetsConstraint("destination_port", conRequest.tpSenders[i].nDestinationPort);
            bMeets &= m_vConstraints[i].MeetsConstraint("fec_destination_ip", conRequest.tpSenders[i].sFecDestinationIp);
            bMeets &= m_vConstraints[i].MeetsConstraint("fec_enabled", conRequest.tpSenders[i].bFecEnabled);
            bMeets &= m_vConstraints[i].MeetsConstraint("fec_mode", TransportParamsRTP::STR_FEC_MODE[conRequest.tpSenders[i].eFecMode]);
            bMeets &= m_vConstraints[i].MeetsConstraint("fec1D_destination_port", conRequest.tpSenders[i].nFec1DDestinationPort);
            bMeets &= m_vConstraints[i].MeetsConstraint("fec2D_destination_port", conRequest.tpSenders[i].nFec2DDestinationPort);
            bMeets &= m_vConstraints[i].MeetsConstraint("rtcp_destination_ip", conRequest.tpSenders[i].sRtcpDestinationIp);
            bMeets &= m_vConstraints[i].MeetsConstraint("rtcp_destination_port", conRequest.tpSenders[i].nRtcpDestinationPort);

            bMeets &= m_vConstraints[i].MeetsConstraint("destination_ip", conRequest.tpSenders[i].sDestinationIp);
            bMeets &= m_vConstraints[i].MeetsConstraint("source_ip", conRequest.tpSenders[i].sSourceIp);
            bMeets &= m_vConstraints[i].MeetsConstraint("source_port", conRequest.tpSenders[i].nSourcePort);
            bMeets &= m_vConstraints[i].MeetsConstraint("fec_type", TransportParamsRTPSender::STR_FEC_TYPE[conRequest.tpSenders[i].eFecType]);
            bMeets &= m_vConstraints[i].MeetsConstraint("fec_block_width", conRequest.tpSenders[i].nFecBlockWidth);
            bMeets &= m_vConstraints[i].MeetsConstraint("fec_block_height", conRequest.tpSenders[i].nFecBlockHeight);
            bMeets &= m_vConstraints[i].MeetsConstraint("fec1D_source_port", conRequest.tpSenders[i].nFec1DSourcePort);
            bMeets &= m_vConstraints[i].MeetsConstraint("fec2D_source_port", conRequest.tpSenders[i].nFec2DSourcePort);
            bMeets &= m_vConstraints[i].MeetsConstraint("rtcp_enabled", conRequest.tpSenders[i].bRtcpEnabled);
            bMeets &= m_vConstraints[i].MeetsConstraint("rtcp_source_port", conRequest.tpSenders[i].nRtcpSourcePort);
            bMeets &= m_vConstraints[i].MeetsConstraint("rtp_enabled", conRequest.tpSenders[i].bRtpEnabled);
        }
    }
    return bMeets;
}

bool Sender::IsLocked()
{
    std::lock_guard<std::mutex> lg(m_mutex);
    return (m_Staged.eActivate == connection::ACT_ABSOLUTE || m_Staged.eActivate == connection::ACT_RELATIVE);
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




const std::string& Sender::GetTransportFile() const
{
    return m_sTransportFile;
}


void Sender::SetManifestHref(const std::string& sHref)
{
    m_sManifest = sHref;
}


const std::string& Sender::GetManifestHref() const
{
    return m_sManifest;
}


bool Sender::IsActivateAllowed() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_bActivateAllowed;
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
