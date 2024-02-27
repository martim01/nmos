#include "senderbase.h"
#include <thread>

#include "flow.h"
#include "device.h"
#include "utils.h"
#include "log.h"
#include <algorithm>
#include <vector>
#include "source.h"

using namespace pml::nmos;



Sender::Sender(const std::string& sLabel, const std::string& sDescription, const std::string& sFlowId, enumTransport eTransport, const std::string& sDeviceId, const std::vector<std::string>& vInterface, TransportParamsRTP::flagsTP flagsTransport, const std::optional<std::string>& multicastIp, const std::optional<std::string>& multicastIpR) :
    IOResource("sender", sLabel, sDescription, eTransport),
    m_sFlowId(sFlowId),
    m_sDeviceId(sDeviceId),
    m_sReceiverId(""),
    m_bSendingToReceiver(false),
    m_Staged(true, flagsTransport),
    m_Active(true, flagsTransport),
    m_bActivateAllowed(false)
{
    for(const auto& sInterface : vInterface)
    {
        AddInterfaceBinding(sInterface);
    }

    CreateConstraints(m_Staged.GetJson());

    if((flagsTransport & TransportParamsRTP::flagsTP::REDUNDANT))
    {
        m_vSourceDestIp.resize(2);
    }
    else
    {
        m_vSourceDestIp.resize(1);
    }

    if((m_eTransport & enumTransport::RTP_MCAST))
    {
        if(multicastIp)
        {
            m_vSourceDestIp[0].second = *multicastIp;
        }
        if((flagsTransport & TransportParamsRTP::flagsTP::REDUNDANT) && multicastIpR)
        {
            m_vSourceDestIp[1].second = *multicastIpR;
        }
    }
}

Sender::Sender() :
    IOResource("sender"),
    m_bSendingToReceiver(false),
    m_Staged(true, TransportParamsRTP::CORE),
    m_Active(true, TransportParamsRTP::CORE),
    m_bActivateAllowed(false)
{
    CreateConstraints(m_Staged.GetJson());
}

std::shared_ptr<Sender> Sender::Create(const Json::Value& jsResponse)
{
    auto pResource  = std::make_shared<Sender>();
    if(pResource->UpdateFromJson(jsResponse))
    {
        return pResource;
    }
    return nullptr;
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
                AddInterfaceBinding(jsData["interface_bindings"][n].asString());
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
        m_bSendingToReceiver = jsData["subscription"]["active"].asBool();
    }
    return m_bIsOk;
}


void Sender::SetReceiverId(const std::string& sReceiverId, bool bActive)
{
    m_sReceiverId = sReceiverId;
    m_bSendingToReceiver = bActive;

}





Json::Value Sender::GetConnectionStagedJson(const ApiVersion& version) const
{
    return m_Staged.GetJson();
}


Json::Value Sender::GetConnectionActiveJson(const ApiVersion& version) const
{
    return m_Active.GetJson();
}


bool Sender::CheckConstraints(const connectionSender<activationResponse>& conRequest)
{
    return IOResource::CheckConstraints(conRequest.GetJson());
}

bool Sender::IsLocked()
{
    std::lock_guard<std::mutex> lg(m_mutex);
    auto act = m_Staged.GetConstActivation().GetMode();
    return (act == activation::ACT_ABSOLUTE || act == activation::ACT_RELATIVE);
}


connectionSender<activationResponse> Sender::GetStaged() const
{
    std::lock_guard<std::mutex> lg(m_mutex);
    return m_Staged;
}

connectionSender<activationResponse> Sender::GetActive()  const
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


void Sender::Activate(const std::vector<std::string>& vSourceIp)
{
    std::lock_guard<std::mutex> lg(m_mutex);

    m_bActivateAllowed = false;
    //move the staged parameters to active parameters
    m_Active = m_Staged;

    if(vSourceIp.size() == m_vSourceDestIp.size())
    {
        for(size_t nStream = 0; nStream < vSourceIp.size(); ++nStream)
        {
            if(m_vSourceDestIp[nStream].first.empty())
            {
                m_vSourceDestIp[nStream].first = vSourceIp[nStream];
            }
        }
    }

    //Change auto settings to what they actually are
    m_Active.Actualize(m_vSourceDestIp);

    //@todo Set the flow to be whatever the flow is...

    //activate - set subscription, receiverId and active on master_enable.
    if(m_Active.GetMasterEnable() && *m_Active.GetMasterEnable())
    {
        auto receiver = m_Active.GetReceiverId();
        if(receiver)
        {
            m_sReceiverId = *receiver;
            m_bSendingToReceiver = true;
        }
        else
        {
            m_sReceiverId.clear();
            m_bSendingToReceiver = false;
        }

    }
    else
    {
        m_sReceiverId.clear();
        m_bSendingToReceiver = false;
    }


}

void Sender::CommitActivation()
{
    //reset the staged activation
    m_Staged.GetActivation().Clear();

    UpdateVersionTime();

}

const std::string& Sender::GetDestinationIp(bool bRedundant) const
{
    if(bRedundant && m_vSourceDestIp.size() == 2)
    {
        return m_vSourceDestIp[1].second;
    }
    else
    {
        return m_vSourceDestIp[0].second;
    }
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
        for(const auto& sInterface : m_vInterfaces)
        {
            m_json["interface_bindings"].append((sInterface));
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

        if(m_bSendingToReceiver)
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

void Sender::SetupActivation(const std::vector<std::pair<std::string, std::string>>& vSourceDestIp, const std::string& sSDP)
{
    if(vSourceDestIp.size() == m_vSourceDestIp.size())
    {
        m_vSourceDestIp = vSourceDestIp;
    }
    m_sSDP = sSDP;
}

void Sender::ActualizeUnitialisedActive(const std::vector<std::string>& vSourceIp)
{
    if(vSourceIp.size() == m_vSourceDestIp.size())
    {
        for(size_t nStream = 0; nStream < vSourceIp.size(); ++nStream)
        {
            m_vSourceDestIp[nStream].first = vSourceIp[nStream];
        }
        m_Active.Actualize(m_vSourceDestIp);
    }
}


void Sender::SetDestinationDetails(const std::string& sDestinationIp, unsigned short nDestinationPort)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_Active.SetDestinationDetails(sDestinationIp, nDestinationPort);

    UpdateVersionTime();
}

void Sender::MasterEnable(bool bEnable)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_Active.MasterEnable(bEnable);
    m_Staged.MasterEnable(bEnable);

    UpdateVersionTime();
}

activation::enumActivate Sender::Stage(const connectionSender<activationResponse>& conRequest)
{
    std::lock_guard<std::mutex> lg(m_mutex);


    m_Staged = conRequest;  //we've already done the patching to make sure that only the bits than need changing are

    return m_Staged.GetActivation().GetMode();
}


void Sender::SetTransportFile(const std::string& sSDP)
{
    m_sTransportFile = sSDP;
}

void Sender::RemoveStagedActivationTime()
{
    m_Staged.GetActivation().SetActivationTime({});
}

void Sender::SetStagedActivationTimePoint(const std::chrono::time_point<std::chrono::high_resolution_clock>& tp)
{
    m_Staged.GetActivation().SetActivationTime(tp);
}

void Sender::MarkRTPTransmissionAsActive(bool bActive, std::optional<size_t> tp)
{
//    if(tp)
//    {
//        if(*tp < m_Staged.GetTransportParams().size())
//        {
//             m_Staged.GetTransportParams()[*tp].EnableRtp(bActive);
//             m_Active.GetTransportParams()[*tp].EnableRtp(bActive);
//        }
//    }
//    else
//    {
//        for(size_t i = 0; i < m_Staged.GetTransportParams().size())
//        {
//            m_Staged.GetTransportParams()[i].EnableRtp(bActive);
//            m_Active.GetTransportParams()[i].EnableRtp(bActive);
//        }
//    }

}
