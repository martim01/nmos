#include "transportparams.h"
#include "log.h"
#include "utils.h"

using namespace pml::nmos;

const std::string TransportParamsRTP::STR_FEC_MODE[2] = {"1D", "2D"};
const std::string TransportParamsRTPSender::STR_FEC_TYPE[2] = {"XOR", "Reed-Solomon"};

TransportParamsRTP::TransportParamsRTP() :
    sSourceIp(""),
    nDestinationPort(0),
    bRtpEnabled(false)
{

}


TransportParamsRTP::TransportParamsRTP(const TransportParamsRTP& tp) :
    sSourceIp(tp.sSourceIp),
    nDestinationPort(tp.nDestinationPort),
    bFecEnabled(tp.bFecEnabled),
    sFecDestinationIp(tp.sFecDestinationIp),
    eFecMode(tp.eFecMode),
    nFec1DDestinationPort(tp.nFec1DDestinationPort),
    nFec2DDestinationPort(tp.nFec2DDestinationPort),
    bRtcpEnabled(tp.bRtcpEnabled),
    sRtcpDestinationIp(tp.sRtcpDestinationIp),
    nRtcpDestinationPort(tp.nRtcpDestinationPort),
    bRtpEnabled(tp.bRtpEnabled)
{
}


TransportParamsRTP& TransportParamsRTP::operator=(const TransportParamsRTP& other)
{
    sSourceIp = other.sSourceIp;
    nDestinationPort = other.nDestinationPort;
    bFecEnabled = other.bFecEnabled;
    sFecDestinationIp  = other.sFecDestinationIp;
    eFecMode = other.eFecMode;
    nFec1DDestinationPort = other.nFec1DDestinationPort;
    nFec2DDestinationPort = other.nFec2DDestinationPort;
    bRtcpEnabled = other.bRtcpEnabled;
    sRtcpDestinationIp = other.sRtcpDestinationIp;
    nRtcpDestinationPort = other.nRtcpDestinationPort;
    bRtpEnabled = other.bRtpEnabled;

    return *this;
}

void TransportParamsRTP::FecAllowed()
{
    bFecEnabled = false;
    sFecDestinationIp = "";
    eFecMode = (char)ONED;
    nFec1DDestinationPort = 0;
    nFec2DDestinationPort = 0;
}

void TransportParamsRTP::RtcpAllowed()
{
    bRtcpEnabled = false;
    sRtcpDestinationIp = "";
    nRtcpDestinationPort = 0;
}

bool TransportParamsRTP::Patch(const Json::Value& jsData)
{
    bool bIsOk(true);
    if(jsData.isObject())
    {

        if(jsData["source_ip"].isString())
        {
            if(jsData["source_ip"].asString() != "auto")
            {
                sSourceIp = jsData["source_ip"].asString();
            }
        }
        else if(JsonMemberExistsAndIsNull(jsData, "source_ip"))
        {
            sSourceIp.clear();
        }
        else if(jsData["source_ip"].empty()== false)
        {
            bIsOk = false;
            pmlLog(pml::LOG_DEBUG) << "NMOS: " << "Patch: source_ip incorrect type" ;
        }

        bIsOk &= DecodePort(jsData, "destination_port", nDestinationPort);

        if(jsData["fec_enabled"].isBool())
        {
            bFecEnabled = jsData["fec_enabled"].asBool();
        }
        else if(jsData["fec_enabled"].empty() == false)
        {
            bIsOk = false;
            pmlLog(pml::LOG_DEBUG) << "NMOS: " << "Patch: fec_enabled incorrect type" ;
        }

        if(jsData["fec_destination_ip"].isString())
        {
            sFecDestinationIp = jsData["fec_destination_ip"].asString();
        }
        else if(jsData["fec_destination_ip"].empty() == false)
        {
            bIsOk = false;
            pmlLog(pml::LOG_DEBUG) << "NMOS: " << "Patch: fec_destination_ip incorrect type" ;
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
                pmlLog(pml::LOG_DEBUG) << "NMOS: " << "Patch: fec_mode not found" ;
                bIsOk = false;
            }
        }
        else if(jsData["fec_mode"].empty() == false)
        {
            bIsOk = false;
            pmlLog(pml::LOG_DEBUG) << "NMOS: " << "Patch: fec_mode incorrect type" ;
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
            pmlLog(pml::LOG_DEBUG) << "NMOS: " << "Patch: rtcp_enabled incorrect type" ;
        }

        if(jsData["rtcp_destination_ip"].isString())
        {
            sRtcpDestinationIp = jsData["rtcp_destination_ip"].asString();
        }
        else if(jsData["rtcp_destination_ip"].empty() == false)
        {
            bIsOk = false;
            pmlLog(pml::LOG_DEBUG) << "NMOS: " << "Patch: rtcp_destination_ip incorrect type" ;
        }

        bIsOk &= DecodePort(jsData, "rtcp_destination_port", nRtcpDestinationPort);

        if(jsData["rtp_enabled"].isBool())
        {

            bRtpEnabled = jsData["rtp_enabled"].asBool();

            pmlLog(pml::LOG_DEBUG) << "NMOS: " << "Patch: transport_params rtp_enabled " << bRtpEnabled ;
        }
        else if(jsData["rtp_enabled"].empty() == false)
        {
            bIsOk = false;
            pmlLog(pml::LOG_DEBUG) << "NMOS: " << "Patch: rtp_enabled incorrect type" ;
        }
    }
    else if(jsData.empty() == false)
    {
        pmlLog(pml::LOG_DEBUG) << "NMOS: " << "Patch: transport_params is not an object" ;
        bIsOk = false;
    }
    pmlLog(pml::LOG_DEBUG) << "NMOS: " << "Patch: TransportParamsRTP: " << bIsOk ;
    //else
    //{
    //    pmlLog(pml::LOG_DEBUG) << "Patch: transport_params is not an object" ;
    //    bIsOk = false;
   // }
    return bIsOk;
}

Json::Value TransportParamsRTP::GetJson(const ApiVersion& version) const
{
    Json::Value jsTp;

    if(sSourceIp.empty() == false)
    {
        jsTp["source_ip"] = sSourceIp;
    }
    else
    {
        jsTp["source_ip"] = Json::nullValue;
    }

    SetPort(jsTp, "destination_port",nDestinationPort);
    if(bFecEnabled)
    {
        jsTp["fec_enabled"] = (*bFecEnabled);
        jsTp["fec_destination_ip"] = (*sFecDestinationIp);
        jsTp["fec_mode"] = STR_FEC_MODE[*eFecMode];
        SetPort(jsTp, "fec1D_destination_port", *nFec1DDestinationPort);
        SetPort(jsTp, "fec2D_destination_port", *nFec2DDestinationPort);
    }
    if(bRtcpEnabled)
    {
        jsTp["rtcp_enabled"] = (*bRtcpEnabled);
        jsTp["rtcp_destination_ip"] = *sRtcpDestinationIp;
        SetPort(jsTp, "rtcp_destination_port", *nRtcpDestinationPort);
    }
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
        pmlLog(pml::LOG_DEBUG) << "NMOS: " << "Patch: " << sPort << " not valid" ;
    }
    return bOk;
}

TransportParamsRTPSender::TransportParamsRTPSender() : TransportParamsRTP(),
    sDestinationIp("auto"),
    nSourcePort(0)
{
    sSourceIp = "auto"; //sender sourceIp defaults to auto. receiver to null
}

TransportParamsRTPSender::TransportParamsRTPSender(const TransportParamsRTPSender& tp) : TransportParamsRTP(tp),
    sDestinationIp(tp.sDestinationIp),
    nSourcePort(tp.nSourcePort),
    eFecType(tp.eFecType),
    nFecBlockWidth(tp.nFecBlockWidth),
    nFecBlockHeight(tp.nFecBlockHeight),
    nFec1DSourcePort(tp.nFec1DSourcePort),
    nFec2DSourcePort(tp.nFec2DSourcePort),
    nRtcpSourcePort(tp.nRtcpSourcePort)
{

}


TransportParamsRTPSender& TransportParamsRTPSender::operator=(const TransportParamsRTPSender& other)
{
    TransportParamsRTP::operator=(other);
    sDestinationIp = other.sDestinationIp;
    nSourcePort = other.nSourcePort;
    eFecType = other.eFecType;
    nFecBlockWidth = other.nFecBlockWidth;
    nFecBlockHeight = other.nFecBlockHeight;
    nFec1DSourcePort = other.nFec1DSourcePort;
    nFec2DSourcePort = other.nFec2DSourcePort;
    nRtcpSourcePort = other.nRtcpSourcePort;
    return *this;
}

void TransportParamsRTP::Actualize()
{
    if(0 == nDestinationPort)
    {
        nDestinationPort = 5004;
    }

    pmlLog(pml::LOG_DEBUG) << "NMOS: " << "Actualize Destination Port = " << nDestinationPort ;

    if(bFecEnabled && *bFecEnabled)
    {
        if(!nFec1DDestinationPort || *nFec1DDestinationPort == 0)
        {
            nFec1DDestinationPort = nDestinationPort+2;
        }
        if(!nFec2DDestinationPort  || *nFec2DDestinationPort == 0)
        {
            nFec2DDestinationPort = nDestinationPort+4;
        }

    }
    if(bRtcpEnabled && *bRtcpEnabled)
    {
        if(!nRtcpDestinationPort || *nRtcpDestinationPort==0)
        {
            if(0 == nDestinationPort%2)
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
    bool bIsOk = TransportParamsRTP::Patch(jsData);
    if(bIsOk && jsData.isObject())
    {
        if(jsData["destination_ip"].isString())
        {
            sDestinationIp = jsData["destination_ip"].asString();
        }
        else if(jsData["destination_ip"].empty() == false)
        {
            bIsOk = false;
            pmlLog(pml::LOG_DEBUG) << "NMOS: " << "Patch: destination_ip incorrect type" ;
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
                pmlLog(pml::LOG_DEBUG) << "NMOS: " << "Patch: fec_type not found" ;
            }
        }
        else if(jsData["fec_type"].empty() == false)
        {
            bIsOk = false;
            pmlLog(pml::LOG_DEBUG) << "NMOS: " << "Patch: fec_type incorrect type" ;
        }

        if(jsData["fec_block_width"].isInt())
        {
            nFecBlockWidth = jsData["fec_block_width"].asInt();
            bIsOk &= (nFecBlockWidth >= 4 && nFecBlockWidth <= 200);
        }
        else if(jsData["fec_block_width"].empty() == false)
        {
            bIsOk = false;
            pmlLog(pml::LOG_DEBUG) << "NMOS: " << "Patch: fec_block_width incorrect type" ;
        }

        if(jsData["fec_block_height"].isInt())
        {
            nFecBlockHeight = jsData["fec_block_height"].asInt();
            bIsOk &= (nFecBlockHeight >= 4 && nFecBlockHeight <= 200);
        }
        else if(jsData["fec_block_height"].empty() == false)
        {
            bIsOk = false;
            pmlLog(pml::LOG_DEBUG) << "NMOS: " << "Patch: fec_block_height incorrect type" ;
        }

        if(!DecodePort(jsData, "fec1D_source_port", nFec1DSourcePort) || !DecodePort(jsData, "fec2D_source_port", nFec2DSourcePort))
        {
            bIsOk = false;
        }

        if(!DecodePort(jsData, "rtcp_source_port", nRtcpSourcePort))
        {
            bIsOk = false;
        }
    }

    pmlLog(pml::LOG_DEBUG) << "NMOS: " << "Patch: TransportParamsRTPSender: " << bIsOk ;
    return bIsOk;
}

Json::Value TransportParamsRTPSender::GetJson(const ApiVersion& version) const
{
    Json::Value jsTp(TransportParamsRTP::GetJson(version));

    if(sDestinationIp.empty() == false)
    {
        jsTp["destination_ip"] = sDestinationIp;
    }
    else
    {
        jsTp["destination_ip"] = Json::nullValue;
    }
    SetPort(jsTp, "source_port", nSourcePort);

    if(eFecType)
    {
        jsTp["fec_type"] = STR_FEC_TYPE[*eFecType];
        jsTp["fec_block_width"] = *nFecBlockWidth;
        jsTp["fec_block_height"] = *nFecBlockHeight;

        SetPort(jsTp, "fec1D_source_port", *nFec1DSourcePort);
        SetPort(jsTp, "fec2D_source_port", *nFec2DSourcePort);
    }
    if(nRtcpSourcePort)
    {
        SetPort(jsTp, "rtcp_source_port", *nRtcpSourcePort);
    }
    return jsTp;
}


void TransportParamsRTPSender::Actualize(const std::string& sSource, const std::string& sDestination)
{
    TransportParamsRTP::Actualize();

    if(sSourceIp == "auto" || sSourceIp.empty())
    {
        sSourceIp =sSource;
    }
    if(sDestinationIp == "auto" || sDestinationIp.empty())
    {
        sDestinationIp = sDestination;
    }

    if(nSourcePort == 0)
    {
        nSourcePort = 5004;
    }

    if(bFecEnabled && &bFecEnabled)
    {
        if(!sFecDestinationIp || *sFecDestinationIp == "auto")
        {
            sFecDestinationIp = sDestinationIp;
        }
        if(!nFec1DSourcePort || *nFec1DSourcePort == 0)
        {
            nFec1DSourcePort = nSourcePort+2;
        }
        if(!nFec2DSourcePort || *nFec2DSourcePort == 0)
        {
            nFec2DSourcePort = nSourcePort+4;
        }
    }

    if(bRtcpEnabled && *bRtcpEnabeld)
    {
        if(!nRtcpSourcePort || *nRtcpSourcePort == 0)
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
        if(!sRtcpDestinationIp || *sRtcpDestinationIp == "auto")
        {
            sRtcpDestinationIp = sDestinationIp;
        }
    }

}


TransportParamsRTPReceiver::TransportParamsRTPReceiver() : TransportParamsRTP(),
    sInterfaceIp("auto")

{
}

TransportParamsRTPReceiver::TransportParamsRTPReceiver(const TransportParamsRTPReceiver& tp) : TransportParamsRTP(tp),
    sMulticastIp(tp.sMulticastIp),
    sInterfaceIp(tp.sInterfaceIp)

{

}


bool TransportParamsRTPReceiver::Patch(const Json::Value& jsData)
{
    bool bIsOk = TransportParamsRTP::Patch(jsData);
    if(bIsOk && jsData.isObject())
    {
        if(jsData["multicast_ip"].isString())
        {
            eMulticast = TP_SUPPORTED;
            if(jsData["multicast_ip"].asString() != "auto")
            {
                pmlLog(pml::LOG_DEBUG) << "NMOS: " << "TransportParamsRTPReceiver::Patch: Multicast=" << jsData["multicast_ip"].asString() ;
                sMulticastIp = jsData["multicast_ip"].asString();
            }
        }
        else if(JsonMemberExistsAndIsNull(jsData, "multicast_ip"))
        {
            pmlLog(pml::LOG_DEBUG) << "NMOS: " << "TransportParamsRTPReceiver::Patch: Multicast=null" ;
            sMulticastIp = "";
        }
        else if(jsData["multicast_ip"].empty() == false)
        {
            bIsOk = false;
            pmlLog(pml::LOG_DEBUG) << "NMOS: " << "Patch: multicast_ip incorrect type" ;
        }
        if(jsData["interface_ip"].isString())
        {
            if(jsData["interface_ip"].asString() != "auto")
            {
                sInterfaceIp = jsData["interface_ip"].asString();
            }
        }
        else if(jsData["interface_ip"].empty() == false)
        {
            bIsOk = false;
            pmlLog(pml::LOG_DEBUG) << "NMOS: " << "Patch: interface_ip incorrect type" ;
        }
    }
    return bIsOk;
}

Json::Value TransportParamsRTPReceiver::GetJson(const ApiVersion& version) const
{
    Json::Value jsTp(TransportParamsRTP::GetJson(version));
    if(sMulticastIp)
    {
        if((*sMulticastIp).empty() == false)
        {
            jsTp["multicast_ip"] = *sMulticastIp;
        }
        else
        {
            jsTp["multicast_ip"] = Json::nullValue;
        }
    }

    jsTp["interface_ip"] = sInterfaceIp;
    return jsTp;
}


void TransportParamsRTPReceiver::Actualize(const std::string& sInterface)
{
    TransportParamsRTP::Actualize();
    sInterfaceIp = sInterface;

    if(bFecEnabled && *bFecEnabled)
    {
        if(!sFecDestinationIp || *sFecDestinationIp == "auto")
        {
            if(!sMulticastIp || (*sMulticastIp).empty())
            {
                sFecDestinationIp = sInterfaceIp;
            }
            else
            {
                sFecDestinationIp = sMulticastIp;
            }
        }
    }

    if(bRtcpEnabled && *bRtcpEnabled)
    {
        if(!sRtcpDestinationIp || *sRtcpDestinationIp == "auto")
        {
            if(!sMulticastIp || (*sMulticastIp).empty())
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


TransportParamsRTPReceiver& TransportParamsRTPReceiver::operator=(const TransportParamsRTPReceiver& other)
{
    TransportParamsRTP::operator=(other);
    sMulticastIp = other.sMulticastIp;
    sInterfaceIp = other.sInterfaceIp;
    return *this;
}

void TransportParamsRTPSender::FecAllowed()
{
    TransportParamsRTP::FecAllowed();
    eFecType = REED;
    nFecBlockWidth = 0;
    nFecBlockHeight= 0;
    nFec1DSourcePort= 0;
    nFec2DSourcePort= 0;
    nRtcpSourcePort= 0;
}

