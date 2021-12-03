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



template<typename T> void connectionSender<T>::MasterEnable(bool bEnable)
{
    m_json[MASTER_ENABLE] = bEnable;
}

template<typename T> void connectionReceiver<T>::MasterEnable(bool bEnable)
{
    m_json[MASTER_ENABLE] = bEnable;
}


template<typename T> std::experimental::optional<bool> connectionReceiver<T>::GetMasterEnable() const
{
    return GetBool(m_json, MASTER_ENABLE);
}



template<typename T> std::experimental::optional<bool> connectionSender<T>::GetMasterEnable() const
{
    return GetBool(m_json, MASTER_ENABLE);
}


template<typename T> connectionSender<T>::connectionSender(std::experimental::optional<bool> masterEnable,TransportParamsRTP::flagsTP allowed)
{
    if(masterEnable)
    {
        m_json[MASTER_ENABLE] = (*masterEnable);
    }

    m_json[RECEIVER_ID] = Json::Value::null;

    m_vTransportParams.push_back(TransportParamsRTPSender(allowed));
    if(allowed & TransportParamsRTP::REDUNDANT)
    {
        m_vTransportParams.push_back(TransportParamsRTPSender(allowed));
    }
    SetTPAllowed(allowed);
}


template<typename T> connectionSender<T>::connectionSender(const connectionSender<T>& conReq) :
    m_json(conReq.GetJsonToCopy()),
    m_activation(conReq.GetConstActivation()),
    m_vTransportParams(conReq.GetTransportParams())
{

}

template<typename T> connectionSender<T>& connectionSender<T>::operator=(const connectionSender<T>& other)
{
    m_json = other.GetJsonToCopy();
    m_vTransportParams = other.GetTransportParams();
    m_activation = other.GetConstActivation();
    return *this;
}

template<typename T> connectionSender<T>::connectionSender(const Json::Value& jsResponse)
{
    if(connectionSender<T>::CheckJson(jsResponse) == false)
    {
        return;
    }

    if(jsResponse.isMember(RECEIVER_ID))
    {
        m_json[RECEIVER_ID] = jsResponse[RECEIVER_ID];
    }
    if(jsResponse.isMember(MASTER_ENABLE))
    {
        m_json[MASTER_ENABLE] = jsResponse[MASTER_ENABLE];
    }

     if(jsResponse[ACTIVATION].isObject())
    {
        m_activation = T(jsResponse[ACTIVATION]);
    }

    if(jsResponse.isMember(TRANSPORT_PARAMS) && jsResponse[TRANSPORT_PARAMS].isArray())
    {
        m_vTransportParams.clear();
        for(Json::ArrayIndex i = 0; i < jsResponse[TRANSPORT_PARAMS].size(); i++)
        {
            m_vTransportParams.push_back(TransportParamsRTPSender(jsResponse[TRANSPORT_PARAMS][i]));
        }
    }
}



template<typename T> bool connectionSender<T>::Patch(const Json::Value& jsData)
{
    pmlLog(pml::LOG_DEBUG) << "NMOS: " << "Patch: connectionSender: NOW" ;
    pmlLog(pml::LOG_DEBUG) << jsData;

    if(connectionSender<T>::CheckJson(jsData) == false)
    {
        return false;
    }

    if(jsData[ACTIVATION].isObject() && m_activation.Patch(jsData[ACTIVATION]) == false)
    {
        return false;
    }

    if(jsData.isMember(MASTER_ENABLE))
    {
        m_json[MASTER_ENABLE] = jsData[MASTER_ENABLE];
    }

    if(jsData.isMember(TRANSPORT_PARAMS) && jsData[TRANSPORT_PARAMS].isArray())
    {
        if(jsData[TRANSPORT_PARAMS].size() != m_vTransportParams.size())
        {
            return false;
        }
        for(Json::ArrayIndex i = 0; i < jsData[TRANSPORT_PARAMS].size(); i++)
        {
            if(m_vTransportParams[i].Patch(jsData[TRANSPORT_PARAMS][i]) == false)
            {
                return false;
            }
        }
    }
    if(jsData.isMember(RECEIVER_ID))
    {
        m_json[RECEIVER_ID] = jsData[RECEIVER_ID];
    }
    return true;
}

template<typename T> Json::Value connectionSender<T>::GetJson() const
{
    auto jsConnect = m_json;
    jsConnect[ACTIVATION] = m_activation.GetJson();

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

template<typename T> connectionReceiver<T>::connectionReceiver(std::experimental::optional<bool> masterEnable, TransportParamsRTP::flagsTP allowed)
{
    if(masterEnable)
    {
        m_json[MASTER_ENABLE] = (*masterEnable);
    }
    m_json[SENDER_ID] = Json::Value::null;
    m_json[TRANSPORT_FILE][TRANSPORT_FILE_DATA] = Json::Value::null;
    m_json[TRANSPORT_FILE][TRANSPORT_FILE_TYPE] = Json::Value::null;

    m_vTransportParams.push_back(TransportParamsRTPReceiver(allowed));
    if((allowed & TransportParamsRTP::REDUNDANT))
    {
        pmlLog() << "NMOS: connectionReceiver: Redundant stream";
        m_vTransportParams.push_back(TransportParamsRTPReceiver(allowed));
    }
    SetTPAllowed(allowed);
}


template<typename T> connectionReceiver<T>::connectionReceiver(const connectionReceiver<T>& conReq) : m_json(conReq.GetJson()),
    m_activation(conReq.GetConstActivation()),
    m_vTransportParams(conReq.GetTransportParams())
{

}

template<typename T> connectionReceiver<T>::connectionReceiver(const Json::Value& jsResponse)
{
    if(connectionReceiver<T>::CheckJson(jsResponse) == false)
    {
        return;
    }

    if(jsResponse.isMember(MASTER_ENABLE))
    {
        m_json[MASTER_ENABLE] = jsResponse[MASTER_ENABLE];
    }

    if(jsResponse.isMember(TRANSPORT_FILE) && jsResponse[TRANSPORT_FILE].isObject())
    {
        m_json[TRANSPORT_FILE] = jsResponse[TRANSPORT_FILE];

        auto type = GetString(m_json[TRANSPORT_FILE], TRANSPORT_FILE_TYPE);
        auto data = GetString(m_json[TRANSPORT_FILE], TRANSPORT_FILE_DATA);
    }

    if(jsResponse.isMember(SENDER_ID))
    {
        m_json[SENDER_ID] = jsResponse[SENDER_ID];
    }


    if(jsResponse[ACTIVATION].isObject())
    {
        m_activation = T(jsResponse[ACTIVATION]);
    }


    if(jsResponse.isMember(TRANSPORT_PARAMS) && jsResponse[TRANSPORT_PARAMS].isArray())
    {
        m_vTransportParams.clear();
        for(Json::ArrayIndex i = 0; i < jsResponse[TRANSPORT_PARAMS].size(); i++)
        {
            m_vTransportParams.push_back(TransportParamsRTPReceiver(jsResponse[TRANSPORT_PARAMS][i]));
        }
    }
}

template<typename T> bool connectionReceiver<T>::Patch(const Json::Value& jsData)
{
    pmlLog(pml::LOG_DEBUG) << "NMOS: " << "Patch: connectionReceiver: NOW" ;
    pmlLog(pml::LOG_DEBUG) << jsData;

    if(connectionReceiver<T>::CheckJson(jsData) == false)
    {
        pmlLog() << "CheckJson failed";
        return false;
    }


    if(jsData[ACTIVATION].isObject() && m_activation.Patch(jsData[ACTIVATION]) == false)
    {
        return false;
    }

    if(jsData.isMember(MASTER_ENABLE))
    {
        m_json[MASTER_ENABLE] = jsData[MASTER_ENABLE];
    }


    if(jsData.isMember(TRANSPORT_FILE) && jsData[TRANSPORT_FILE].isObject())
    {
        m_json[TRANSPORT_FILE] = jsData[TRANSPORT_FILE];

        auto type = GetString(m_json[TRANSPORT_FILE], TRANSPORT_FILE_TYPE);
        auto data = GetString(m_json[TRANSPORT_FILE], TRANSPORT_FILE_DATA);
        if(type && *type == TRANSPORT_FILE_SDP && data)
        {
            SdpManager::SdpToTransportParams(*data, m_vTransportParams);
        }
    }

    if(jsData.isMember(TRANSPORT_PARAMS) && jsData[TRANSPORT_PARAMS].isArray())
    {
        if(jsData[TRANSPORT_PARAMS].size() != m_vTransportParams.size())
        {
            pmlLog() << "Transport param array";
            return false;
        }
        for(Json::ArrayIndex i = 0; i < jsData[TRANSPORT_PARAMS].size(); i++)
        {
            if(m_vTransportParams[i].Patch(jsData[TRANSPORT_PARAMS][i]) == false)
            {
                pmlLog() << "Transport param patch failed";
                return false;
            }
        }
    }
    if(jsData.isMember(SENDER_ID))
    {
        m_json[SENDER_ID] = jsData[SENDER_ID];
    }
    return true;
}



template<typename T> Json::Value connectionReceiver<T>::GetJson()  const
{
    auto jsConnect = m_json;
    jsConnect[ACTIVATION] = m_activation.GetJson();

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
    return GetString(m_json, RECEIVER_ID);
}

template<typename T> std::experimental::optional<std::string> connectionReceiver<T>::GetSenderId() const
{
    return GetString(m_json, SENDER_ID);
}

template<typename T> std::experimental::optional<std::string> connectionReceiver<T>::GetTransportFileType() const
{
    return GetString(m_json[TRANSPORT_FILE], TRANSPORT_FILE_TYPE);
}

template<typename T> std::experimental::optional<std::string> connectionReceiver<T>::GetTransportFileData() const
{
    return GetString(m_json[TRANSPORT_FILE], TRANSPORT_FILE_DATA);
}

template<typename T> void connectionReceiver<T>::SetSenderId(const std::experimental::optional<std::string>& id)
{
    if(id)
    {
        m_json[SENDER_ID] = *id;
    }
    else
    {
        m_json[SENDER_ID] = Json::Value::null;
    }
}

template<typename T> void connectionReceiver<T>::SetTransportFile(const std::experimental::optional<std::string>& type, const std::experimental::optional<std::string>& data)
{
    if(type)
    {
        m_json[TRANSPORT_FILE][TRANSPORT_FILE_TYPE] = *type;
        if(data)
        {
            m_json[TRANSPORT_FILE][TRANSPORT_FILE_DATA] = *data;
        }
        else
        {
            m_json[TRANSPORT_FILE][TRANSPORT_FILE_DATA] = Json::Value::null;
        }
    }
    else
    {
        m_json[TRANSPORT_FILE][TRANSPORT_FILE_TYPE] = Json::Value::null;
        m_json[TRANSPORT_FILE][TRANSPORT_FILE_DATA] = Json::Value::null;
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
    m_json = other.GetJsonToCopy();
    m_vTransportParams = other.GetTransportParams();
    m_activation = other.GetConstActivation();
    return *this;
}


template<typename T> bool connectionSender<T>::CheckJson(const Json::Value& jsPatch)
{
    if (CheckJsonAllowed(jsPatch, {{RECEIVER_ID, {jsondatatype::_STRING, jsondatatype::_NULL}},
                                 {MASTER_ENABLE, {jsondatatype::_BOOLEAN}},
                                 {ACTIVATION , {jsondatatype::_OBJECT}},
                                 {TRANSPORT_PARAMS, {jsondatatype::_ARRAY}}}) == false)
    {
        return false;
    }

    if(jsPatch.isMember(ACTIVATION) && T::CheckJson(jsPatch[ACTIVATION]) == false)
    {
        return false;
    }
    if(jsPatch.isMember(TRANSPORT_PARAMS))
    {
        for(Json::ArrayIndex ai = 0; ai < jsPatch[TRANSPORT_PARAMS].size(); ai++)
        {
            if(TransportParamsRTPSender::CheckJson(jsPatch[TRANSPORT_PARAMS][ai]) == false)
            {
                return false;
            }
        }
    }
    return true;
}

template<typename T> bool connectionReceiver<T>::CheckJson(const Json::Value& jsPatch)
{
    if (CheckJsonAllowed(jsPatch, {{SENDER_ID, {jsondatatype::_STRING, jsondatatype::_NULL}},
                                 {MASTER_ENABLE, {jsondatatype::_BOOLEAN}},
                                 {ACTIVATION , {jsondatatype::_OBJECT}},
                                 {TRANSPORT_PARAMS, {jsondatatype::_ARRAY}},
                                 {TRANSPORT_FILE, {jsondatatype::_OBJECT}}}) == false)
    {
        return false;
    }
    if(jsPatch.isMember(ACTIVATION) && T::CheckJson(jsPatch[ACTIVATION]) == false)
    {
        return false;
    }
    if(jsPatch.isMember(TRANSPORT_FILE) && CheckJsonAllowed(jsPatch[TRANSPORT_FILE], {{TRANSPORT_FILE_DATA, {jsondatatype::_STRING, jsondatatype::_NULL}},{TRANSPORT_FILE_TYPE, {jsondatatype::_STRING, jsondatatype::_NULL}}}) == false)
    {
        return false;
    }
    if(jsPatch.isMember(TRANSPORT_PARAMS))
    {
        for(Json::ArrayIndex ai = 0; ai < jsPatch[TRANSPORT_PARAMS].size(); ai++)
        {
            if(TransportParamsRTPReceiver::CheckJson(jsPatch[TRANSPORT_PARAMS][ai]) == false)
            {
                return false;
            }
        }
    }
    return true;
}

template<typename T> void connectionSender<T>::EnableTransport(size_t nTP, bool bEnable)
{
    if(nTP < m_vTransportParams.size())
    {
        m_vTransportParams[nTP].EnableRtp(bEnable);
    }
}

template<typename T> void connectionReceiver<T>::EnableTransport(size_t nTP, bool bEnable)
{
    if(nTP < m_vTransportParams.size())
    {
        m_vTransportParams[nTP].EnableRtp(bEnable);
    }
}



template class connectionSender<activationRequest>;
template class connectionSender<activationResponse>;
template class connectionReceiver<activationResponse>;
template class connectionReceiver<activationRequest>;
