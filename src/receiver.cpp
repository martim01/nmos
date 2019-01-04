#include "receiver.h"
#include "sender.h"
#include <chrono>
#include "eventposter.h"
#include "connection.h"
#include "activationthread.h"
#include "log.h"

static void ActivationThreadReceiver(const std::chrono::time_point<std::chrono::high_resolution_clock>& tp, const std::string& sReceiverId, std::shared_ptr<EventPoster> pPoster)
{
    std::this_thread::sleep_until(tp);
    if(pPoster)
    {
        pPoster->_ActivateReceiver(sReceiverId);
    }
}


const std::string Receiver::TRANSPORT[4] = {"urn:x-nmos:transport:rtp", "urn:x-nmos:transport:rtp.ucast", "urn:x-nmos:transport:rtp.mcast","urn:x-nmos:transport:dash"};
const std::string Receiver::TYPE[4] = {"urn:x-nmos:format:audio", "urn:x-nmos:format:video", "urn:x-nmos:format:data", "urn:x-nmos:format:mux"};



Receiver::Receiver(std::string sLabel, std::string sDescription, enumTransport eTransport, std::string sDeviceId, enumType eFormat) :
    Resource(sLabel, sDescription),
    m_eTransport(eTransport),
    m_sDeviceId(sDeviceId),
    m_sSenderId(""),
    m_bSenderActive(false),
    m_eType(eFormat),
    m_pSender(0)
{

}

Receiver::~Receiver()
{

}

void Receiver::AddInterfaceBinding(std::string sInterface)
{
    m_setInterfaces.insert(sInterface);
    UpdateVersionTime();
}

void Receiver::RemoveInterfaceBinding(std::string sInterface)
{
    m_setInterfaces.erase(sInterface);
    UpdateVersionTime();
}

void Receiver::AddCap(std::string sCap)
{
    m_setCaps.insert(sCap);
    UpdateVersionTime();
}

void Receiver::RemoveCap(std::string sCap)
{
    m_setCaps.erase(sCap);
    UpdateVersionTime();
}



void Receiver::SetTransport(enumTransport eTransport)
{
    m_eTransport = eTransport;
    UpdateVersionTime();
}

void Receiver::SetType(enumType eType)
{
    m_eType = eType;
    UpdateVersionTime();
}

bool Receiver::Commit()
{
    if(Resource::Commit())
    {
        m_json["device_id"] = m_sDeviceId;
        m_json["transport"] = TRANSPORT[m_eTransport];
        m_json["format"] = TYPE[m_eType];

        m_json["interface_bindings"] = Json::Value(Json::arrayValue);
        for(std::set<std::string>::iterator itInterface = m_setInterfaces.begin(); itInterface != m_setInterfaces.end(); ++itInterface)
        {
            m_json["interface_bindings"].append((*itInterface));
        }

        m_json["caps"] = Json::Value(Json::objectValue);
        m_json["caps"]["media_types"] = Json::Value(Json::arrayValue);
        for(std::set<std::string>::iterator itCap = m_setCaps.begin(); itCap != m_setCaps.end(); ++itCap)
        {
            m_json["caps"]["media_types"].append((*itCap));
        }


        m_json["subscription"] = Json::Value(Json::objectValue);

        if(m_pSender == NULL)
        {
             m_json["subscription"]["sender_id"] = Json::nullValue;
             m_json["subscription"]["active"] = false;
        }
        else
        {
            m_json["subscription"]["sender_id"] = m_pSender->GetId();
            m_json["subscription"]["active"] = true;    //@todo we need to get this subscription information from somewhere sensible
        }


        return true;
    }
    return false;
}



Json::Value Receiver::GetConnectionStagedJson() const
{
    return m_Staged.GetJson();
}


Json::Value Receiver::GetConnectionActiveJson() const
{
    return m_Active.GetJson();
}



std::shared_ptr<Sender> Receiver::GetSender() const
{
    return m_pSender;
}

void Receiver::SetSender(std::shared_ptr<Sender> pSender)
{

    if(pSender->GetId().empty() == false)
    {
        Log::Get(Log::DEBUG) << "Receiver subscribe to sender " << pSender->GetId() << std::endl;
        m_pSender = pSender;
    }
    else
    {   //this means unsubscribe
        Log::Get(Log::DEBUG) << "Receiver unssubscribe " << std::endl;
        m_pSender = 0;
    }
    UpdateVersionTime();

    // @todo need to update the IS-05 stage and active connection settings to match
}


Json::Value Receiver::GetConnectionConstraintsJson() const
{
    Json::Value jsArray(Json::arrayValue);
    jsArray.append(m_constraints.GetJson());
    return jsArray;
}


bool Receiver::CheckConstraints(const connectionReceiver& conRequest)
{
    bool bMeets = m_constraints.destination_port.MeetsConstraint(conRequest.tpReceiver.nDestinationPort);
    bMeets &= m_constraints.fec_destination_ip.MeetsConstraint(conRequest.tpReceiver.sFecDestinationIp);
    bMeets &= m_constraints.fec_enabled.MeetsConstraint(conRequest.tpReceiver.bFecEnabled);
    bMeets &= m_constraints.fec_mode.MeetsConstraint(TransportParamsRTP::STR_FEC_MODE[conRequest.tpReceiver.eFecMode]);
    bMeets &= m_constraints.fec1D_destination_port.MeetsConstraint(conRequest.tpReceiver.nFec1DDestinationPort);
    bMeets &= m_constraints.fec2D_destination_port.MeetsConstraint(conRequest.tpReceiver.nFec2DDestinationPort);
    bMeets &= m_constraints.rtcp_destination_ip.MeetsConstraint(conRequest.tpReceiver.sRtcpDestinationIp);
    bMeets &= m_constraints.rtcp_destination_port.MeetsConstraint(conRequest.tpReceiver.nRtcpDestinationPort);
    bMeets &= m_constraints.interface_ip.MeetsConstraint(conRequest.tpReceiver.sInterfaceIp);

    return bMeets;
}


bool Receiver::IsLocked() const
{
    std::lock_guard<std::mutex> lg(m_mutex);
    return (m_Staged.eActivate == connection::ACT_ABSOLUTE || m_Staged.eActivate == connection::ACT_RELATIVE);
}

bool Receiver::Stage(const connectionReceiver& conRequest, std::shared_ptr<EventPoster> pPoster)
{
    bool bOk(true);
    m_mutex.lock();
    m_Staged = conRequest;

    //need to decode the SDP and make the correct transport param changes here
    if(m_Staged.sTransportFileType == "application/sdp" && m_Staged.sTransportFileData.empty() == false)
    {
        SdpManager::SdpToTransportParams(m_Staged.sTransportFileData, m_Staged.tpReceiver);
    }

    m_mutex.unlock();

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
                pPoster->_ActivateReceiver(GetId());
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
                    std::thread thActive(ActivationThreadReceiver, tp, GetId(), pPoster);
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
                    std::thread thActive(ActivationThreadReceiver, (tp+nano), GetId(), pPoster);
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


connectionReceiver Receiver::GetStaged() const
{
    std::lock_guard<std::mutex> lg(m_mutex);
    return m_Staged;
}


void Receiver::Activate(const std::string& sInterfaceIp)
{
    //move the staged parameters to active parameters
    m_Active = m_Staged;

    //Change auto settings to what they actually are
    m_Active.tpReceiver.Actualize(sInterfaceIp);

    //activeate - set subscription, receiverId and active on master_enable. Commit afterwards
    m_sSenderId = m_Staged.sSenderId;
    m_bSenderActive = m_Staged.bMasterEnable;

    //reset the staged activation
    m_Staged.eActivate = connection::ACT_NULL;
    m_Staged.sActivationTime.clear();
    m_Staged.sRequestedTime.clear();

}
