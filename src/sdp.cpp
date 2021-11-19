#include "sdp.h"
#include "transportparams.h"
#include "utils.h"
#include "log.h"

using namespace std;
using namespace pml::nmos;

const string SdpManager::STR_FILTER = "a=source-filter:";
const string SdpManager::STR_RTCP = "a=rtcp:";

bool SdpManager::SdpToTransportParams(const std::string& sSdp, std::vector<TransportParamsRTPReceiver>& tpReceivers)
{
    istringstream f(sSdp);
    string sLine;
    int nMedia = -1;
    while (getline(f, sLine,'\n'))
    {
        if(nMedia >= (int)tpReceivers.size())
        {
            break;
        }

        switch(sLine[0])
        {
            case 'o':
                if(ParseOriginLine(sLine, tpReceivers) == false)
                {
                    pmlLog(pml::LOG_DEBUG) << "NMOS: " << "SdpManager:: ParseOriginLine failed";
                    return false;
                }
                break;
            case 'c':
                if(ParseConnectionLine(sLine, tpReceivers,nMedia) == false)
                {
                    return false;
                }
                break;
            case 'a':
                if(ParseAttributeLine(sLine, tpReceivers,nMedia) == false)
                {
                    return false;
                }
                break;
            case 'm':
                ++nMedia;
                if(ParseMediaLine(sLine, tpReceivers,nMedia) == false)
                {
                    return false;
                }

                break;
        }

    }


    return true;
}

bool SdpManager::ParseOriginLine(const std::string& sLine, std::vector<TransportParamsRTPReceiver>& tpReceivers)
{
    pmlLog(pml::LOG_DEBUG) << "NMOS: " << "SdpManager::ParseOriginLine " << sLine;
    vector<string> vSplit;
    SplitString(vSplit, sLine, ' ');
    if(vSplit.size() == 6)
    {
        if(vSplit[4] == "IP4")
        {
            return ParseOriginIp4(vSplit[5], tpReceivers);
        }
        else if(vSplit[4] == "IP6")
        {
            return ParseOriginIp6(vSplit[5], tpReceivers);
        }
    }
    return false;
}

bool SdpManager::ParseConnectionLine(const std::string& sLine, std::vector<TransportParamsRTPReceiver>& tpReceivers, size_t nMedia)
{
    pmlLog(pml::LOG_DEBUG) << "NMOS: " << "SdpManager::ParseConnectionLine: " << sLine ;
    //format is 'type addtypr address'
    vector<string> vSplit;
    SplitString(vSplit, sLine, ' ');
    if(vSplit.size() == 3 && vSplit[0] == "c=IN")
    {
        if(vSplit[1] == "IP4")
        {
            return ParseConnectionIp4(vSplit[2], tpReceivers, nMedia);
        }
        else if(vSplit[1] == "IP4")
        {
            return ParseConnectionIp6(vSplit[2], tpReceivers, nMedia);
        }
    }
    pmlLog(pml::LOG_DEBUG) << "NMOS: " << "SdpManager::ParseConnectionLine: " << vSplit.size() << " " << vSplit[0] ;
    return false;
}

bool SdpManager::ParseConnectionIp4(const std::string& sAddress, std::vector<TransportParamsRTPReceiver>& tpReceivers, size_t nMedia)
{
    size_t nTTL = sAddress.find('/');
    switch(ValidateIp4Address(sAddress.substr(0, nTTL)))
    {
        case IP4_MULTI:
            tpReceivers[nMedia].sMulticastIp = sAddress.substr(0, nTTL);
            pmlLog(pml::LOG_DEBUG) << "NMOS: " << "SdpManager::ParseConnectionIp4: " << sAddress.substr(0, nTTL) << " multicast" ; //@todo do we care about TTL and possible address grouping??
            return true;
        case IP4_UNI:
            tpReceivers[nMedia].sMulticastIp.clear();
            pmlLog(pml::LOG_DEBUG) << "NMOS: " << "SdpManager::ParseConnectionIp4: " << sAddress << " unicast" ; //@todo do we care about TTL and possible address grouping??
            return true;
        default:
            pmlLog(pml::LOG_DEBUG) << "NMOS: " << "SdpManager::ParseConnectionIp4: " << sAddress << " invalid" ;
            return false;
    }
    return false;
}

bool SdpManager::ParseConnectionIp6(const std::string& sAddress, std::vector<TransportParamsRTPReceiver>& tpReceivers, size_t nMedia)
{
    size_t nGrouping = sAddress.find('/');
    switch(ValidateIp6Address(sAddress.substr(0, nGrouping)))
    {
        case IP6_MULTI:
            tpReceivers[nMedia].sMulticastIp = sAddress.substr(0, nGrouping);
            //@todo do we care about TTL and possible address grouping??
            return true;
        case IP6_UNI:
            tpReceivers[nMedia].sMulticastIp.clear();
            //tpReceiver.sSourceIp = sAddress.substr(0, nGrouping);
            //@todo do we care about TTL and possible address grouping??
            return true;
        default:
            return false;
    }
    return false;
}


bool SdpManager::ParseOriginIp4(const std::string& sAddress, std::vector<TransportParamsRTPReceiver>& tpReceivers)
{
    switch(ValidateIp4Address(sAddress))
    {
        case IP4_MULTI:
            for(auto& tpReceiver : tpReceivers)
            {
                tpReceiver.sSourceIp.clear();
            }
            return false;
        case IP4_UNI:
            for(auto& tpReceiver : tpReceivers)
            {
                tpReceiver.sSourceIp = sAddress;
            }
            return true;
        default:
            return false;
    }
    return false;
}

bool SdpManager::ParseOriginIp6(const std::string& sAddress, std::vector<TransportParamsRTPReceiver>& tpReceivers)
{
    switch(ValidateIp6Address(sAddress))
    {
        case IP6_MULTI:
           for(auto& tpReceiver : tpReceivers)
           {
               tpReceiver.sSourceIp.clear();
           }
           return false;
        case IP6_UNI:
            for(auto& tpReceiver : tpReceivers)
            {
                tpReceiver.sSourceIp = sAddress;
            }
            return true;
        default:
            return false;

    }
    return false;
}
SdpManager::enumIPType SdpManager::ValidateIp4Address(const std::string& sAddress)
{
    vector<string> vSplit;
    SplitString(vSplit, sAddress, '.');
    if(vSplit.size() != 4)
    {
        return IP_INVALID;
    }
    vector<long> vValue;
    vValue.resize(4);
    for(size_t i = 0; i < 4; i++)
    {

        try
        {
            if(vSplit[i].empty())
            {
                return IP_INVALID;
            }
            if(vSplit[i].length() > 1 && vSplit[i][0] == '0')
            {
                if(vSplit[i][1] == 'x' || vSplit[i][1] == 'X')
                {   //hex
                    vValue[i] = stol(vSplit[i], 0, 16);
                }
                else
                {   //oct
                    vValue[i] = stol(vSplit[i], 0, 8);
                }
            }
            else
            {   //dec
                vValue[i] = stol(vSplit[i]);
            }
            if(vValue[i] < 0 || vValue[i] > 255)
            {
                return IP_INVALID;
            }
        }
        catch(invalid_argument& ia)
        {
            return IP_INVALID;
        }
    }
    //check if multicast address or unicast
    if(vValue[0] >= 224 && vValue[0] <= 239)
    {
        return IP4_MULTI;
    }

    return IP4_UNI;
}

SdpManager::enumIPType SdpManager::ValidateIp6Address(const std::string& sAddress)
{
    //@todo ValidateIp6Address and workout unicast or multicast
    return IP_INVALID;
}

bool SdpManager::ParseAttributeLine(const std::string& sLine, std::vector<TransportParamsRTPReceiver>& tpReceivers, size_t nMedia)
{
    pmlLog(pml::LOG_DEBUG) << "NMOS: " << "SdpManager::ParseAttributeLine: " << sLine ;
    //find source filter line for ssmc destination address
    size_t nStart = sLine.find(STR_FILTER);
    if(nStart != string::npos)
    {
        return ParseSourceFilter(sLine.substr(nStart+STR_FILTER.length()), tpReceivers, nMedia);
    }
    nStart = sLine.find(STR_RTCP);
    if(nStart != string::npos)
    {
        return ParseRTCP(sLine.substr(nStart+STR_RTCP.length()), tpReceivers, nMedia);
    }

    // @todo what other settings are possible here? Possibly FEC

    return true;
}

bool SdpManager::ParseSourceFilter(const std::string& sLine, std::vector<TransportParamsRTPReceiver>& tpReceivers, size_t nMedia)
{
    vector<string> vSplit;
    SplitString(vSplit, sLine, ' ');
    if(vSplit.size() > 4)
    {
        pmlLog(pml::LOG_DEBUG) << "NMOS: " << "SdpManager::ParseSourceFilter " << vSplit[0] << " " << vSplit[1] << " " << vSplit[2] ;
        if(vSplit[0] == "IN" && vSplit[1] == "incl")
        {
            if(vSplit[2] == "IP4" && ValidateIp4Address(vSplit[3]) == IP4_MULTI && ValidateIp4Address(vSplit[4]) == IP4_UNI)
            {
                tpReceivers[nMedia].sSourceIp = vSplit[4];
                return true;
            }
            else if(vSplit[2] == "IP6" && ValidateIp6Address(vSplit[3]) == IP6_MULTI && ValidateIp6Address(vSplit[4]) == IP6_UNI)
            {
                tpReceivers[nMedia].sSourceIp = vSplit[4];
                return true;
            }
        }
    }
    pmlLog(pml::LOG_DEBUG) << "NMOS: " << "SdpManager::ParseSourceFilter " << vSplit.size() ;
    return false;
}

bool SdpManager::ParseRTCP(const std::string& sLine, std::vector<TransportParamsRTPReceiver>& tpReceivers, size_t nMedia)
{
    //format is 'port [IN IP4/6 address]'
    vector<string> vSplit;
    SplitString(vSplit, sLine, ' ');
    if(vSplit.empty() == false)
    {
        try
        {
            long nPort = stol(vSplit[0]);
            if(nPort > 0 && nPort < 65536)
            {
                tpReceivers[nMedia].nRtcpDestinationPort = nPort;
            }
            else
            {
                return false;
            }
        }
        catch(invalid_argument& ia)
        {
            return false;
        }
        if(vSplit.size() == 1)
        {
            return true;
        }
        //also have destination address which should contain 3 more parts
        if(vSplit.size() >= 4 && vSplit[1] != "IN")
        {
            if(vSplit[2] == "IP4" && ValidateIp4Address(vSplit[3]) != IP_INVALID)
            {
                tpReceivers[nMedia].sRtcpDestinationIp = vSplit[3];
                return true;
            }
            else if(vSplit[2] == "IP6" && ValidateIp6Address(vSplit[3]) != IP_INVALID)
            {
                tpReceivers[nMedia].sRtcpDestinationIp = vSplit[3];
                return true;
            }
        }
    }
    return false;
}

bool SdpManager::ParseMediaLine(const std::string& sLine, std::vector<TransportParamsRTPReceiver>& tpReceivers, size_t nMedia)
{
    pmlLog(pml::LOG_DEBUG) << "NMOS: " << "SdpManager::ParseMediaLine: " << sLine ;
    //format is media port proto etc. We want the port
    vector<string> vSplit;
    SplitString(vSplit, sLine, ' ');
    if(vSplit.size() > 1)
    {
        try
        {
            long nPort;
            nPort = stol(vSplit[1]);
            if(nPort > 0 && nPort < 65536)
            {
                tpReceivers[nMedia].nDestinationPort = nPort;
                return true;
            }
        }
        catch(invalid_argument& ia)
        {
            return false;
        }
    }
    return false;
}


bool SdpManager::TransportParamsToSdp(const TransportParamsRTPSender& tpSender, std::string& sSdp)
{
    // @todo convert tp to SDP

    return true;
}


SdpManager::enumIPType SdpManager::CheckIpAddress(const std::string& sAddress)
{
    enumIPType eType = ValidateIp4Address(sAddress);
    if(eType != IP_INVALID)
    {
        return eType;
    }

    return ValidateIp6Address(sAddress);

}
