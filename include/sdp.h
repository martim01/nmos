#pragma once
#include <string>
#include <vector>

struct TransportParamsRTPSender;
struct TransportParamsRTPReceiver;

class SdpManager
{
    public:
        SdpManager();

        static bool SdpToTransportParams(const std::string& sSdp, TransportParamsRTPReceiver& tpReceiver);
        static bool TransportParamsToSdp(const TransportParamsRTPSender& tpSender, std::string& sSdp);

        enum enumIPType{IP_INVALID, IP4_UNI, IP4_MULTI, IP6_UNI, IP6_MULTI};
        static enumIPType CheckIpAddress(const std::string& sAddress);

    protected:
        static bool ParseOriginLine(const std::string& sLine, TransportParamsRTPReceiver& tpReceiver);
        static bool ParseConnectionLine(const std::string& sLine, TransportParamsRTPReceiver& tpReceiver);
        static bool ParseAttributeLine(const std::string& sLine, TransportParamsRTPReceiver& tpReceiver);
        static bool ParseMediaLine(const std::string& sLine, TransportParamsRTPReceiver& tpReceiver);

        static bool ParseConnectionIp4(const std::string& sAddress, TransportParamsRTPReceiver& tpReceiver);
        static bool ParseConnectionIp6(const std::string& sAddress, TransportParamsRTPReceiver& tpReceiver);

        static bool ParseOriginIp4(const std::string& sAddress, TransportParamsRTPReceiver& tpReceiver);
        static bool ParseOriginIp6(const std::string& sAddress, TransportParamsRTPReceiver& tpReceiver);

        static bool ParseSourceFilter(const std::string& sLine, TransportParamsRTPReceiver& tpReceiver);
        static bool ParseRTCP(const std::string& sLine, TransportParamsRTPReceiver& tpReceiver);


        static enumIPType ValidateIp4Address(const std::string& sAddress);
        static enumIPType ValidateIp6Address(const std::string& sAddress);



        static const std::string STR_FILTER;
        static const std::string STR_RTCP;
};
