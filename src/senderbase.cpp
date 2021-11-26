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
    m_Staged(true, ((flagsTransport & TransportParamsRTP::REDUNDANT) ? 2 : 1)),
    m_Active(true, ((flagsTransport & TransportParamsRTP::REDUNDANT) ? 2 : 1)),
    m_bActivateAllowed(false)
{

    AddInterfaceBinding(sInterface);


    m_Staged.SetTPAllowed(flagsTransport);
    m_Active.SetTPAllowed(flagsTransport);
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
    return m_Staged.GetJson();
}


Json::Value Sender::GetConnectionActiveJson(const ApiVersion& version) const
{
    return m_Active.GetJson();
}




Json::Value Sender::GetConnectionConstraintsJson(const ApiVersion& version) const
{
    Json::Value jsArray(Json::arrayValue);
    for(const auto& tp : m_Staged.GetTransportParams())
    {
        jsArray.append(tp.GetConstraints().GetJson());
    }
    return jsArray;
}



bool Sender::CheckConstraints(const connectionSender<activationResponse>& conRequest)
{
    return m_Staged.CheckConstraints(conRequest);
}

bool Sender::IsLocked()
{
    std::lock_guard<std::mutex> lg(m_mutex);
    auto act = m_Staged.GetConstActivation().GetMode();
    return (act == activation::ACT_ABSOLUTE || act == activation::ACT_RELATIVE);
}


connectionSender<activationResponse> Sender::GetStaged()
{
    std::lock_guard<std::mutex> lg(m_mutex);
    return m_Staged;
}

connectionSender<activationResponse> Sender::GetActive()
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
    m_Active.Actualize(m_sSourceIp, m_sDestinationIp);

    //@todo Set the flow to be whatever the flow is...

    //activate - set subscription, receiverId and active on master_enable.
    if(m_Active.GetMasterEnable() && *m_Active.GetMasterEnable())
    {
        auto receiver = m_Active.GetReceiverId();
        if(receiver)
        {
            m_sReceiverId = *receiver;
        }
        else
        {
            m_sReceiverId.clear();
        }
        m_bReceiverActive = true;
    }
    else
    {
        m_sReceiverId.clear();
        m_bReceiverActive = false;
    }


}

void Sender::CommitActivation()
{
    //reset the staged activation
    pmlLog(pml::LOG_DEBUG) << "NMOS: " << "ActivateSender: Reset Staged activation parameters..." ;
    m_Staged.GetActivation().Clear();

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

void Sender::ActualizeUnitialisedActive(const std::string& sSourceIp, const std::string& sDestinationIp, const std::string& sSDP)
{
    m_sSourceIp = sSourceIp;
    m_sDestinationIp = sDestinationIp;
    m_sSDP = sSDP;
    m_Active.Actualize(m_sSourceIp, m_sDestinationIp);
}


void Sender::SetDestinationDetails(const std::string& sDestinationIp, unsigned short nDestinationPort)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_Active.SetDestinationDetails(sDestinationIp, nDestinationPort);

    //@todo create the SDP somehow         CreateSDP(api, m_Active);

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

    pmlLog() << "Sender::Staged = " << m_Staged.GetJson();
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


bool Sender::AddConstraint(const std::string& sKey, const std::experimental::optional<int>& minValue, const std::experimental::optional<int>& maxValue, const std::experimental::optional<std::string>& pattern,
                                   const std::vector<pairEnum_t>& vEnum, const std::experimental::optional<size_t>& tp)
{
    std::lock_guard<std::mutex> lg(m_mutex);
    return m_Staged.AddConstraint(sKey, minValue, maxValue, pattern, vEnum, tp);
}
