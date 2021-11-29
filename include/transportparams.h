#pragma once
#include <string>
#include "json/json.h"
#include "nmosapiversion.h"
#include "optional.hpp"
#include "constraint.h"

namespace pml
{
    namespace nmos
    {
        struct TransportParams
        {
            public:

                TransportParams() : m_json(Json::objectValue){}
                TransportParams(const TransportParams& tp);
                TransportParams& operator=(const TransportParams& tp);

                virtual ~TransportParams(){}

                const Json::Value& GetJson() const { return m_json;}



            protected:
                Json::Value m_json;

        };

        struct TransportParamsRTP : public TransportParams
        {
            public:
                enum flagsTP { CORE, FEC=1, RTCP=2, MULTICAST=4, REDUNDANT=8};

                TransportParamsRTP();
                TransportParamsRTP(const TransportParamsRTP& tp);
                TransportParamsRTP& operator=(const TransportParamsRTP& other);

                void Actualize();

                virtual void FecAllowed();
                virtual void RtcpAllowed();

                std::string GetSourceIP() const;
                unsigned short GetDestinationPort() const;

                bool IsRtpEnabled() const;

                std::experimental::optional<bool> IsFecEnabled() const;
                std::experimental::optional<std::string> GetFecDestinationIp() const;
                std::experimental::optional<std::string> GetFecMode() const;
                std::experimental::optional<uint32_t> GetFec1DDestinationPort() const;
                std::experimental::optional<uint32_t> GetFec2DDestinationPort() const;

                std::experimental::optional<bool> IsRtcpEnabled() const;
                std::experimental::optional<std::string> GetRtcpDestinationIp() const;
                std::experimental::optional<uint32_t> GetRtcpDestinationPort() const;


                void EnableRtp(bool bEnable);
                void SetSourceIp(const std::string& sAddress);
                void SetDestinationPort(unsigned short nPort);

                void SetRtcpDestinationPort(unsigned short nPort);
                void SetRtcpDestinationIp(const std::string& sAddress);

                static const std::string STR_FEC_TYPE_XOR;
                static const std::string STR_FEC_TYPE_REED;
                static const std::string AUTO;
                static const std::string SOURCE_IP;
                static const std::string SOURCE_PORT;
                static const std::string DESTINATION_PORT;
                static const std::string DESTINATION_IP;
                static const std::string MULTICAST_IP;
                static const std::string INTERFACE_IP;
                static const std::string RTP_ENABLED;
                static const std::string FEC_ENABLED;
                static const std::string FEC_DESTINATION_IP;
                static const std::string FEC_MODE;
                static const std::string FEC_1DDESTINATION_PORT;
                static const std::string FEC_2DDESTINATION_PORT;
                static const std::string FEC_TYPE;
                static const std::string FEC_BLOCK_WIDTH;
                static const std::string FEC_BLOCK_HEIGHT;
                static const std::string FEC_1DSOURCE_PORT;
                static const std::string FEC_2DSOURCE_PORT;
                static const std::string RTCP_ENABLED;
                static const std::string RTCP_DESTINATION_IP;
                static const std::string RTCP_DESTINATION_PORT;
                static const std::string RTCP_SOURCE_PORT;


            protected:

                void ActualizePort(const std::string& sKey, unsigned short nPort);
                void ActualizeIp(const std::string& sKey, const std::string& sIp);

                void SetPort(const std::string& sKey, unsigned short nPort);
        };

        struct TransportParamsRTPReceiver : public TransportParamsRTP
        {

            public:
                TransportParamsRTPReceiver(flagsTP allowed);
                TransportParamsRTPReceiver(const TransportParamsRTPReceiver& tp);
                TransportParamsRTPReceiver& operator=(const TransportParamsRTPReceiver& other);

                bool Patch(const Json::Value& jsData);

                void Actualize(const std::string& sInterfaceIp);

                std::experimental::optional<std::string> GetMulticastIp() const;
                std::string GetInterfaceIp() const;

                void SetMulticastIp(const std::string& sAddress);

                static bool CheckJson(const Json::Value&  jsPatch);
        };


        struct TransportParamsRTPSender : public TransportParamsRTP
        {
            public:

                TransportParamsRTPSender(flagsTP allowed);
                TransportParamsRTPSender(const TransportParamsRTPSender& tp);
                TransportParamsRTPSender& operator=(const TransportParamsRTPSender& other);

                void FecAllowed() override;

                bool Patch(const Json::Value& jsData);

                void Actualize(const std::string& sSource, const std::string& sDestination);


                std::string GetDestinationIp() const;
                unsigned short GetSourcePort() const;

                void SetDestinationIp(const std::string& sAddress);

                std::experimental::optional<std::string> GetFecType() const;
                std::experimental::optional<uint32_t> GetFecBlockWidth() const;
                std::experimental::optional<uint32_t> GetFecBlockHeight() const;
                std::experimental::optional<uint32_t> GetFec1DSourcePort() const;
                std::experimental::optional<uint32_t> GetFec2DSourcePort() const;

                std::experimental::optional<uint32_t> GetRtcpSourcePort() const;

                static bool CheckJson(const Json::Value&  jsPatch);

        };
    };
};


