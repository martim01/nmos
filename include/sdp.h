#pragma once
#include <string>
#include <vector>

struct TransportParamsRTPSender;
struct TransportParamsRTPReceiver;

class SdpManager
{
    public:
        SdpManager();

        static bool SdpToTransportParams(std::string sSdp, TransportParamsRTPReceiver& tpReceiver);
        static bool TransportParamsToSdp(const TransportParamsRTPSender& tpSender, std::string& sSdp);

    protected:
        static bool ParseConnectionLine(std::string sLine, TransportParamsRTPReceiver& tpReceiver);
        static bool ParseAttributeLine(std::string sLine, TransportParamsRTPReceiver& tpReceiver);
        static bool ParseMediaLine(std::string sLine, TransportParamsRTPReceiver& tpReceiver);

        static bool ParseConnectionIp4(std::string sAddress, TransportParamsRTPReceiver& tpReceiver);
        static bool ParseConnectionIp6(std::string sAddress, TransportParamsRTPReceiver& tpReceiver);

        static bool ParseSourceFilter(std::string sLine, TransportParamsRTPReceiver& tpReceiver);
        static bool ParseRTCP(std::string sLine, TransportParamsRTPReceiver& tpReceiver);


        enum enumAddress{ADDR_INVALID, ADDR_UNICAST, ADDR_MULTICAST};
        static enumAddress ValidateIp4Address(std::string sAddress);
        static enumAddress ValidateIp6Address(std::string sAddress);

        static const std::string STR_FILTER;
        static const std::string STR_RTCP;
};
