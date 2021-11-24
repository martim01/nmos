#pragma once
#include <string>
#include "json/json.h"
#include "nmosapiversion.h"
#include "optional.hpp"

namespace pml
{
    namespace nmos
    {
        struct TransportParams
        {
            public:
                TransportParams(){}
                virtual ~TransportParams(){}
                virtual bool Patch(const Json::Value& jsData)=0;
                virtual Json::Value GetJson(const ApiVersion& version) const=0;


        };

        struct TransportParamsRTP : public TransportParams
        {
            enum enumFecMode {ONED, TWOD};
            enum enumTParams {TP_UNKNOWN = -1, TP_NOT_SUPPORTED=0, TP_SUPPORTED=1};
            enum flagsTP {CORE=0, FEC=1, RTCP=2, MULTICAST=4, REDUNDANT=8};
            TransportParamsRTP();
            TransportParamsRTP(const TransportParamsRTP& tp);

            virtual bool Patch(const Json::Value& jsData) override;
            virtual Json::Value GetJson(const ApiVersion& version) const override;

            void SetPort(Json::Value& js, const std::string& sPort, unsigned short nPort) const;
            bool DecodePort(const Json::Value& jsData, const std::string& sPort, unsigned short& nPort);

            void Actualize();

            TransportParamsRTP& operator=(const TransportParamsRTP& other);

            virtual void FecAllowed();
            virtual void RtcpAllowed();

            std::string sSourceIp;
            unsigned short nDestinationPort;
            bool bFecEnabled;

            std::experimental::optional<std::string> sFecDestinationIp;
            std::experimental::optional<enumFecMode> eFecMode;
            std::experimental::optional<unsigned short> nFec1DDestinationPort;
            std::experimental::optional<unsigned short> nFec2DDestinationPort;

            std::experimental::optional<bool> bRtcpEnabled;
            std::experimental::optional<std::string> sRtcpDestinationIp;
            std::experimental::optional<unsigned short> nRtcpDestinationPort;

            bool bRtpEnabled;

            //std::experimental::optional<enumTParams eFec;
            //std::experimental::optional<enumTParams eRTCP;
            //enumTParams eMulticast;
            static const std::string STR_FEC_MODE[2];

        };

        struct TransportParamsRTPReceiver : public TransportParamsRTP
        {

            std::experimental::optional<std::string> sMulticastIp;
            std::string sInterfaceIp;

            TransportParamsRTPReceiver();
            TransportParamsRTPReceiver(const TransportParamsRTPReceiver& tp);
            bool Patch(const Json::Value& jsData) override;
            Json::Value GetJson(const ApiVersion& version) const override;
            void Actualize(const std::string& sInterfaceIp);
            TransportParamsRTPReceiver& operator=(const TransportParamsRTPReceiver& other);


        };


        struct TransportParamsRTPSender : public TransportParamsRTP
        {
            enum enumFecType {XOR, REED};

            TransportParamsRTPSender();
            TransportParamsRTPSender(const TransportParamsRTPSender& tp);
            bool Patch(const Json::Value& jsData) override;

            Json::Value GetJson(const ApiVersion& version) const override;

            void Actualize(const std::string& sSource, const std::string& sDestination);

            TransportParamsRTPSender& operator=(const TransportParamsRTPSender& other);

            void FecAllowed() override;

            std::string sDestinationIp;
            unsigned short nSourcePort;
            std::experimental::optional<enumFecType> eFecType;
            std::experimental::optional<unsigned char> nFecBlockWidth;
            std::experimental::optional<unsigned char> nFecBlockHeight;
            std::experimental::optional<unsigned short> nFec1DSourcePort;
            std::experimental::optional<unsigned short> nFec2DSourcePort;
            std::experimental::optional<unsigned short> nRtcpSourcePort;



            static const std::string STR_FEC_TYPE[2];
        };
    };
};


