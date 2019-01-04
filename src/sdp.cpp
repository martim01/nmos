#include "sdp.h"
#include "transportparams.h"
#include "utils.h"

using namespace std;

const string SdpManager::STR_FILTER = "a=source-filter:";
const string SdpManager::STR_RTCP = "a=rtcp:";

bool SdpManager::SdpToTransportParams(string sSdp, TransportParamsRTPReceiver& tpReceiver)
{
    istringstream f(sSdp);
    string sLine;
    while (getline(f, sLine,'\n'))
    {
        switch(sLine[0])
        {
        case 'c':
            if(ParseConnectionLine(sLine, tpReceiver) == false)
            {
                return false;
            }
            break;
        case 'a':
            if(ParseAttributeLine(sLine, tpReceiver) == false)
            {
                return false;
            }
            break;
        case 'm':
            if(ParseMediaLine(sLine, tpReceiver) == false)
            {
                return false;
            }
            break;
        }

    }
    return true;
}

bool SdpManager::ParseConnectionLine(string sLine, TransportParamsRTPReceiver& tpReceiver)
{
    //format is 'type addtypr address'
    vector<string> vSplit;
    SplitString(vSplit, sLine, ' ');
    if(vSplit.size() == 3 && vSplit[0] == "IN")
    {
        if(vSplit[1] == "IP4")
        {
            return ParseConnectionIp4(vSplit[2], tpReceiver);
        }
        else if(vSplit[1] == "IP4")
        {
            return ParseConnectionIp6(vSplit[2], tpReceiver);
        }
    }
    return false;
}

bool SdpManager::ParseConnectionIp4(string sAddress, TransportParamsRTPReceiver& tpReceiver)
{
    size_t nTTL = sAddress.find('/');
    switch(ValidateIp4Address(sAddress.substr(0, nTTL)))
    {
        case ADDR_INVALID:
            return false;
        case ADDR_MULTICAST:
            tpReceiver.sMulticastIp = sAddress.substr(0, nTTL);
            //@todo do we care about TTL and possible address grouping??
            break;
        case ADDR_UNICAST:
            tpReceiver.sMulticastIp.clear();
            tpReceiver.sSourceIp = sAddress.substr(0, nTTL);
            //@todo do we care about TTL and possible address grouping??
            break;

        return true;
    }
    return false;
}

bool SdpManager::ParseConnectionIp6(string sAddress, TransportParamsRTPReceiver& tpReceiver)
{
    size_t nGrouping = sAddress.find('/');
    switch(ValidateIp6Address(sAddress.substr(0, nGrouping)))
    {
        case ADDR_INVALID:
            return false;
        case ADDR_MULTICAST:
            tpReceiver.sMulticastIp = sAddress.substr(0, nGrouping);
            //@todo do we care about TTL and possible address grouping??
            break;
        case ADDR_UNICAST:
            tpReceiver.sMulticastIp.clear();
            tpReceiver.sSourceIp = sAddress.substr(0, nGrouping);
            //@todo do we care about TTL and possible address grouping??
            break;
        return true;
    }
    return false;
}

SdpManager::enumAddress SdpManager::ValidateIp4Address(string sAddress)
{
    vector<string> vSplit;
    SplitString(vSplit, sAddress, '.');
    if(vSplit.size() != 4)
    {
        return ADDR_INVALID;
    }
    vector<long> vValue;
    vValue.resize(4);
    for(size_t i = 0; i < 4; i++)
    {

        try
        {
            if(vSplit[i].empty())
            {
                return ADDR_INVALID;
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
                return ADDR_INVALID;
            }
        }
        catch(invalid_argument& ia)
        {
            return ADDR_INVALID;
        }
    }
    //check if multicast address or unicast
    if(vValue[0] >= 224 && vValue[0] <= 239)
    {
        return ADDR_MULTICAST;
    }

    return ADDR_UNICAST;
}

SdpManager::enumAddress SdpManager::ValidateIp6Address(std::string sAddress)
{
    //@todo
    return ADDR_INVALID;
}

bool SdpManager::ParseAttributeLine(string sLine, TransportParamsRTPReceiver& tpReceiver)
{
    //find source filter line for ssmc destination address
    size_t nStart = sLine.find(STR_FILTER);
    if(nStart != string::npos)
    {
        return ParseSourceFilter(sLine.substr(nStart+STR_FILTER.length()), tpReceiver);
    }
    nStart = sLine.find(STR_RTCP);
    if(nStart != string::npos)
    {
        return ParseRTCP(sLine.substr(nStart+STR_RTCP.length()), tpReceiver);
    }

}

bool SdpManager::ParseSourceFilter(std::string sLine, TransportParamsRTPReceiver& tpReceiver)
{
    vector<string> vSplit;
    SplitString(vSplit, sLine, ' ');
    if(vSplit.size() > 4)
    {
        if(vSplit[0] == "IN" && vSplit[1] == "incl")
        {
            if(vSplit[2] == "IP4" && ValidateIp4Address(vSplit[3]) == ADDR_MULTICAST && ValidateIp4Address(vSplit[4]) == ADDR_UNICAST)
            {
                tpReceiver.sSourceIp = vSplit[4];
                return true;
            }
            else if(vSplit[2] == "IP6" && ValidateIp6Address(vSplit[3]) == ADDR_MULTICAST && ValidateIp6Address(vSplit[4]) == ADDR_UNICAST)
            {
                tpReceiver.sSourceIp = vSplit[4];
                return true;
            }
        }
    }
}

bool SdpManager::ParseRTCP(std::string sLine, TransportParamsRTPReceiver& tpReceiver)
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
                tpReceiver.nRtcpDestinationPort = nPort;
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
            if(vSplit[2] == "IP4" && ValidateIp4Address(vSplit[3]) != ADDR_INVALID)
            {
                tpReceiver.sRtcpDestinationIp = vSplit[3];
                return true;
            }
            else if(vSplit[2] == "IP6" && ValidateIp6Address(vSplit[3]) != ADDR_INVALID)
            {
                tpReceiver.sRtcpDestinationIp = vSplit[3];
                return true;
            }
        }
    }
    return false;
}

bool SdpManager::ParseMediaLine(string sLine, TransportParamsRTPReceiver& tpReceiver)
{
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
                tpReceiver.nDestinationPort = nPort;
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
