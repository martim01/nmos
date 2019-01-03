#include "transportparams.h"
#include "log.h"

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
        else if(jsData["source_ip"].empty()== false)
        {
            bIsOk = false;
            Log::Get(Log::DEBUG) << "Patch: source_ip incorrect type" << std::endl;
        }

        bIsOk &= DecodePort(jsData, "destination_port", nDestinationPort);

        if(jsData["fec_enabled"].isBool())
        {
            bFecEnabled = jsData["fec_enabled"].asBool();
        }
        else if(jsData["fec_enabled"].empty() == false)
        {
            bIsOk = false;
            Log::Get(Log::DEBUG) << "Patch: fec_enabled incorrect type" << std::endl;
        }

        if(jsData["fec_destination_ip"].isString())
        {
            sFecDestinationIp = jsData["fec_destination_ip"].asString();
        }
        else if(jsData["fec_destination_ip"].empty() == false)
        {
            bIsOk = false;
            Log::Get(Log::DEBUG) << "Patch: fec_destination_ip incorrect type" << std::endl;
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
                Log::Get(Log::DEBUG) << "Patch: fec_mode not found" << std::endl;
                bIsOk = false;
            }
        }
        else if(jsData["fec_mode"].empty() == false)
        {
            bIsOk = false;
            Log::Get(Log::DEBUG) << "Patch: fec_mode incorrect type" << std::endl;
        }

        bIsOk &= DecodePort(jsData, "fec1D_destination_port", nFec1DDestinationPort);
        bIsOk &= DecodePort(jsData, "fec2D_destination_port", nFec2DDestinationPort);

        if(jsData["rtcp_enabled"].isBool())
        {
            bRtcpEnabled = jsData["rtcp_enabled"].asBool();
        }
        else if(jsData["rtcp_enabled"].empty() == false)
        {
            bIsOk = false;
            Log::Get(Log::DEBUG) << "Patch: rtcp_enabled incorrect type" << std::endl;
        }

        if(jsData["rtcp_destination_ip"].isString())
        {
            sRtcpDestinationIp = jsData["rtcp_destination_ip"].asString();
        }
        else if(jsData["rtcp_destination_ip"].empty() == false)
        {
            bIsOk = false;
            Log::Get(Log::DEBUG) << "Patch: rtcp_destination_ip incorrect type" << std::endl;
        }

        bIsOk &= DecodePort(jsData, "rtcp_destination_port", nRtcpDestinationPort);

        if(jsData["rtp_enabled"].isBool())
        {
            bRtpEnabled = jsData["rtp_enabled"].asBool();
        }
        else if(jsData["rtp_enabled"].empty() == false)
        {
            bIsOk = false;
            Log::Get(Log::DEBUG) << "Patch: rtp_enabled incorrect type" << std::endl;
        }
    }
    Log::Get(Log::DEBUG) << "Patch: TransportParamsRTP: " << bIsOk << std::endl;
    //else
    //{
    //    Log::Get(Log::DEBUG) << "Patch: transport_params is not an object" << endl;
    //    bIsOk = false;
   // }
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
    else
    {
        Log::Get(Log::DEBUG) << "Patch: " << sPort << " not valid" << std::endl;
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


void TransportParamsRTP::Actualize()
{
    if(nDestinationPort == 0)
    {
        nDestinationPort = 5004;
    }

    if(bFecEnabled)
    {
        if(nFec1DDestinationPort == 0)
        {
            nFec1DDestinationPort = nDestinationPort+2;
        }
        if(nFec2DDestinationPort == 0)
        {
            nFec2DDestinationPort = nDestinationPort+4;
        }

    }
    if(bRtcpEnabled)
    {
        if(nRtcpDestinationPort==0)
        {
            if(nDestinationPort%2 == 0)
            {
                nRtcpDestinationPort = nDestinationPort+1;
            }
            else
            {
                nRtcpDestinationPort = nDestinationPort+2;
            }
        }

    }
}

bool TransportParamsRTPSender::Patch(const Json::Value& jsData)
{
    if(jsData.isObject())
    {
        bool bIsOk = TransportParamsRTP::Patch(jsData);
        if(bIsOk)
        {
            if(jsData["destination_ip"].isString())
            {
                sDestinationIp = jsData["destination_ip"].asString();
            }
            else if(jsData["destination_ip"].empty() == false)
            {
                bIsOk = false;
                Log::Get(Log::DEBUG) << "Patch: destination_ip incorrect type" << std::endl;
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
                    Log::Get(Log::DEBUG) << "Patch: fec_type not found" << std::endl;
                }
            }
            else if(jsData["fec_type"].empty() == false)
            {
                bIsOk = false;
                Log::Get(Log::DEBUG) << "Patch: fec_type incorrect type" << std::endl;
            }

            if(jsData["fec_block_width"].isInt())
            {
                nFecBlockWidth = jsData["fec_block_width"].asInt();
                bIsOk &= (nFecBlockWidth >= 4 && nFecBlockWidth <= 200);
            }
            else if(jsData["fec_block_width"].empty() == false)
            {
                bIsOk = false;
                Log::Get(Log::DEBUG) << "Patch: fec_block_width incorrect type" << std::endl;
            }

            if(jsData["fec_block_height"].isInt())
            {
                nFecBlockHeight = jsData["fec_block_height"].asInt();
                bIsOk &= (nFecBlockHeight >= 4 && nFecBlockHeight <= 200);
            }
            else if(jsData["fec_block_height"].empty() == false)
            {
                bIsOk = false;
                Log::Get(Log::DEBUG) << "Patch: fec_block_height incorrect type" << std::endl;
            }

            bIsOk &= DecodePort(jsData, "fec1D_source_port", nFec1DSourcePort);
            bIsOk &= DecodePort(jsData, "fec2D_source_port", nFec2DSourcePort);

            bIsOk &= DecodePort(jsData, "rtcp_source_port", nRtcpSourcePort);
        }

        Log::Get(Log::DEBUG) << "Patch: TransportParamsRTPSender: " << bIsOk << std::endl;
        return bIsOk;
    }
    return true;
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


void TransportParamsRTPSender::Actualize(const std::string& sSource, const std::string& sDestination)
{
    TransportParamsRTP::Actualize();

    sSourceIp =sSource;
    sDestinationIp = sDestination;

    if(nSourcePort == 0)
    {
        nSourcePort = 5004;
    }

    if(bFecEnabled)
    {
        if(sFecDestinationIp == "auto")
        {
            sFecDestinationIp = sDestinationIp;
        }
        if(nFec1DSourcePort == 0)
        {
            nFec1DSourcePort = nSourcePort+2;
        }
        if(nFec2DSourcePort == 0)
        {
            nFec2DSourcePort = nSourcePort+4;
        }
    }

    if(bRtcpEnabled)
    {
        if(nRtcpSourcePort == 0)
        {
            if(nSourcePort%2 == 0)
            {
                nRtcpSourcePort = nSourcePort+1;
            }
            else
            {
                nRtcpSourcePort = nSourcePort+2;
            }
        }
        if(sRtcpDestinationIp == "auto")
        {
            sRtcpDestinationIp = sDestinationIp;
        }
    }

}


TransportParamsRTPReceiver::TransportParamsRTPReceiver() : TransportParamsRTP(),
    sMulticastIp("auto"),
    sInterfaceIp("auto")
{

}


bool TransportParamsRTPReceiver::Patch(const Json::Value& jsData)
{
    if(jsData.isObject())
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
            else if(jsData["multicast_ip"].empty() == false)
            {
                bIsOk = false;
                Log::Get(Log::DEBUG) << "Patch: multicast_ip incorrect type" << std::endl;
            }

            if(jsData["interface_ip"].isString())
            {
                sInterfaceIp = jsData["interface_ip"].asString();
            }
            else if(jsData["interface_ip"].empty() == false)
            {
                bIsOk = false;
                Log::Get(Log::DEBUG) << "Patch: interface_ip incorrect type" << std::endl;
            }
        }
        return bIsOk;
    }
    return true;
}

Json::Value TransportParamsRTPReceiver::GetJson() const
{
    Json::Value jsTp(TransportParamsRTP::GetJson());
    jsTp["multicast_ip"] = sMulticastIp;
    jsTp["interface_ip"] = sInterfaceIp;
    return jsTp;
}


void TransportParamsRTPReceiver::Actualize(const std::string& sInterface)
{
    TransportParamsRTP::Actualize();
    sInterfaceIp = sInterface;

    if(bFecEnabled)
    {
        if(sFecDestinationIp == "auto")
        {
            if(sMulticastIp.empty())
            {
                sFecDestinationIp = sInterfaceIp;
            }
            else
            {
                sFecDestinationIp = sMulticastIp;
            }
        }
    }

    if(bRtcpEnabled)
    {
        if(sRtcpDestinationIp == "auto")
        {
            if(sMulticastIp.empty())
            {
                sRtcpDestinationIp = sInterfaceIp;
            }
            else
            {
                sRtcpDestinationIp = sMulticastIp;
            }
        }
    }
}
