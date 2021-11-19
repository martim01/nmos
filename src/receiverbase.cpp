#include "receiverbase.h"
#include "senderbase.h"
#include <chrono>
#include "eventposter.h"
#include "connection.h"

#include "log.h"
#include "utils.h"

using namespace pml::nmos;


const std::array<std::string,4> ReceiverBase::STR_TYPE = {"urn:x-nmos:format:audio", "urn:x-nmos:format:video", "urn:x-nmos:format:data", "urn:x-nmos:format:mux"};



ReceiverBase::ReceiverBase(const std::string& sLabel, const std::string& sDescription, enumTransport eTransport, const std::string& sDeviceId, enumType eFormat, int flagsTransport) :
    IOResource("receiver", sLabel, sDescription, eTransport),
    m_sDeviceId(sDeviceId),
    m_sSenderId(""),
    m_bSenderActive(false),
    m_eType(eFormat),
    m_vConstraints(1),
    m_bActivateAllowed(false)
{
    if(flagsTransport & TransportParamsRTP::REDUNDANT)
    {
        m_vConstraints.resize(2);
        m_Staged.tpReceivers.resize(2);
        m_Active.tpReceivers.resize(2);
    }

    for(size_t i = 0; i < m_vConstraints.size(); i++)
    {
        if(flagsTransport & TransportParamsRTP::FEC)
        {
            m_Staged.tpReceivers[i].eFec = TransportParamsRTP::TP_SUPPORTED;
            m_Active.tpReceivers[i].eFec = TransportParamsRTP::TP_SUPPORTED;
        }
        else
        {
            m_Staged.tpReceivers[i].eFec = TransportParamsRTP::TP_NOT_SUPPORTED;
            m_Active.tpReceivers[i].eFec = TransportParamsRTP::TP_NOT_SUPPORTED;
        }

        if(flagsTransport & TransportParamsRTP::RTCP)
        {
            m_Staged.tpReceivers[i].eRTCP = TransportParamsRTP::TP_SUPPORTED;
            m_Active.tpReceivers[i].eRTCP = TransportParamsRTP::TP_SUPPORTED;
        }
        else
        {
            m_Staged.tpReceivers[i].eRTCP = TransportParamsRTP::TP_NOT_SUPPORTED;
            m_Active.tpReceivers[i].eRTCP = TransportParamsRTP::TP_NOT_SUPPORTED;
        }
        if(flagsTransport & TransportParamsRTP::MULTICAST)
        {
            m_Staged.tpReceivers[i].eMulticast = TransportParamsRTP::TP_SUPPORTED;
            m_Active.tpReceivers[i].eMulticast = TransportParamsRTP::TP_SUPPORTED;
        }
        else
        {
            m_Staged.tpReceivers[i].eMulticast = TransportParamsRTP::TP_NOT_SUPPORTED;
            m_Active.tpReceivers[i].eMulticast = TransportParamsRTP::TP_NOT_SUPPORTED;
        }
        m_Active.tpReceivers[i].nDestinationPort = 5004;
        m_Active.tpReceivers[i].sInterfaceIp = "127.0.0.1";

        m_vConstraints[i].nParamsSupported = flagsTransport;
    }
}

ReceiverBase::~ReceiverBase()
{

}

ReceiverBase::ReceiverBase() : IOResource("receiver")
{

}



bool ReceiverBase::AddCap(const std::string& sCap)
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

void ReceiverBase::RemoveCap(const std::string& sCap)
{
    m_setCaps.erase(sCap);
    UpdateVersionTime();
}




void ReceiverBase::SetType(enumType eType)
{
    m_eType = eType;
    UpdateVersionTime();
}

bool ReceiverBase::UpdateFromJson(const Json::Value& jsData)
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
bool ReceiverBase::Commit(const ApiVersion& version)
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
        m_json["subscription"]["active"] = m_Active.bMasterEnable;


        return true;
    }
    return false;
}



Json::Value ReceiverBase::GetConnectionStagedJson(const ApiVersion& version) const
{
    return m_Staged.GetJson(version);
}


Json::Value ReceiverBase::GetConnectionActiveJson(const ApiVersion& version) const
{
    return m_Active.GetJson(version);
}



const std::string& ReceiverBase::GetSender() const
{
    return m_sSenderId;
}





Json::Value ReceiverBase::GetConnectionConstraintsJson(const ApiVersion& version) const
{
    Json::Value jsArray(Json::arrayValue);
    for(const auto& con : m_vConstraints)
    {
        jsArray.append(con.GetJson(version));
    }
    return jsArray;
}


bool ReceiverBase::CheckConstraints(const connectionReceiver& conRequest)
{
    bool bMeets = (m_vConstraints.size() == conRequest.tpReceivers.size());
    if(bMeets)
    {
        for(size_t i = 0; i < m_vConstraints.size(); i++)
        {
            bMeets &= m_vConstraints[i].destination_port.MeetsConstraint(conRequest.tpReceivers[i].nDestinationPort);
            bMeets &= m_vConstraints[i].fec_destination_ip.MeetsConstraint(conRequest.tpReceivers[i].sFecDestinationIp);
            bMeets &= m_vConstraints[i].fec_enabled.MeetsConstraint(conRequest.tpReceivers[i].bFecEnabled);
            bMeets &= m_vConstraints[i].fec_mode.MeetsConstraint(TransportParamsRTP::STR_FEC_MODE[conRequest.tpReceivers[i].eFecMode]);
            bMeets &= m_vConstraints[i].fec1D_destination_port.MeetsConstraint(conRequest.tpReceivers[i].nFec1DDestinationPort);
            bMeets &= m_vConstraints[i].fec2D_destination_port.MeetsConstraint(conRequest.tpReceivers[i].nFec2DDestinationPort);
            bMeets &= m_vConstraints[i].rtcp_destination_ip.MeetsConstraint(conRequest.tpReceivers[i].sRtcpDestinationIp);
            bMeets &= m_vConstraints[i].rtcp_destination_port.MeetsConstraint(conRequest.tpReceivers[i].nRtcpDestinationPort);
            bMeets &= m_vConstraints[i].interface_ip.MeetsConstraint(conRequest.tpReceivers[i].sInterfaceIp);
        }
    }
    return bMeets;
}


bool ReceiverBase::IsLocked() const
{
    std::lock_guard<std::mutex> lg(m_mutex);
    return (m_Staged.eActivate == connection::ACT_ABSOLUTE || m_Staged.eActivate == connection::ACT_RELATIVE);
}


connectionReceiver ReceiverBase::GetStaged() const
{
    std::lock_guard<std::mutex> lg(m_mutex);
    return m_Staged;
}



bool ReceiverBase::IsMasterEnabled() const
{
    return m_Active.bMasterEnable;
}


bool ReceiverBase::IsActivateAllowed() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_bActivateAllowed;
}