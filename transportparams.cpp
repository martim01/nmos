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

bool TransportParamsRTP::Patch(const Json::Value& jsData)
{
    bool bIsOk(true);
    if(jsData.isObject())
    {
        if(jsData["source_ip"].isString())
        {
            sSourceIp = jsData["source_ip"].asString();
        }
        else if(jsData["source_ip"].isNull())
        {
            sSourceIp.clear();
        }
        else
        {
            bIsOk = jsData["source_ip"].empty();
        }

        bIsOk &= DecodePort(jsData, "destination_port", nDestinationPort);

        if(jsData["fec_enabled"].isBool())
        {
            bFecEnabled = jsData["fec_enabled"].asBool();
        }
        else
        {   //if not bool must not exist
            bIsOk &= jsData["fec_enabled"].empty();
        }
        if(jsData["fec_destination_ip"].isString())
        {
            sFecDestinationIp = jsData["fec_destination_ip"].asString();
        }
        else
        {   //if not string must not exist
            bIsOk &= jsData["fec_destination_ip"].empty();
        }

        if(jsData["fec_mode"].isString())
        {
            if(jsData["fec_mode"] == STR_FEC_MODE[ONED])
            {
                eFecMode = ONED;
            }
            else if(jsData["fec_mode"] == STR_FEC_MODE[TWOD])
            {
                eFecMode = TWOD;
            }
            else
            {
                bIsOk = false;
            }
        }
        else
        {   //if not string must not exist
            bIsOk &= jsData["fec_mode"].empty();
        }

        bIsOk &= DecodePort(jsData, "fec1D_destination_port", nFec1DDestinationPort);
        bIsOk &= DecodePort(jsData, "fec2D_destination_port", nFec2DDestinationPort);

        if(jsData["rtcp_enabled"].isBool())
        {
            bRtcpEnabled = jsData["rtcp_enabled"].asBool();
        }
        else
        {   //if not string must not exist
            bIsOk &= jsData["rtcp_enabled"].empty();
        }

        if(jsData["rtcp_destination_ip"].isString())
        {
            sRtcpDestinationIp = jsData["rtcp_destination_ip"].asString();
        }
        else
        {   //if not string must not exist
            bIsOk &= jsData["rtcp_destination_ip"].empty();
        }

        bIsOk &= DecodePort(jsData, "rtcp_destination_port", nRtcpDestinationPort);
        if(jsData["rtp_enabled"].isBool())
        {
            bRtpEnabled = jsData["rtp_enabled"].asBool();
        }
        else
        {
            bIsOk &= jsData["rtp_enabled"].empty();
        }
    }
    else
    {
        bIsOk = false;
    }
    return bIsOk;
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

bool TransportParamsRTP::DecodePort(const Json::Value& jsData, const std::string& sPort, unsigned short& nPort)
{
    bool bOk(false);
    if(jsData[sPort].isString())
    {
        if(jsData[sPort].asString() == "auto")
        {
            nPort = 0;
            bOk = true;
        }
    }
    else if(jsData[sPort].isInt())
    {
        nPort = jsData[sPort].asInt();
        bOk = true;
    }
    else if(jsData[sPort].empty())
    {
        bOk = true;
        nPort = 0;
    }
    return bOk;
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

bool TransportParamsRTPSender::Patch(const Json::Value& jsData)
{
    bool bIsOk = TransportParamsRTP::Patch(jsData);
    if(bIsOk)
    {
        if(jsData["destination_ip"].isString())
        {
            sDestinationIp = jsData["destination_ip"].asString();
        }
        else
        {
            bIsOk &= (jsData["destination_ip"].empty());
        }

        bIsOk &= DecodePort(jsData, "source_port", nSourcePort);

        if(jsData["fec_type"].isString())
        {
            if(jsData["fec_type"].asString() == STR_FEC_TYPE[XOR])
            {
                eFecType = XOR;
            }
            else if(jsData["fec_type"].asString() == STR_FEC_TYPE[REED])
            {
                eFecType = REED;
            }
            else
            {
                bIsOk = false;
            }
        }
        else
        {   //if not string must not exist
            bIsOk &= jsData["fec_type"].empty();
        }

        if(jsData["fec_block_width"].isInt())
        {
            nFecBlockWidth = jsData["fec_block_width"].asInt();
            bIsOk &= (nFecBlockWidth >= 4 && nFecBlockWidth <= 200);
        }
        else
        {   //if not int must not exist
            bIsOk &= jsData["fec_block_width"].empty();
        }

        if(jsData["fec_block_height"].isInt())
        {
            nFecBlockHeight = jsData["fec_block_height"].asInt();
            bIsOk &= (nFecBlockHeight >= 4 && nFecBlockHeight <= 200);
        }
        else
        {   //if not int must not exist
            bIsOk &= jsData["fec_block_height"].empty();
        }

        bIsOk &= DecodePort(jsData, "fec1D_source_port", nFec1DSourcePort);
        bIsOk &= DecodePort(jsData, "fec2D_source_port", nFec2DSourcePort);

        bIsOk &= DecodePort(jsData, "rtcp_source_port", nRtcpSourcePort);
    }
    return bIsOk;
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


bool TransportParamsRTPReceiver::Patch(const Json::Value& jsData)
{
    bool bIsOk = TransportParamsRTP::Patch(jsData);
    if(bIsOk)
    {
        if(jsData["multicast_ip"].isString())
        {
            sMulticastIp = jsData["multicast_ip"].asString();
        }
        else if(jsData["multicast_ip"].isNull())
        {
            sMulticastIp.clear();
        }
        else
        {
            bIsOk &= (jsData["multicast_ip"].empty());
        }

        if(jsData["interface_ip"].isString())
        {
            sInterfaceIp = jsData["interface_ip"].asString();
        }
        else
        {
            bIsOk &= (jsData["multicast_ip"].empty());
        }
    }
    return bIsOk;
}

Json::Value TransportParamsRTPReceiver::GetJson() const
{
    Json::Value jsTp(TransportParamsRTP::GetJson());
    jsTp["multicast_ip"] = sMulticastIp;
    jsTp["interface_ip"] = sInterfaceIp;
    return jsTp;
}
