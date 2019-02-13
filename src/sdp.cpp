#include "sdp.h"
#include "transportparams.h"
#include "utils.h"
#include "log.h"

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
    Log::Get(Log::LOG_DEBUG) << "SdpManager::ParseConnectionLine: " << sLine << endl;
    //format is 'type addtypr address'
    vector<string> vSplit;
    SplitString(vSplit, sLine, ' ');
    if(vSplit.size() == 3 && vSplit[0] == "c=IN")
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
    Log::Get(Log::LOG_DEBUG) << "SdpManager::ParseConnectionLine: " << vSplit.size() << " " << vSplit[0] << endl;
    return false;
}

bool SdpManager::ParseConnectionIp4(string sAddress, TransportParamsRTPReceiver& tpReceiver)
{
    size_t nTTL = sAddress.find('/');
    switch(ValidateIp4Address(sAddress.substr(0, nTTL)))
    {
        case IP4_MULTI:
            tpReceiver.sMulticastIp = sAddress.substr(0, nTTL);
            tpReceiver.sSourceIp.clear();
            Log::Get(Log::LOG_DEBUG) << "SdpManager::ParseConnectionIp4: " << sAddress.substr(0, nTTL) << " multicast" << endl;
            //@todo do we care about TTL and possible address grouping??
            return true;
        case IP4_UNI:
            tpReceiver.sMulticastIp.clear();
            tpReceiver.sSourceIp = sAddress.substr(0, nTTL);
            Log::Get(Log::LOG_DEBUG) << "SdpManager::ParseConnectionIp4: " << sAddress << " unicast" << endl;
            //@todo do we care about TTL and possible address grouping??
            return true;
        default:
            Log::Get(Log::LOG_DEBUG) << "SdpManager::ParseConnectionIp4: " << sAddress << " invalid" << endl;
            return false;
    }
    return false;
}

bool SdpManager::ParseConnectionIp6(string sAddress, TransportParamsRTPReceiver& tpReceiver)
{
    size_t nGrouping = sAddress.find('/');
    switch(ValidateIp6Address(sAddress.substr(0, nGrouping)))
    {
        case IP6_MULTI:
            tpReceiver.sMulticastIp = sAddress.substr(0, nGrouping);
            //@todo do we care about TTL and possible address grouping??
            return true;
        case IP6_UNI:
            tpReceiver.sMulticastIp.clear();
            tpReceiver.sSourceIp = sAddress.substr(0, nGrouping);
            //@todo do we care about TTL and possible address grouping??
            return true;
        default:
            return false;
    }
    return false;
}

SdpManager::enumIPType SdpManager::ValidateIp4Address(string sAddress)
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

SdpManager::enumIPType SdpManager::ValidateIp6Address(std::string sAddress)
{
    //@todo ValidateIp6Address and workout unicast or multicast
    return IP_INVALID;
}

bool SdpManager::ParseAttributeLine(string sLine, TransportParamsRTPReceiver& tpReceiver)
{
    Log::Get(Log::LOG_DEBUG) << "SdpManager::ParseAttributeLine: " << sLine << endl;
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

    // @todo what other settings are possible here? Possibly FEC

    return true;
}

bool SdpManager::ParseSourceFilter(std::string sLine, TransportParamsRTPReceiver& tpReceiver)
{
    vector<string> vSplit;
    SplitString(vSplit, sLine, ' ');
    if(vSplit.size() > 4)
    {
        Log::Get(Log::LOG_DEBUG) << "SdpManager::ParseSourceFilter " << vSplit[0] << " " << vSplit[1] << " " << vSplit[2] << endl;
        if(vSplit[0] == "IN" && vSplit[1] == "incl")
        {
            if(vSplit[2] == "IP4" && ValidateIp4Address(vSplit[3]) == IP4_MULTI && ValidateIp4Address(vSplit[4]) == IP4_UNI)
            {
                tpReceiver.sSourceIp = vSplit[4];
                return true;
            }
            else if(vSplit[2] == "IP6" && ValidateIp6Address(vSplit[3]) == IP6_MULTI && ValidateIp6Address(vSplit[4]) == IP6_UNI)
            {
                tpReceiver.sSourceIp = vSplit[4];
                return true;
            }
        }
    }
    Log::Get(Log::LOG_DEBUG) << "SdpManager::ParseSourceFilter " << vSplit.size() << endl;
    return false;
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
            if(vSplit[2] == "IP4" && ValidateIp4Address(vSplit[3]) != IP_INVALID)
            {
                tpReceiver.sRtcpDestinationIp = vSplit[3];
                return true;
            }
            else if(vSplit[2] == "IP6" && ValidateIp6Address(vSplit[3]) != IP_INVALID)
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
    Log::Get(Log::LOG_DEBUG) << "SdpManager::ParseMediaLine: " << sLine << endl;
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


SdpManager::enumIPType SdpManager::CheckIpAddress(std::string sAddress)
{
    enumIPType eType = ValidateIp4Address(sAddress);
    if(eType != IP_INVALID)
    {
        return eType;
    }

    return ValidateIp6Address(sAddress);

}
