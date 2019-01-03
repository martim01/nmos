#include "transportparams.h"

const std::string TransportParamsRTP::STR_FEC_MODE[2] = {"1D", "2D"};
const std::string TransportParamsRTPSender::STR_FEC_TYPE[2] = {"XOR", "Reed-Solomon"};

TransportParamsRTP::TransportParamsRTP() :
    sSourceIp("auto"),
    nDestinationPort(0),
    bFecEnabled(false),
    sFecDestinationIp("auto"),
    eFecMode(TransportParamsRTP::ONED),
    nFec1DDestinationPort(0),
    nFec2DDestinationPort(0),
    bRtcpEnabled(false),
    sRtcpDestinationIp("auto"),
    nRtcpDestinationPort(0),
    bRtpEnabled(false)
{

}

Json::Value TransportParamsRTP::GetJson() const
{
    Json::Value jsTp;

    jsTp["source_ip"] = sSourceIp;
    SetPort(jsTp, "destination_port",nDestinationPort);
    jsTp["fec_enabled"] = bFecEnabled;
    jsTp["fec_destination_ip"] = sFecDestinationIp;
    jsTp["fec_mode"] = STR_FEC_MODE[eFecMode];
    SetPort(jsTp, "fec1D_destination_port", nFec1DDestinationPort);
    SetPort(jsTp, "fec2D_destination_port", nFec2DDestinationPort);
    jsTp["rtcp_enabled"] = bRtcpEnabled;
    jsTp["rtcp_destination_ip"] = sRtcpDestinationIp;
    SetPort(jsTp, "rtcp_destination_port", nRtcpDestinationPort);
    jsTp["rtp_enabled"] = bRtpEnabled;
    return jsTp;

}

void TransportParamsRTP::SetPort(Json::Value& js, const std::string& sPort, unsigned short nPort) const
{
    if(nPort == 0)
    {
        js[sPort] = "auto";
    }
    else
    {
        js[sPort] = nPort;
    }
}


TransportParamsRTPSender::TransportParamsRTPSender() : TransportParamsRTP(),
    sDestinationIp("auto"),
    nSourcePort(0),
    eFecType(TransportParamsRTPSender::XOR),
    nFecBlockWidth(4),
    nFecBlockHeight(4),
    nFec1DSourcePort(0),
    nFec2DSourcePort(0),
    nRtcpSourcePort(0)
{

}

Json::Value TransportParamsRTPSender::GetJson() const
{
    Json::Value jsTp(TransportParamsRTP::GetJson());

    jsTp["destination_ip"] = sDestinationIp;
    SetPort(jsTp, "source_port", nSourcePort);
    jsTp["fec_type"] = STR_FEC_TYPE[eFecType];
    jsTp["fec_block_width"] = nFecBlockWidth;
    jsTp["fec_block_height"] = nFecBlockHeight;

    SetPort(jsTp, "fec1D_source_port", nFec1DSourcePort);
    SetPort(jsTp, "fec2D_source_port", nFec2DSourcePort);

    SetPort(jsTp, "rtcp_source_port", nRtcpSourcePort);
    return jsTp;
}


TransportParamsRTPReceiver::TransportParamsRTPReceiver() : TransportParamsRTP(),
    sMulticastIp("auto"),
    sInterfaceIp("auto")
{

}

Json::Value TransportParamsRTPReceiver::GetJson() const
{
    Json::Value jsTp(TransportParamsRTP::GetJson());
    jsTp["multicast_ip"] = sMulticastIp;
    jsTp["interace_ip"] = sInterfaceIp;
    return jsTp;
}
