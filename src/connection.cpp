#include "connection.h"
#include "log.h"
#include "sdp.h"
#include "utils.h"
#include "activation.h"

using namespace pml::nmos;
const std::string STR_ACTIVATE[4] = {"", "activate_immediate", "activate_scheduled_absolute", "activate_scheduled_relative"};

const std::string MASTER_ENABLE = "master_enable";
const std::string RECEIVER_ID = "receiver_id";
const std::string SENDER_ID = "sender_id";
const std::string TRANSPORT_PARAMS = "transport_params";
const std::string TRANSPORT_FILE = "transport_file";
const std::string TRANSPORT_FILE_DATA = "data";
const std::string TRANSPORT_FILE_TYPE = "type";
const std::string TRANSPORT_FILE_SDP  = "application/sdp";
const std::string ACTIVATION = "activation";


template<typename T> connection<T>::connection(std::experimental::optional<bool> masterEnable)
{
    if(masterEnable)
    {
        m_json[MASTER_ENABLE] = (*masterEnable);
    }
}


template<typename T> connection<T>::connection(const connection<T>& conReq) :
    m_json(conReq.GetJson())
{

}

template<typename T> connection<T>& connection<T>::operator=(const connection<T>& con)
{
    m_json = con.GetJson();
    return *this;
}

template<typename T> void connection<T>::MasterEnable(bool bEnable)
{
    m_json[MASTER_ENABLE] = bEnable;
}

template<typename T> bool connection<T>::Patch(const Json::Value& jsData)
{
    pmlLog(pml::LOG_DEBUG) << "NMOS: " << "Patch: connection" ;

    auto jsTemp = m_json;

    if(!DoPatch(jsData))
    {
        m_json = jsTemp;
        return false;
    }
    return true;
}

template<typename T> bool connection<T>::DoPatch(const Json::Value& jsData)
{
    if(CheckJsonOptional(jsData, {{MASTER_ENABLE, {jsondatatype::_BOOLEAN}}}) == false)
    {
        return false;
    }

    if(jsData[ACTIVATION].isObject() && m_activation.Patch(jsData[ACTIVATION]) == false)
    {
        return false;
    }

    m_json[MASTER_ENABLE] = jsData[MASTER_ENABLE];

    return true;
}


template<typename T> Json::Value connection<T>::GetJson() const
{
    Json::Value jsConnect = m_json;
    jsConnect[ACTIVATION] = m_activation.GetJson();

    return jsConnect;
}


template<typename T> std::experimental::optional<bool> connection<T>::GetMasterEnable() const
{
    return GetBool(m_json, MASTER_ENABLE);
}


template<typename T> connectionSender<T>::connectionSender(std::experimental::optional<bool> masterEnable, size_t nTPLegs) :
  connection<T>(masterEnable),
  m_vTransportParams(nTPLegs)
{
    connection<T>::m_json[RECEIVER_ID] = Json::Value::null;
}


template<typename T> connectionSender<T>::connectionSender(const connectionSender<T>& conReq) : connection<T>(conReq),
    m_vTransportParams(conReq.GetTransportParams())
{

}

template<typename T> connectionSender<T>& connectionSender<T>::operator=(const connectionSender<T>& other)
{
    connection<T>::m_json = other.GetJson();
    m_vTransportParams = other.GetTransportParams();
    return *this;
}

template<typename T> bool connectionSender<T>::DoPatch(const Json::Value& jsData)
{
    pmlLog(pml::LOG_DEBUG) << "NMOS: " << "Patch: connectionSender: NOW" ;
    pmlLog(pml::LOG_DEBUG) << jsData;

    if(CheckJsonOptional(jsData, {{TRANSPORT_PARAMS, {jsondatatype::_ARRAY}},
                                  {RECEIVER_ID, {jsondatatype::_STRING, jsondatatype::_NULL}}}) == false)
    {
        return false;
    }
    if(!connection<T>::DoPatch(jsData))
    {
        return false;
    }

    if(jsData[TRANSPORT_PARAMS].isArray())
    {
        if(jsData[TRANSPORT_PARAMS].size() != m_vTransportParams.size())
        {
            return false;
        }
        for(size_t i = 0; i < jsData[TRANSPORT_PARAMS].size(); i++)
        {
            if(m_vTransportParams[i].Patch(jsData[TRANSPORT_PARAMS][i]) == false)
            {
                return false;
            }
        }
    }

    if(jsData.isMember(RECEIVER_ID))
    {
        connection<T>::m_json[RECEIVER_ID] = jsData[RECEIVER_ID];
    }
    return true;
}

template<typename T> Json::Value connectionSender<T>::GetJson() const
{
    Json::Value jsConnect(connection<T>::GetJson());

    if(m_vTransportParams.empty() == false)
    {
        jsConnect[TRANSPORT_PARAMS] = Json::arrayValue;
        for(const auto& tpSender : m_vTransportParams)
        {
            jsConnect[TRANSPORT_PARAMS].append(tpSender.GetJson());
        }
    }
    return jsConnect;
}

template<typename T> void connectionSender<T>::Actualize(const std::string& sSourceIp, const std::string& sDestinationIp)
{
    for(auto& tpSender : m_vTransportParams)
    {
        tpSender.Actualize(sSourceIp, sDestinationIp);
    }
}

template<typename T> void connectionSender<T>::SetTPAllowed(int flagsTransport)
{
    for(auto& tpSender : m_vTransportParams)
    {
        if(flagsTransport & TransportParamsRTP::FEC)
        {
            pmlLog(pml::LOG_DEBUG) << "NMOS: ConnectionSender: Gec Allowed";
            tpSender.FecAllowed();
        }
        if(flagsTransport & TransportParamsRTP::RTCP)
        {
            tpSender.RtcpAllowed();
        }
    }
}

template<typename T> connectionReceiver<T>::connectionReceiver(std::experimental::optional<bool> masterEnable, size_t nTPLegs) :
  connection<T>(masterEnable),
  m_vTransportParams(nTPLegs)
{
    connection<T>::m_json[SENDER_ID] = Json::Value::null;
    connection<T>::m_json[TRANSPORT_FILE][TRANSPORT_FILE_DATA] = Json::Value::null;
    connection<T>::m_json[TRANSPORT_FILE][TRANSPORT_FILE_TYPE] = Json::Value::null;
}


template<typename T> connectionReceiver<T>::connectionReceiver(const connectionReceiver<T>& conReq) : connection<T>(conReq),
    m_vTransportParams(conReq.GetTransportParams())
{

}

template<typename T> bool connectionReceiver<T>::DoPatch(const Json::Value& jsData)
{
    if(CheckJsonOptional(jsData, {{TRANSPORT_PARAMS,{jsondatatype::_ARRAY}},
                                  {SENDER_ID,{jsondatatype::_STRING, jsondatatype::_NULL}},
                                  {TRANSPORT_FILE, {jsondatatype::_OBJECT}}}) == false)
    {
        return false;
    }
    if(!connection<T>::DoPatch(jsData))
    {
        return false;
    }

    if(jsData[TRANSPORT_FILE].isObject())
    {
        connection<T>::m_json[TRANSPORT_FILE] == jsData[TRANSPORT_FILE];

        auto type = GetString(connection<T>::m_json[TRANSPORT_FILE], TRANSPORT_FILE_TYPE);
        auto data = GetString(connection<T>::m_json[TRANSPORT_FILE], TRANSPORT_FILE_DATA);
        if(type && *type == TRANSPORT_FILE_SDP && data)
        {
            SdpManager::SdpToTransportParams(*data, m_vTransportParams);
        }
    }

    if(jsData[TRANSPORT_PARAMS].isArray())
    {
        if(jsData[TRANSPORT_PARAMS].size() != m_vTransportParams.size())
        {
            return false;
        }
        for(size_t i = 0; i < jsData[TRANSPORT_PARAMS].size(); i++)
        {
            if(m_vTransportParams[i].Patch(jsData[TRANSPORT_PARAMS][i]) == false)
            {
                return false;
            }
        }
    }
    if(jsData.isMember(SENDER_ID))
    {
        connection<T>::m_json[SENDER_ID] = SENDER_ID;
    }
    return true;
}



template<typename T> Json::Value connectionReceiver<T>::GetJson()  const
{
    Json::Value jsConnect(connection<T>::GetJson());

    if(m_vTransportParams.empty() == false)
    {
        jsConnect[TRANSPORT_PARAMS] = Json::arrayValue;
        for(const auto& tpReceiver : m_vTransportParams)
        {
            jsConnect[TRANSPORT_PARAMS].append(tpReceiver.GetJson());
        }
    }
    return jsConnect;
}


template<typename T> void connectionReceiver<T>::Actualize(const std::string& sInterfaceIp)
{
    for(auto& tpReceiver : m_vTransportParams)
    {
        tpReceiver.Actualize(sInterfaceIp);
    }
}

template<typename T> void connectionReceiver<T>::SetTPAllowed(int flagsTransport)
{
    for(auto& tpReceiver : m_vTransportParams)
    {
        if(flagsTransport & TransportParamsRTP::FEC)
        {
            tpReceiver.FecAllowed();
        }
        if(flagsTransport & TransportParamsRTP::RTCP)
        {
            tpReceiver.RtcpAllowed();
        }
        if(flagsTransport & TransportParamsRTP::MULTICAST)
        {
            tpReceiver.SetMulticastIp(std::string());
        }
    }
}


template<typename T> std::experimental::optional<std::string> connectionSender<T>::GetReceiverId() const
{
    return GetString(connection<T>::m_json, SENDER_ID);
}

template<typename T> std::experimental::optional<std::string> connectionReceiver<T>::GetSenderId() const
{
    return GetString(connection<T>::m_json, RECEIVER_ID);
}

template<typename T> std::experimental::optional<std::string> connectionReceiver<T>::GetTransportFileType() const
{
    return GetString(connection<T>::m_json[TRANSPORT_FILE], TRANSPORT_FILE_TYPE);
}

template<typename T> std::experimental::optional<std::string> connectionReceiver<T>::GetTransportFileData() const
{
    return GetString(connection<T>::m_json[TRANSPORT_FILE], TRANSPORT_FILE_DATA);
}

template<typename T> void connectionReceiver<T>::SetSenderId(const std::experimental::optional<std::string>& id)
{
    if(id)
    {
        connection<T>::m_json[SENDER_ID] = *id;
    }
    else
    {
        connection<T>::m_json[SENDER_ID] = Json::Value::null;
    }
}

template<typename T> void connectionReceiver<T>::SetTransportFile(const std::experimental::optional<std::string>& type, const std::experimental::optional<std::string>& data)
{
    if(type)
    {
        connection<T>::m_json[TRANSPORT_FILE][TRANSPORT_FILE_TYPE] = *type;
        if(data)
        {
            connection<T>::m_json[TRANSPORT_FILE][TRANSPORT_FILE_DATA] = *data;
        }
        else
        {
            connection<T>::m_json[TRANSPORT_FILE][TRANSPORT_FILE_DATA] = Json::Value::null;
        }
    }
    else
    {
        connection<T>::m_json[TRANSPORT_FILE][TRANSPORT_FILE_TYPE] = Json::Value::null;
        connection<T>::m_json[TRANSPORT_FILE][TRANSPORT_FILE_DATA] = Json::Value::null;
    }
}

template<typename T>void connectionSender<T>::SetDestinationDetails(const std::string& sIp, unsigned short nPort)
{
    for(auto& tpSender : m_vTransportParams)
    {
        tpSender.SetDestinationIp(sIp);
        tpSender.SetDestinationPort(nPort);
    }
}

template<typename T> connectionReceiver<T>& connectionReceiver<T>::operator=(const connectionReceiver<T>& other)
{
    connection<T>::m_json = other.GetJson();
    m_vTransportParams = other.GetTransportParams();
    return *this;
}



template class connection<activationRequest>;
template class connection<activationResponse>;
template class connectionSender<activationRequest>;
template class connectionSender<activationResponse>;
template class connectionReceiver<activationResponse>;
template class connectionReceiver<activationRequest>;
