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


const std::string Receiver::STR_TRANSPORT[4] = {"urn:x-nmos:transport:rtp", "urn:x-nmos:transport:rtp.ucast", "urn:x-nmos:transport:rtp.mcast","urn:x-nmos:transport:dash"};
const std::string Receiver::STR_TYPE[4] = {"urn:x-nmos:format:audio", "urn:x-nmos:format:video", "urn:x-nmos:format:data", "urn:x-nmos:format:mux"};



Receiver::Receiver(std::string sLabel, std::string sDescription, enumTransport eTransport, std::string sDeviceId, enumType eFormat, TransportParamsRTP::flagsTP flagsTransport) :
    Resource("receiver", sLabel, sDescription),
    m_eTransport(eTransport),
    m_sDeviceId(sDeviceId),
    m_sSenderId(""),
    m_bSenderActive(false),
    m_eType(eFormat),
    m_pSender(0)
{
    if(flagsTransport & TransportParamsRTP::FEC)
    {
        m_Staged.tpReceiver.eFec = TransportParamsRTP::TP_SUPPORTED;
        m_Active.tpReceiver.eFec = TransportParamsRTP::TP_SUPPORTED;
    }
    else
    {
        m_Staged.tpReceiver.eFec = TransportParamsRTP::TP_NOT_SUPPORTED;
        m_Active.tpReceiver.eFec = TransportParamsRTP::TP_NOT_SUPPORTED;
    }

    if(flagsTransport & TransportParamsRTP::RTCP)
    {
        m_Staged.tpReceiver.eRTCP = TransportParamsRTP::TP_SUPPORTED;
        m_Active.tpReceiver.eRTCP = TransportParamsRTP::TP_SUPPORTED;
    }
    else
    {
        m_Staged.tpReceiver.eRTCP = TransportParamsRTP::TP_NOT_SUPPORTED;
        m_Active.tpReceiver.eRTCP = TransportParamsRTP::TP_NOT_SUPPORTED;
    }
    if(flagsTransport & TransportParamsRTP::MULTICAST)
    {
        m_Staged.tpReceiver.eMulticast = TransportParamsRTP::TP_SUPPORTED;
        m_Active.tpReceiver.eMulticast = TransportParamsRTP::TP_SUPPORTED;
    }
    else
    {
        m_Staged.tpReceiver.eMulticast = TransportParamsRTP::TP_NOT_SUPPORTED;
        m_Active.tpReceiver.eMulticast = TransportParamsRTP::TP_NOT_SUPPORTED;
    }
}

Receiver::~Receiver()
{

}

Receiver::Receiver() : Resource("receiver")
{

}

void Receiver::AddInterfaceBinding(std::string sInterface)
{
    m_setInterfaces.insert(sInterface);
}

void Receiver::RemoveInterfaceBinding(std::string sInterface)
{
    m_setInterfaces.erase(sInterface);
}

bool Receiver::AddCap(std::string sCap)
{
    //check that the caps are valid
    switch(m_eType)
    {
        case AUDIO:
            if(sCap.find("audio/") == std::string::npos)
            {
                return false;
            }
            break;
        case VIDEO:
            if(sCap.find("video/") == std::string::npos)
            {
               return false;
            }
            break;
        case DATA:
            if(sCap != "video/smpte291")
            {
                return false;
            }
            break;
        case MUX:
            if(sCap != "video/SMPTE2022-6")
            {
                return false;
            }
            break;
    }
    m_setCaps.insert(sCap);
    UpdateVersionTime();
    return true;
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

bool Receiver::UpdateFromJson(const Json::Value& jsData)
{
    Resource::UpdateFromJson(jsData);
    if(jsData["device_id"].isString() == false)
    {
        m_bIsOk = false;
        m_ssJsonError << "'device_id' not string" << std::endl;
    }
    if(jsData["transport"].isString() == false)
    {
        m_bIsOk = false;
        m_ssJsonError << "'transport' not string" << std::endl;
    }
    if(jsData["interface_bindings"].isArray() == false)
    {
        m_bIsOk = false;
        m_ssJsonError << "'interface_bindings' not array" << std::endl;
    }
    if(jsData["subscription"].isObject() == false)
    {
        m_bIsOk = false;
        m_ssJsonError << "'subscription' not object" << std::endl;
    }
    if(jsData["caps"].isObject() == false)
    {
        m_bIsOk = false;
        m_ssJsonError << "'caps' not object" << std::endl;
    }
    if(jsData["caps"]["media_types"].isArray() == false)
    {
        m_bIsOk = false;
        m_ssJsonError << "'caps' 'media_types' not array" << std::endl;
    }
    if(jsData["caps"]["media_types"].size() == 0)
    {
        m_bIsOk = false;
        m_ssJsonError << "'caps' 'media_types' empty" << std::endl;
    }
    if(jsData["format"].isString() == false)
    {
        m_bIsOk = false;
        m_ssJsonError << "'format' not string" << std::endl;
    }

    if(m_bIsOk)
    {
        m_sDeviceId = jsData["device_id"].asString();
        bool bFound(false);
        for(int i = 3; i >= 0; i--)
        {
            if(STR_TRANSPORT[i].find(jsData["transport"].asString()) != std::string::npos)
            {
                m_eTransport = enumTransport(i);
                bFound = true;
                break;
            }
        }
        if(!bFound)
        {
            m_ssJsonError << "'transport' " <<jsData["transport"].asString() <<" incorrect" << std::endl;
        }

        m_bIsOk &= bFound;
        for(int i = 0; i < 4; i++)
        {
            if(STR_TYPE[i].find(jsData["format"].asString()) != std::string::npos)
            {
                m_eType = enumType(i);
                bFound = true;
                break;
            }
        }
        if(!bFound)
        {
            m_ssJsonError << "'format' " <<jsData["format"].asString() <<" incorrect" << std::endl;
        }

        m_bIsOk &= bFound;
        for(Json::ArrayIndex ai = 0; ai < jsData["interface_bindings"].size(); ++ai)
        {
            if(jsData["interface_bindings"][ai].isString() == false)
            {
                m_bIsOk = false;
                break;
            }
            else
            {
                m_setInterfaces.insert(jsData["interface_bindings"][ai].asString());
            }
        }
        if(!bFound)
        {
            m_ssJsonError << "'interface_bindings' " <<jsData["interface_bindings"].asString() <<" incorrect" << std::endl;
        }

        for(Json::ArrayIndex ai = 0; ai < jsData["caps"]["media_types"].size(); ++ai)
        {
            if(jsData["caps"]["media_types"][ai].isString() == false)
            {
                m_bIsOk = false;
                m_ssJsonError << "'caps' 'media_types' #" << ai << "not a string" << std::endl;
            }
            else
            {
                //check that the caps are valid
                switch(m_eType)
                {
                case AUDIO:
                    if(jsData["caps"]["media_types"][ai].asString().find("audio/") == std::string::npos)
                    {
                        m_bIsOk = false;
                        m_ssJsonError << "'caps' 'media_types' #" << ai << " not audio whilst 'format' is" << std::endl;
                    }
                    else
                    {
                        m_setCaps.insert(jsData["caps"]["media_types"][ai].asString());
                    }
                    break;
                case VIDEO:
                    if(jsData["caps"]["media_types"][ai].asString().find("video/") == std::string::npos)
                    {
                        m_bIsOk = false;
                        m_ssJsonError << "'caps' 'media_types' #" << ai << " not video whilst 'format' is" << std::endl;
                    }
                    else
                    {
                        m_setCaps.insert(jsData["caps"]["media_types"][ai].asString());
                    }
                    break;
                case DATA:
                    if(jsData["caps"]["media_types"][ai].asString() != "video/smpte291")
                    {
                        m_bIsOk = false;
                        m_ssJsonError << "'caps' 'media_types' #" << ai << " not smpte291 whilst 'format' is data" << std::endl;
                    }
                    else
                    {
                        m_setCaps.insert(jsData["caps"]["media_types"][ai].asString());
                    }
                    break;
                case MUX:
                    if(jsData["caps"]["media_types"][ai].asString() != "video/SMPTE2022-6")
                    {
                        m_bIsOk = false;
                        m_ssJsonError << "'caps' 'media_types' #" << ai << "not 2022-6 whilst 'format' is mux" << std::endl;
                    }
                    else
                    {
                        m_setCaps.insert(jsData["caps"]["media_types"][ai].asString());
                    }
                }
            }

            if(m_bIsOk == false)
            {
                break;
            }
        }

        if(jsData["subscription"]["sender_id"].isString())
        {
            m_sSenderId = jsData["subscription"]["sender_id"].asString();
        }
        else if(jsData["subscription"]["sender_id"].isNull())
        {
            m_sSenderId.clear();
        }
        else
        {
            m_bIsOk = false;
            m_ssJsonError << "'subscription' 'sender_id' neither a string or null" << std::endl;
        }
        if(jsData["subscription"]["active"].isBool())
        {
            m_bSenderActive = jsData["subscription"]["active"].asBool();
        }
        else
        {
            m_bIsOk = false;
            m_ssJsonError << "'subscription' 'active' not bool" << std::endl;
        }
    }
    return m_bIsOk;
}
bool Receiver::Commit(const ApiVersion& version)
{
    if(Resource::Commit(version))
    {
        m_json["device_id"] = m_sDeviceId;
        m_json["transport"] = STR_TRANSPORT[m_eTransport];
        m_json["format"] = STR_TYPE[m_eType];

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



Json::Value Receiver::GetConnectionStagedJson(const ApiVersion& version) const
{
    return m_Staged.GetJson(version);
}


Json::Value Receiver::GetConnectionActiveJson(const ApiVersion& version) const
{
    return m_Active.GetJson(version);
}



std::shared_ptr<Sender> Receiver::GetSender() const
{
    return m_pSender;
}

void Receiver::SetSender(std::shared_ptr<Sender> pSender)
{

    if(pSender->IsOk() && pSender->GetId().empty() == false)
    {
        Log::Get(Log::LOG_INFO) << "Receiver subscribe to sender " << pSender->GetId() << std::endl;
        m_pSender = pSender;
    }
    else
    {   //this means unsubscribe
        Log::Get(Log::LOG_INFO) << "Receiver unssubscribe " << std::endl;
        m_pSender = 0;
    }
    UpdateVersionTime();

    // @todo need to update the IS-05 stage and active connection settings to match
}


Json::Value Receiver::GetConnectionConstraintsJson(const ApiVersion& version) const
{
    Json::Value jsArray(Json::arrayValue);
    jsArray.append(m_constraints.GetJson(version));
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
