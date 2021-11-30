#pragma once
#include <string>
#include <vector>
#include <memory>

namespace pml
{
    namespace nmos
    {
        class TransportParamsRTPSender;
        class TransportParamsRTPReceiver;
        class Self;
        class Source;
        class Flow;
        class Sender;

        class SdpManager
        {
            public:
                SdpManager();

                static bool SdpToTransportParams(const std::string& sSdp, std::vector<TransportParamsRTPReceiver>& tpReceivers);
                static std::string TransportParamsToSdp(const Self& self, std::shared_ptr<const Sender> pSender, std::shared_ptr<const Flow> pFlow, std::shared_ptr<const Source> pSource);

                enum enumIPType{IP_INVALID, IP4_UNI, IP4_MULTI, IP6_UNI, IP6_MULTI};
                static enumIPType CheckIpAddress(const std::string& sAddress);

            protected:
                static bool ParseOriginLine(const std::string& sLine, std::vector<TransportParamsRTPReceiver>& tpReceivers);
                static bool ParseConnectionLine(const std::string& sLine, std::vector<TransportParamsRTPReceiver>& tpReceivers, size_t nMedia);
                static bool ParseAttributeLine(const std::string& sLine, std::vector<TransportParamsRTPReceiver>& tpReceivers, size_t nMedia);
                static bool ParseMediaLine(const std::string& sLine, std::vector<TransportParamsRTPReceiver>& tpReceivers, size_t nMedia);

                static bool ParseConnectionIp4(const std::string& sAddress, std::vector<TransportParamsRTPReceiver>& tpReceivers, size_t nMedia);
                static bool ParseConnectionIp6(const std::string& sAddress, std::vector<TransportParamsRTPReceiver>& tpReceivers, size_t nMedia);

                static bool ParseOriginIp4(const std::string& sAddress, std::vector<TransportParamsRTPReceiver>& tpReceivers);
                static bool ParseOriginIp6(const std::string& sAddress, std::vector<TransportParamsRTPReceiver>& tpReceivers);

                static bool ParseSourceFilter(const std::string& sLine, std::vector<TransportParamsRTPReceiver>& tpReceivers, size_t nMedia);
                static bool ParseRTCP(const std::string& sLine, std::vector<TransportParamsRTPReceiver>& tpReceivers, size_t nMedia);


                static enumIPType ValidateIp4Address(const std::string& sAddress);
                static enumIPType ValidateIp6Address(const std::string& sAddress);

                static std::string CreateOriginLine(const TransportParamsRTPSender& tpSender);
                static std::string CreateSessionLines(std::shared_ptr<const Sender> pSender);
                static std::string CreateClockLine(const Self& self, std::shared_ptr<const Source> pSource, const TransportParamsRTPSender& tpSender);
                static std::string CreateMediaLine(std::shared_ptr<const Flow> pFlow, unsigned short nPort);
                static std::string CreateConnectionLine(const TransportParamsRTPSender& tpSender);
                static std::string CreateAttributeLines(std::shared_ptr<const Flow> pFlow, std::shared_ptr<const Source> pSource);
                static std::string CreateRtcpLines(const TransportParamsRTPSender& tpSender);




                static const std::string STR_FILTER;
                static const std::string STR_RTCP;
        };
    };
};
