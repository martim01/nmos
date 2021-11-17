#pragma once
#include <string>
#include "json/json.h"
#include "nmosapiversion.h"

namespace pml
{
    namespace nmos
    {
        struct TransportParamsRTP
        {
            enum enumFecMode {ONED, TWOD};
            enum enumTParams {TP_UNKNOWN = -1, TP_NOT_SUPPORTED=0, TP_SUPPORTED=1};
            enum flagsTP {CORE=0, FEC=1, RTCP=2, MULTICAST=4};
            TransportParamsRTP();
            TransportParamsRTP(const TransportParamsRTP& tp);

            virtual bool Patch(const Json::Value& jsData);

            virtual Json::Value GetJson(const ApiVersion& version) const;
            void SetPort(Json::Value& js, const std::string& sPort, unsigned short nPort) const;
            bool DecodePort(const Json::Value& jsData, const std::string& sPort, unsigned short& nPort);

            void Actualize();

            TransportParamsRTP& operator=(const TransportParamsRTP& other);

            std::string sSourceIp;
            unsigned short nDestinationPort;
            bool bFecEnabled;
            std::string sFecDestinationIp;
            enumFecMode eFecMode;
            unsigned short nFec1DDestinationPort;
            unsigned short nFec2DDestinationPort;
            bool bRtcpEnabled;
            std::string sRtcpDestinationIp;
            unsigned short nRtcpDestinationPort;
            bool bRtpEnabled;

            enumTParams eFec;
            enumTParams eRTCP;
            enumTParams eMulticast;
            static const std::string STR_FEC_MODE[2];

        };

        struct TransportParamsRTPReceiver : public TransportParamsRTP
        {

            std::string sMulticastIp;
            std::string sInterfaceIp;

            TransportParamsRTPReceiver();
            TransportParamsRTPReceiver(const TransportParamsRTPReceiver& tp);
            virtual bool Patch(const Json::Value& jsData);
            virtual Json::Value GetJson(const ApiVersion& version) const;
            void Actualize(const std::string& sInterfaceIp);
            TransportParamsRTPReceiver& operator=(const TransportParamsRTPReceiver& other);
        };


        struct TransportParamsRTPSender : public TransportParamsRTP
        {
            enum enumFecType {XOR, REED};

            TransportParamsRTPSender();
            TransportParamsRTPSender(const TransportParamsRTPSender& tp);
            virtual bool Patch(const Json::Value& jsData);

            virtual Json::Value GetJson(const ApiVersion& version) const;

            void Actualize(const std::string& sSource, const std::string& sDestination);

            TransportParamsRTPSender& operator=(const TransportParamsRTPSender& other);

            std::string sDestinationIp;
            unsigned short nSourcePort;
            enumFecType eFecType;
            unsigned char nFecBlockWidth;
            unsigned char nFecBlockHeight;
            unsigned short nFec1DSourcePort;
            unsigned short nFec2DSourcePort;
            unsigned short nRtcpSourcePort;




            static const std::string STR_FEC_TYPE[2];
        };
    };
};


