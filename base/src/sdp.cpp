#include "sdp.h"
#include "transportparams.h"
#include "utils.h"
#include "log.h"
#include "senderbase.h"
#include "flow.h"
#include "source.h"
#include "self.h"


using namespace std;
using namespace pml::nmos;

const string SdpManager::STR_FILTER = "a=source-filter:";
const string SdpManager::STR_RTCP = "a=rtcp:";

bool SdpManager::SdpToTransportParams(const std::string& sSdp, std::vector<TransportParamsRTPReceiver>& tpReceivers)
{
    for(auto& tp : tpReceivers)
    {
        tp.EnableRtp(false);
    }

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
                    pmlLog(pml::LOG_DEBUG, "pml::nmos") << "NMOS: " << "SdpManager:: ParseOriginLine failed";
                    return false;
                }
                break;
            case 'c':
                if(ParseConnectionLine(sLine, tpReceivers,nMedia) == false)
                {
                    pmlLog(pml::LOG_DEBUG, "pml::nmos") << "NMOS: " << "SdpManager:: ParseConnectionLine failed";
                    return false;
                }
                break;
            case 'a':
                if(ParseAttributeLine(sLine, tpReceivers,nMedia) == false)
                {
                    pmlLog(pml::LOG_DEBUG, "pml::nmos") << "NMOS: " << "SdpManager:: ParseAttributeLine failed";
                    return false;
                }
                break;
            case 'm':
                ++nMedia;
                if(nMedia < tpReceivers.size() && ParseMediaLine(sLine, tpReceivers,nMedia) == false)
                {
                    pmlLog(pml::LOG_DEBUG, "pml::nmos") << "NMOS: " << "SdpManager:: ParseMediaLine failed";
                    return false;
                }

                break;
        }

    }

    for(auto& tp : tpReceivers)
    {
        tp.EnableRtp(true);
    }
    return true;
}

bool SdpManager::ParseOriginLine(const std::string& sLine, std::vector<TransportParamsRTPReceiver>& tpReceivers)
{
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
    return false;
}

bool SdpManager::ParseConnectionIp4(const std::string& sAddress, std::vector<TransportParamsRTPReceiver>& tpReceivers, size_t nMedia)
{
    size_t nTTL = sAddress.find('/');
    switch(ValidateIp4Address(sAddress.substr(0, nTTL)))
    {
        case IP4_MULTI:
            tpReceivers[nMedia].SetMulticastIp(sAddress.substr(0, nTTL));
            return true;
        case IP4_UNI:
            tpReceivers[nMedia].SetMulticastIp("");
            return true;
        default:
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
            tpReceivers[nMedia].SetMulticastIp(sAddress.substr(0, nGrouping));
            //@todo do we care about TTL and possible address grouping??
            return true;
        case IP6_UNI:
            tpReceivers[nMedia].SetMulticastIp("");
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
                tpReceiver.SetSourceIp(std::string());
            }
            return false;
        case IP4_UNI:
            for(auto& tpReceiver : tpReceivers)
            {
                tpReceiver.SetSourceIp(sAddress);
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
               tpReceiver.SetSourceIp(std::string());
           }
           return false;
        case IP6_UNI:
            for(auto& tpReceiver : tpReceivers)
            {
                tpReceiver.SetSourceIp(sAddress);
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
        if((vSplit[0] == "IN" && vSplit[1] == "incl") || (vSplit[1] == "IN" && vSplit[0] == "incl"))
        {
            if(vSplit[2] == "IP4" && ValidateIp4Address(vSplit[3]) == IP4_MULTI && ValidateIp4Address(vSplit[4]) == IP4_UNI)
            {
                tpReceivers[nMedia].SetSourceIp(vSplit[4]);
                return true;
            }
            else if(vSplit[2] == "IP6" && ValidateIp6Address(vSplit[3]) == IP6_MULTI && ValidateIp6Address(vSplit[4]) == IP6_UNI)
            {
                tpReceivers[nMedia].SetSourceIp(vSplit[4]);
                return true;
            }
        }
        else
        {
            pmlLog(pml::LOG_DEBUG, "pml::nmos") << "NMOS: " << "SdpManager::ParseSourceFilter failed: " << sLine;
        }
    }

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
                tpReceivers[nMedia].SetRtcpDestinationPort(nPort);
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
                tpReceivers[nMedia].SetRtcpDestinationIp(vSplit[3]);
                return true;
            }
            else if(vSplit[2] == "IP6" && ValidateIp6Address(vSplit[3]) != IP_INVALID)
            {
                tpReceivers[nMedia].SetRtcpDestinationIp(vSplit[3]);
                return true;
            }
        }
    }
    return false;
}

bool SdpManager::ParseMediaLine(const std::string& sLine, std::vector<TransportParamsRTPReceiver>& tpReceivers, size_t nMedia)
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
                tpReceivers[nMedia].SetDestinationPort(nPort);
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


std::string SdpManager::TransportParamsToSdp(const Self& self, std::shared_ptr<const Sender> pSender, std::shared_ptr<const Flow> pFlow, std::shared_ptr<const Source> pSource)
{
    auto active = pSender->GetActive();
    std::string sSDP = CreateOriginLine(active.GetTransportParams()[0]);    //@todo what should we use for the origin address??
    sSDP += CreateSessionLines(pSender);

    if(active.GetTransportParams().size() > 1)
    {
        std::string sGroup;
        for(size_t nStream = 1; nStream <= active.GetTransportParams().size(); ++nStream)
        {
            if(sGroup.empty() == false)
            {
                sGroup += " ";
            }
            sGroup += "S"+std::to_string(nStream);
        }
        sSDP += "a=group:DUP "+sGroup+"\r\n";
    }

    auto nStream = 1;
    for(const auto& tpSender: active.GetTransportParams())
    {
        sSDP += CreateMediaLine(pFlow, tpSender.GetDestinationPort());
        sSDP += CreateConnectionLine(tpSender);
        sSDP += CreateClockLine(self, pSource, tpSender);
        sSDP += CreateAttributeLines(pFlow, pSource);
        sSDP += CreateRtcpLines(tpSender);

        if(active.GetTransportParams().size() > 1)
        {
            sSDP += "a=mid:S"+std::to_string(nStream)+"\r\n";
            ++nStream;
        }
        
    }
    return sSDP;
}

std::string SdpManager::CreateOriginLine(const TransportParamsRTPSender& tpSender)
{
    //Create origin line
    std::string sLine = "v=0\r\n";
    sLine += "o=- " + GetCurrentTaiTime(false) + " " + GetCurrentTaiTime(false) + " IN IP";
    switch(SdpManager::CheckIpAddress(tpSender.GetSourceIp()))
    {
        case SdpManager::IP4_UNI:
        case SdpManager::IP4_MULTI:
            sLine += "4 ";
            break;
        case SdpManager::IP6_UNI:
        case SdpManager::IP6_MULTI:
            sLine += "6 ";
            break;
        case SdpManager::IP_INVALID:
            sLine += " ";
            break;
    }
    sLine += tpSender.GetSourceIp() + "\r\n";
    return sLine;

}

std::string SdpManager::CreateSessionLines(std::shared_ptr<const Sender> pSender)
{
    std::string sLine = "s=" + pSender->GetLabel() + "\r\n";
    //create a session information line
    //sLine += "i=" + "\r\n";
    //create time line
    sLine += "t=0 0 \r\n";
    return sLine;
}

std::string SdpManager::CreateClockLine(const Self& self, std::shared_ptr<const Source> pSource, const TransportParamsRTPSender& tpSender)
{
    //get clock name from source
    auto sClock = pSource->GetClock();
    return self.CreateSDPClockLine(sClock, tpSender.GetSourceIp());
}

std::string SdpManager::CreateMediaLine(std::shared_ptr<const Flow> pFlow, unsigned short nPort)
{
    return pFlow->CreateSDPMediaLine(nPort==0 ? 5004 : nPort);
}

std::string SdpManager::CreateConnectionLine(const TransportParamsRTPSender& tpSender)
{
    std::string sLine;
    switch(CheckIpAddress(tpSender.GetDestinationIp()))
    {
        case SdpManager::IP4_UNI:
            sLine = "c=IN IP4 " + tpSender.GetDestinationIp() + "\r\n";
            break;
        case SdpManager::IP4_MULTI:
            sLine = "c=IN IP4 " + tpSender.GetDestinationIp() + "/32\r\n";
            sLine += "a=source-filter: incl IN IP4 " + tpSender.GetDestinationIp() + " " + tpSender.GetSourceIp() + "\r\n";
            break;
        case SdpManager::IP6_UNI:
            sLine = "c=IN IP6 " + tpSender.GetDestinationIp() + "\r\n";
            break;
        case SdpManager::IP6_MULTI:
            sLine = "c=IN IP6 " + tpSender.GetDestinationIp() + "\r\n";
            sLine += "a=source-filter: incl IN IP6 " + tpSender.GetDestinationIp() + " " + tpSender.GetSourceIp() + "\r\n";
            break;
        case SdpManager::SdpManager::IP_INVALID:
            pmlLog(pml::LOG_WARN, "pml::nmos") << "NMOS: Sender can't create SDP - destination IP invalid '" << tpSender.GetDestinationIp() << "'";
            break;
    }
    return sLine;
}

std::string SdpManager::CreateAttributeLines(std::shared_ptr<const Flow> pFlow, std::shared_ptr<const Source> pSource)
{
    return pFlow->CreateSDPAttributeLines(pSource);
}

std::string SdpManager::CreateRtcpLines(const TransportParamsRTPSender& tpSender)
{
    std::stringstream ssLine;
    if(tpSender.IsRtcpEnabled() && *tpSender.IsRtcpEnabled() && tpSender.GetRtcpDestinationIp() && tpSender.GetRtcpDestinationPort())
    {
        switch(CheckIpAddress(*tpSender.GetRtcpDestinationIp()))
        {
            case SdpManager::IP4_UNI:
            case SdpManager::IP4_MULTI:
                ssLine << "a=rtcp:" << *tpSender.GetRtcpDestinationPort() << " IN IP4 " << *tpSender.GetRtcpDestinationIp() << "\r\n";
                break;
            case SdpManager::IP6_UNI:
            case SdpManager::IP6_MULTI:
                ssLine << "a=rtcp:" << *tpSender.GetRtcpDestinationPort() << " IN IP6 " << *tpSender.GetRtcpDestinationIp() << "\r\n";
                break;
            default:
                break;
        }
    }
    return ssLine.str();
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
