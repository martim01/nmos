#include "transportparams.h"
#include "log.h"
#include "utils.h"

using namespace pml::nmos;

//const std::string TransportParamsRTP::STR_FEC_MODE[2] = {"1D", "2D"};
const std::string TransportParamsRTP::STR_FEC_TYPE_XOR  = "XOR";
const std::string TransportParamsRTP::STR_FEC_TYPE_REED = "Reed-Solomon";
const std::string TransportParamsRTP::AUTO                  = "auto";
const std::string TransportParamsRTP::SOURCE_IP             = "source_ip";
const std::string TransportParamsRTP::SOURCE_PORT           = "source_port";
const std::string TransportParamsRTP::DESTINATION_PORT      = "destination_port";
const std::string TransportParamsRTP::DESTINATION_IP        = "destination_ip";
const std::string TransportParamsRTP::MULTICAST_IP          = "multicast_ip";
const std::string TransportParamsRTP::INTERFACE_IP          = "interface_ip";
const std::string TransportParamsRTP::RTP_ENABLED           = "rtp_enabled";
const std::string TransportParamsRTP::FEC_ENABLED           = "fec_enabled";
const std::string TransportParamsRTP::FEC_DESTINATION_IP    = "fec_destination_ip";
const std::string TransportParamsRTP::FEC_MODE              = "fec_mode";
const std::string TransportParamsRTP::FEC_1DDESTINATION_PORT= "fec1D_destination_port";
const std::string TransportParamsRTP::FEC_2DDESTINATION_PORT= "fec2D_destination_port";
const std::string TransportParamsRTP::FEC_TYPE              = "fec_type";
const std::string TransportParamsRTP::FEC_BLOCK_WIDTH       = "fec_block_width";
const std::string TransportParamsRTP::FEC_BLOCK_HEIGHT      = "fec_block_height";
const std::string TransportParamsRTP::FEC_1DSOURCE_PORT     = "fec1D_source_port";
const std::string TransportParamsRTP::FEC_2DSOURCE_PORT     = "fec2D_source_port";
const std::string TransportParamsRTP::RTCP_ENABLED          = "rtcp_enabled";
const std::string TransportParamsRTP::RTCP_DESTINATION_IP   = "rtcp_destination_ip";
const std::string TransportParamsRTP::RTCP_DESTINATION_PORT = "rtcp_destination_port";
const std::string TransportParamsRTP::RTCP_SOURCE_PORT      = "rtcp_source_port";



TransportParams::TransportParams(const TransportParams& tp) :
    m_json(tp.GetJson())
{

}


TransportParams& TransportParams::operator=(const TransportParams& other)
{
    if(&other != this)
    {
        m_json = other.GetJson();
    }
    return *this;
}




TransportParamsRTP::TransportParamsRTP() : TransportParams()
{
    m_json[DESTINATION_PORT] = AUTO;
    m_json[RTP_ENABLED] = false;

}


TransportParamsRTP::TransportParamsRTP(const TransportParamsRTP& tp) : TransportParams(tp)
{

}


TransportParamsRTP& TransportParamsRTP::operator=(const TransportParamsRTP& other)
{
    TransportParams::operator=(other);
    return *this;
}

void TransportParamsRTP::SetSourceIp(const std::string& sAddress)
{
    if(sAddress.empty())
    {
        m_json[SOURCE_IP] = Json::Value::null;
    }
    else
    {
        m_json[SOURCE_IP] = sAddress;
    }
}

void TransportParamsRTP::SetPort(const std::string& sKey, unsigned short nPort)
{
    if(m_json.isMember(sKey))
    {
        if(nPort != 0)
        {
            m_json[sKey] = nPort;
        }
        else
        {
            m_json[sKey] = AUTO;
        }
    }
}

void TransportParamsRTP::SetRtcpDestinationPort(unsigned short nPort)
{
    SetPort(RTCP_DESTINATION_PORT, nPort);
}

void TransportParamsRTP::SetDestinationPort(unsigned short nPort)
{
        SetPort(DESTINATION_PORT, nPort);
}

void TransportParamsRTP::SetRtcpDestinationIp(const std::string& sAddress)
{
    if(m_json.isMember(RTCP_DESTINATION_IP))
    {
        if(sAddress.empty())
        {
            m_json[RTCP_DESTINATION_IP] = AUTO;
        }
        else
        {
            m_json[RTCP_DESTINATION_IP] = sAddress;
        }
    }
}

void TransportParamsRTP::FecAllowed()
{
    m_json[FEC_ENABLED] = false;
    m_json[FEC_DESTINATION_IP] = AUTO;
    m_json[FEC_MODE] = AUTO;
    m_json[FEC_1DDESTINATION_PORT] = AUTO;
    m_json[FEC_2DDESTINATION_PORT] = AUTO;


}

void TransportParamsRTP::RtcpAllowed()
{
    m_json[RTCP_ENABLED] = false;
    m_json[RTCP_DESTINATION_PORT] = AUTO;
    m_json[RTCP_DESTINATION_IP] = AUTO;

}



std::string TransportParamsRTP::GetSourceIP() const
{
    if(m_json[SOURCE_IP].isString())
    {
        m_json[SOURCE_IP].asString();
    }
    return std::string();
}

unsigned short TransportParamsRTP::GetDestinationPort() const
{
    return m_json[DESTINATION_PORT].asUInt();
}

bool TransportParamsRTP::IsRtpEnabled() const
{
    return m_json[RTP_ENABLED].asBool();
}

std::experimental::optional<bool> TransportParamsRTP::IsFecEnabled() const
{
    return GetBool(m_json, FEC_ENABLED);
}

std::experimental::optional<std::string> TransportParamsRTP::GetFecDestinationIp() const
{
    return GetString(m_json, FEC_DESTINATION_IP);
}

std::experimental::optional<std::string> TransportParamsRTP::GetFecMode() const
{
    return GetString(m_json, FEC_MODE);
}

std::experimental::optional<uint32_t> TransportParamsRTP::GetFec1DDestinationPort() const
{
    return GetUInt(m_json, FEC_1DDESTINATION_PORT);
}

std::experimental::optional<uint32_t> TransportParamsRTP::GetFec2DDestinationPort() const
{
    return GetUInt(m_json, FEC_2DDESTINATION_PORT);
}

std::experimental::optional<bool> TransportParamsRTP::IsRtcpEnabled() const
{
    return GetBool(m_json, RTCP_ENABLED);
}

std::experimental::optional<std::string> TransportParamsRTP::GetRtcpDestinationIp() const
{
    return GetString(m_json, RTCP_DESTINATION_IP);
}

std::experimental::optional<uint32_t> TransportParamsRTP::GetRtcpDestinationPort() const
{
    return GetUInt(m_json, RTCP_DESTINATION_PORT);
}


void TransportParamsRTP::ActualizePort(const std::string& sKey, unsigned short nPort)
{
    auto port = GetInt(m_json, sKey);
    if(!port || *port == 0)
    {
        m_json[sKey] = nPort;
    }
}

void TransportParamsRTP::ActualizeIp(const std::string& sKey, const std::string& sIp)
{
    auto ip = GetString(m_json, sKey);
    if(!ip || *ip == AUTO)
    {
        if(sIp.empty() == false)
        {
            m_json[sKey] = sIp;
        }
        else
        {
            m_json[sKey] = Json::Value::null;
        }
    }
}

void TransportParamsRTP::Actualize()
{
    if(m_json[DESTINATION_PORT] == AUTO)
    {
        m_json[DESTINATION_PORT] = 5004;
    }

    if(m_json.isMember(FEC_ENABLED) && m_json[FEC_ENABLED].asBool())
    {
        ActualizePort(FEC_1DDESTINATION_PORT, m_json[DESTINATION_PORT].asUInt()+2);
        ActualizePort(FEC_2DDESTINATION_PORT, m_json[DESTINATION_PORT].asUInt()+4);
    }

    if(m_json.isMember(RTCP_ENABLED) && m_json[RTCP_ENABLED].asBool())
    {
        ActualizePort(RTCP_DESTINATION_PORT, m_json[DESTINATION_PORT].asUInt()+1);
    }
}


void TransportParamsRTP::EnableRtp(bool bEnable)
{
    m_json[RTP_ENABLED] = bEnable;
}




/** SENDER
**/






TransportParamsRTPSender::TransportParamsRTPSender(flagsTP allowed) : TransportParamsRTP()
{
    m_json[SOURCE_IP] = AUTO;
    m_json[DESTINATION_IP] = AUTO;
    m_json[SOURCE_PORT] = AUTO;

    if(allowed & flagsTP::FEC)
    {
        FecAllowed();
    }
    if(allowed & flagsTP::RTCP)
    {
        RtcpAllowed();
    }

}

TransportParamsRTPSender::TransportParamsRTPSender(const TransportParamsRTPSender& tp) : TransportParamsRTP(tp)
{

}


TransportParamsRTPSender& TransportParamsRTPSender::operator=(const TransportParamsRTPSender& other)
{
    if(&other != this)
    {
        m_json = other.GetJson();
    }
    return *this;
}


void TransportParamsRTPSender::FecAllowed()
{
    TransportParamsRTP::FecAllowed();
    m_json[FEC_TYPE] = STR_FEC_TYPE_REED;
    m_json[FEC_BLOCK_WIDTH] = 4;
    m_json[FEC_BLOCK_HEIGHT] = 4;
    m_json[FEC_1DSOURCE_PORT] = AUTO;
    m_json[FEC_2DSOURCE_PORT] = AUTO;

}

bool TransportParamsRTPSender::CheckJson(const Json::Value&  jsPatch)
{
    return CheckJsonAllowed(jsPatch, {{SOURCE_IP, {jsondatatype::_STRING}},
                                      {DESTINATION_IP, {jsondatatype::_STRING}},
                                      {SOURCE_PORT, {jsondatatype::_STRING, jsondatatype::_INTEGER}},
                                      {DESTINATION_PORT, {jsondatatype::_STRING, jsondatatype::_INTEGER}},
                                      {FEC_ENABLED, {jsondatatype::_BOOLEAN}},
                                      {FEC_DESTINATION_IP, {jsondatatype::_STRING}},
                                      {FEC_TYPE, {jsondatatype::_STRING}},
                                      {FEC_MODE, {jsondatatype::_STRING}},
                                      {FEC_BLOCK_WIDTH, {jsondatatype::_INTEGER}},
                                      {FEC_BLOCK_HEIGHT, {jsondatatype::_INTEGER}},
                                      {FEC_1DDESTINATION_PORT, {jsondatatype::_INTEGER,jsondatatype::_STRING}},
                                      {FEC_2DDESTINATION_PORT, {jsondatatype::_INTEGER,jsondatatype::_STRING}},
                                      {FEC_1DSOURCE_PORT, {jsondatatype::_INTEGER,jsondatatype::_STRING}},
                                      {FEC_2DSOURCE_PORT, {jsondatatype::_INTEGER,jsondatatype::_STRING}},
                                      {RTCP_ENABLED, {jsondatatype::_BOOLEAN}},
                                      {RTCP_DESTINATION_IP, {jsondatatype::_STRING}},
                                      {RTCP_DESTINATION_PORT, {jsondatatype::_INTEGER,jsondatatype::_STRING}},
                                      {RTCP_SOURCE_PORT, {jsondatatype::_INTEGER,jsondatatype::_STRING}},
                                      {RTP_ENABLED, {jsondatatype::_BOOLEAN}}});

}

bool TransportParamsRTPSender::Patch(const Json::Value& jsData)
{
    if(TransportParamsRTPSender::CheckJson(jsData) == false)
    {
        return false;
    }

    if(m_json.isMember(FEC_ENABLED) == false &&
       CheckJsonNotAllowed(jsData, {FEC_TYPE, FEC_MODE, FEC_DESTINATION_IP, FEC_BLOCK_WIDTH, FEC_BLOCK_HEIGHT, FEC_1DSOURCE_PORT, FEC_1DDESTINATION_PORT, FEC_2DDESTINATION_PORT, FEC_2DSOURCE_PORT}) == false)
    {
        return false;
    }

    if(m_json.isMember(RTCP_ENABLED) == false && CheckJsonNotAllowed(jsData, {RTCP_SOURCE_PORT, RTCP_DESTINATION_IP, RTCP_DESTINATION_PORT}) == false)
    {
        return false;
    }

    //check some constraints
    if(jsData.isMember(FEC_TYPE) &&  (jsData[FEC_TYPE].asString() != STR_FEC_TYPE_XOR && jsData[FEC_TYPE].asString() != STR_FEC_TYPE_REED))
    {
        return false;
    }

    if(jsData.isMember(FEC_BLOCK_WIDTH) && (jsData[FEC_BLOCK_WIDTH].asInt() < 4 || jsData[FEC_BLOCK_WIDTH].asInt() > 200))
    {
        return false;
    }
    if(jsData.isMember(FEC_BLOCK_HEIGHT) &&  (jsData[FEC_BLOCK_HEIGHT].asInt() < 4 || jsData[FEC_BLOCK_HEIGHT].asInt() > 200))
    {
        return false;
    }
    PatchJson(m_json, jsData);
    return true;
}


void TransportParamsRTPSender::Actualize(const std::string& sSource, const std::string& sDestination)
{
    TransportParamsRTP::Actualize();


    ActualizeIp(SOURCE_IP, sDestination);
    ActualizeIp(DESTINATION_IP, sDestination);
    ActualizePort(SOURCE_PORT, 5004);


    if(m_json.isMember(FEC_ENABLED) && m_json[FEC_ENABLED].asBool())
    {
        ActualizeIp(FEC_DESTINATION_IP, m_json[DESTINATION_IP].asString());
        ActualizePort(FEC_1DSOURCE_PORT, m_json[SOURCE_PORT].asUInt()+2);
        ActualizePort(FEC_2DSOURCE_PORT, m_json[SOURCE_PORT].asUInt()+4);
    }
    if(m_json.isMember(RTCP_ENABLED) && m_json[RTCP_ENABLED].asBool())
    {
        ActualizePort(RTCP_SOURCE_PORT, m_json[SOURCE_PORT].asUInt()+1);
        ActualizeIp(RTCP_DESTINATION_IP, m_json[DESTINATION_IP].asString());
    }

}

void TransportParamsRTPSender::SetDestinationIp(const std::string& sAddress)
{
    m_json[DESTINATION_IP] = sAddress;
}

std::string TransportParamsRTPSender::GetDestinationIp() const
{
    return m_json[DESTINATION_IP].asString();
}

unsigned short TransportParamsRTPSender::GetSourcePort() const
{
    return m_json[SOURCE_PORT].asUInt();
}

std::experimental::optional<std::string> TransportParamsRTPSender::GetFecType() const
{
    return GetString(m_json, FEC_TYPE);
}

std::experimental::optional<uint32_t> TransportParamsRTPSender::GetFecBlockWidth() const
{
    return GetUInt(m_json, FEC_BLOCK_WIDTH);
}

std::experimental::optional<uint32_t> TransportParamsRTPSender::GetFecBlockHeight() const
{
    return GetUInt(m_json, FEC_BLOCK_HEIGHT);
}

std::experimental::optional<uint32_t> TransportParamsRTPSender::GetFec1DSourcePort() const
{
    return GetUInt(m_json, FEC_1DSOURCE_PORT);
}

std::experimental::optional<uint32_t> TransportParamsRTPSender::GetFec2DSourcePort() const
{
    return GetUInt(m_json, FEC_2DSOURCE_PORT);
}

std::experimental::optional<uint32_t> TransportParamsRTPSender::GetRtcpSourcePort() const
{
    return GetUInt(m_json, RTCP_SOURCE_PORT);
}






TransportParamsRTPReceiver::TransportParamsRTPReceiver(flagsTP allowed) : TransportParamsRTP()
{
    m_json[SOURCE_IP] = Json::Value::null;
    m_json[INTERFACE_IP] = AUTO;

    if(allowed & flagsTP::FEC)
    {
        FecAllowed();
    }
    if(allowed & flagsTP::RTCP)
    {
        RtcpAllowed();
    }
    if(allowed & flagsTP::MULTICAST)
    {
        m_json[MULTICAST_IP] = Json::Value::null;
    }
}

TransportParamsRTPReceiver::TransportParamsRTPReceiver(const TransportParamsRTPReceiver& tp) : TransportParamsRTP(tp)
{

}


bool TransportParamsRTPReceiver::Patch(const Json::Value& jsData)
{
    if(TransportParamsRTPReceiver::CheckJson(jsData) == false)
    {
        return false;
    }
    if(m_json.isMember(FEC_ENABLED) == false &&
       CheckJsonNotAllowed(jsData, {FEC_TYPE, FEC_MODE, FEC_DESTINATION_IP, FEC_BLOCK_WIDTH, FEC_BLOCK_HEIGHT, FEC_1DSOURCE_PORT, FEC_1DDESTINATION_PORT, FEC_2DDESTINATION_PORT, FEC_2DSOURCE_PORT}) == false)
    {
        return false;
    }

    if(m_json.isMember(RTCP_ENABLED) == false && CheckJsonNotAllowed(jsData, {RTCP_SOURCE_PORT, RTCP_DESTINATION_IP, RTCP_DESTINATION_PORT}) == false)
    {
        return false;
    }



    PatchJson(m_json, jsData);
    return true;
}

bool TransportParamsRTPReceiver::CheckJson(const Json::Value&  jsPatch)
{
    return CheckJsonAllowed(jsPatch, {{SOURCE_IP, {jsondatatype::_STRING, jsondatatype::_NULL}},
                                      {MULTICAST_IP, {jsondatatype::_STRING, jsondatatype::_NULL}},
                                      {INTERFACE_IP, {jsondatatype::_STRING}},
                                      {DESTINATION_PORT, {jsondatatype::_STRING, jsondatatype::_INTEGER}},
                                      {FEC_ENABLED, {jsondatatype::_BOOLEAN}},
                                      {FEC_DESTINATION_IP, {jsondatatype::_STRING}},
                                      {FEC_MODE, {jsondatatype::_STRING}},
                                      {FEC_1DDESTINATION_PORT, {jsondatatype::_INTEGER,jsondatatype::_STRING}},
                                      {FEC_2DDESTINATION_PORT, {jsondatatype::_INTEGER,jsondatatype::_STRING}},
                                      {RTCP_ENABLED, {jsondatatype::_BOOLEAN}},
                                      {RTCP_DESTINATION_IP, {jsondatatype::_STRING}},
                                      {RTCP_DESTINATION_PORT, {jsondatatype::_INTEGER,jsondatatype::_STRING}},
                                      {RTP_ENABLED, {jsondatatype::_BOOLEAN}}});

}



void TransportParamsRTPReceiver::Actualize(const std::string& sInterface)
{
    TransportParamsRTP::Actualize();
    m_json[INTERFACE_IP] = sInterface;

    ActualizeIp(SOURCE_IP, "");

    if(m_json.isMember(FEC_ENABLED))
    {
        if(m_json.isMember(MULTICAST_IP) == false || m_json[MULTICAST_IP].isNull())
        {
            ActualizeIp(FEC_DESTINATION_IP, m_json[INTERFACE_IP].asString());
        }
        else
        {
            ActualizeIp(FEC_DESTINATION_IP, m_json[MULTICAST_IP].asString());
        }
    }

    if(m_json.isMember(RTCP_ENABLED))
    {
        if(m_json.isMember(MULTICAST_IP) == false || m_json[MULTICAST_IP].isNull())
        {
            ActualizeIp(RTCP_DESTINATION_IP, m_json[INTERFACE_IP].asString());
        }
        else
        {
            ActualizeIp(RTCP_DESTINATION_IP, m_json[MULTICAST_IP].asString());
        }
    }
}


TransportParamsRTPReceiver& TransportParamsRTPReceiver::operator=(const TransportParamsRTPReceiver& other)
{
    if(&other != this)
    {
        m_json = other.GetJson();
    }
    return *this;
}



std::experimental::optional<std::string> TransportParamsRTPReceiver::GetMulticastIp() const
{
    return GetString(m_json, MULTICAST_IP);
}

std::string TransportParamsRTPReceiver::GetInterfaceIp() const
{
    return m_json[INTERFACE_IP].asString();
}

void TransportParamsRTPReceiver::SetMulticastIp(const std::string& sAddress)
{
    if(sAddress.empty() == false)
    {
        m_json[MULTICAST_IP] = sAddress;
    }
    else
    {
        m_json[MULTICAST_IP] = Json::Value::null;
    }
}

