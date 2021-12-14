#include "receiverbase.h"
#include "senderbase.h"
#include <chrono>
#include "eventposter.h"
#include "connection.h"
#include <algorithm>
#include "log.h"
#include "utils.h"

using namespace pml::nmos;


const std::array<std::string,4> Receiver::STR_TYPE = {"urn:x-nmos:format:audio", "urn:x-nmos:format:video", "urn:x-nmos:format:data", "urn:x-nmos:format:mux"};



Receiver::Receiver(const std::string& sLabel, const std::string& sDescription, enumTransport eTransport, const std::string& sDeviceId, enumType eFormat, TransportParamsRTP::flagsTP flagsTransport) :
    IOResource("receiver", sLabel, sDescription, eTransport),
    m_sDeviceId(sDeviceId),
    m_sSenderId(""),
    m_bSenderActive(false),
    m_eType(eFormat),
    m_Staged(true, flagsTransport),
    m_Active(true, flagsTransport),
    m_bActivateAllowed(false)
{


    CreateConstraints(m_Staged.GetJson());

}

Receiver::Receiver() :
    IOResource("receiver"),
    m_Staged(true, TransportParamsRTP::CORE),
    m_Active(true, TransportParamsRTP::CORE),
    m_bActivateAllowed(false)
{
    CreateConstraints(m_Staged.GetJson());
}

Receiver::~Receiver()
{

}

//Receiver::Receiver() : IOResource("receiver")
//{
//
//}
//

bool Receiver::AddCaps(const std::set<std::string>& setCaps)
{
    for(const auto& sCap : setCaps)
    {
        //check that the caps are valid
        if(CheckCap(sCap) == false)
        {
            return false;
        }
    }

    std::copy(setCaps.begin(), setCaps.end(), std::inserter(m_setCaps, m_setCaps.begin()));

    UpdateVersionTime();
    return true;
}

bool Receiver::AddCap(const std::string& sCap)
{
    if(CheckCap(sCap))
    {
        m_setCaps.insert(sCap);
        UpdateVersionTime();
        return true;
    }
    else
    {
        return false;
    }
}

bool Receiver::CheckCap(const std::string& sCap)
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
    return true;
}

void Receiver::RemoveCap(const std::string& sCap)
{
    m_setCaps.erase(sCap);
    UpdateVersionTime();
}

std::shared_ptr<Receiver> Receiver::Create(const Json::Value& jsResponse)
{
    auto pResource  = std::make_shared<Receiver>();
    if(pResource->UpdateFromJson(jsResponse))
    {
        return pResource;
    }
    return nullptr;
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
        m_ssJsonError << "'device_id' not string" ;
    }
    if(jsData["transport"].isString() == false)
    {
        m_bIsOk = false;
        m_ssJsonError << "'transport' not string" ;
    }
    if(jsData["interface_bindings"].isArray() == false)
    {
        m_bIsOk = false;
        m_ssJsonError << "'interface_bindings' not array" ;
    }
    if(jsData["subscription"].isObject() == false)
    {
        m_bIsOk = false;
        m_ssJsonError << "'subscription' not object" ;
    }
    if(jsData["caps"].isObject() == false)
    {
        m_bIsOk = false;
        m_ssJsonError << "'caps' not object" ;
    }
    if(jsData["caps"]["media_types"].isArray() == false)
    {
        m_bIsOk = false;
        m_ssJsonError << "'caps' 'media_types' not array" ;
    }
    if(jsData["caps"]["media_types"].size() == 0)
    {
        m_bIsOk = false;
        m_ssJsonError << "'caps' 'media_types' empty" ;
    }
    if(jsData["format"].isString() == false)
    {
        m_bIsOk = false;
        m_ssJsonError << "'format' not string" ;
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
            m_ssJsonError << "'transport' " <<jsData["transport"].asString() <<" incorrect" ;
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
            m_ssJsonError << "'format' " <<jsData["format"].asString() <<" incorrect" ;
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
            m_ssJsonError << "'interface_bindings' " <<jsData["interface_bindings"].asString() <<" incorrect" ;
        }

        for(Json::ArrayIndex ai = 0; ai < jsData["caps"]["media_types"].size(); ++ai)
        {
            if(jsData["caps"]["media_types"][ai].isString() == false)
            {
                m_bIsOk = false;
                m_ssJsonError << "'caps' 'media_types' #" << ai << "not a string" ;
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
                        m_ssJsonError << "'caps' 'media_types' #" << ai << " not audio whilst 'format' is" ;
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
                        m_ssJsonError << "'caps' 'media_types' #" << ai << " not video whilst 'format' is" ;
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
                        m_ssJsonError << "'caps' 'media_types' #" << ai << " not smpte291 whilst 'format' is data" ;
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
                        m_ssJsonError << "'caps' 'media_types' #" << ai << "not 2022-6 whilst 'format' is mux" ;
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
        else if(JsonMemberExistsAndIsNull(jsData["subscription"], "sender_id"))
        {
            m_sSenderId.clear();
        }
        else
        {
            m_bIsOk = false;
            m_ssJsonError << "'subscription' 'sender_id' neither a string or null" ;
        }
        if(jsData["subscription"]["active"].isBool())
        {
            m_bSenderActive = jsData["subscription"]["active"].asBool();
        }
        else
        {
            m_bIsOk = false;
            m_ssJsonError << "'subscription' 'active' not bool" ;
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

        if(m_sSenderId.empty())
        {
             m_json["subscription"]["sender_id"] = Json::nullValue;
        }
        else
        {
            m_json["subscription"]["sender_id"] = m_sSenderId;
        }
        auto active = m_Active.GetMasterEnable();
        if(active && *active)
        {
            m_json["subscription"]["active"] = true;
        }
        else
        {
            m_json["subscription"]["active"] = false;
        }


        return true;
    }
    return false;
}



Json::Value Receiver::GetConnectionStagedJson(const ApiVersion& version) const
{
    return m_Staged.GetJson();
}


Json::Value Receiver::GetConnectionActiveJson(const ApiVersion& version) const
{
    return m_Active.GetJson();
}



const std::string& Receiver::GetSender() const
{
    return m_sSenderId;
}







bool Receiver::CheckConstraints(const connectionReceiver<activationResponse>& conRequest)
{
    return IOResource::CheckConstraints(conRequest.GetJson());

}


bool Receiver::IsLocked() const
{
    std::lock_guard<std::mutex> lg(m_mutex);
    auto act = m_Staged.GetConstActivation().GetMode();
    return (act == activation::ACT_ABSOLUTE || act == activation::ACT_RELATIVE);
}


connectionReceiver<activationResponse> Receiver::GetStaged() const
{
    std::lock_guard<std::mutex> lg(m_mutex);
    return m_Staged;
}





bool Receiver::IsActivateAllowed() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_bActivateAllowed;
}

void Receiver::CommitActivation()
{
    m_Staged.GetActivation().Clear();

    UpdateVersionTime();

}

activation::enumActivate Receiver::Stage(const connectionReceiver<activationResponse>& conRequest)
{
    std::lock_guard<std::mutex> lg(m_mutex);
    m_Staged = conRequest;
    return m_Staged.GetActivation().GetMode();
}

void Receiver::Activate()
{

    m_bActivateAllowed = false;
    //move the staged parameters to active parameters
    m_Active = m_Staged;

    //Change auto settings to what they actually are
    m_Active.Actualize(m_sInterfaceIp);

    //activeate - set subscription, receiverId and active on master_enable. Commit afterwards
    if(m_Active.GetMasterEnable() && *m_Active.GetMasterEnable())
    {

        auto sender = m_Active.GetSenderId();
        if(sender)
        {
            m_sSenderId = *sender;
            m_bSenderActive = true;
        }
        else
        {
            m_sSenderId.clear();
            m_bSenderActive = false;
        }
    }
    else
    {
        m_sSenderId.clear();
        m_bSenderActive = false;
    }

}

void Receiver::SubscribeToSender(const std::string& sSenderId, const std::string& sSdp, const std::string& sInterfaceIp)
{
    if(sSenderId.empty() == false)
    {
        m_sSenderId = sSenderId;

        // need to update the IS-05 stage and active connection settings to match
        m_Staged.SetSenderId(sSenderId);
        m_Staged.MasterEnable(true);
        m_Staged.SetTransportFile(std::string("application/sdp"), sSdp);
        m_Staged.GetActivation().SetActivationTime(GetTaiTimeNow());
        SetupActivation(sInterfaceIp);
    }
    else
    {   //this means unsubscribe
        m_sSenderId.clear();

        // need to update the IS-05 stage and active connection settings to match
        m_Staged.SetSenderId({});
        m_Staged.MasterEnable(false);
        m_Staged.SetTransportFile({},{});
        m_Staged.GetActivation().SetActivationTime(GetTaiTimeNow());
        SetupActivation(sInterfaceIp);
    }
}

void Receiver::MasterEnable(bool bEnable)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_Active.MasterEnable(bEnable);
    m_Staged.MasterEnable(bEnable);

    //@todo enable the RTP??
    UpdateVersionTime();
}

void Receiver::SetupActivation(const std::string& sInterfaceIp)
{
    m_sInterfaceIp = sInterfaceIp;
}

void Receiver::ActualizeUnitialisedActive(const std::string& sInterfaceIp)
{
    m_sInterfaceIp = sInterfaceIp;
    m_Active.Actualize(m_sInterfaceIp);
}


void Receiver::RemoveStagedActivationTime()
{
    m_Staged.GetActivation().SetActivationTime({});

}

void Receiver::SetStagedActivationTimePoint(const std::chrono::time_point<std::chrono::high_resolution_clock>& tp)
{
    m_Staged.GetActivation().SetActivationTime(tp);
}

