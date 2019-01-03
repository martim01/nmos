#pragma once
#include <string>
#include "json/json.h"

struct TransportParamsRTP
{
    enum enumFecMode {ONED, TWOD};

    TransportParamsRTP();
    virtual bool Patch(const Json::Value& jsData);

    virtual Json::Value GetJson() const;
    void SetPort(Json::Value& js, const std::string& sPort, unsigned short nPort) const;
    bool DecodePort(const Json::Value& jsData, const std::string& sPort, unsigned short& nPort);

    void Actualize();

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

    static const std::string STR_FEC_MODE[2];

};

struct TransportParamsRTPReceiver : public TransportParamsRTP
{
    std::string sMulticastIp;
    std::string sInterfaceIp;

    TransportParamsRTPReceiver();
    virtual bool Patch(const Json::Value& jsData);
    virtual Json::Value GetJson() const;
    void Actualize(const std::string& sInterfaceIp);
};


struct TransportParamsRTPSender : public TransportParamsRTP
{
    enum enumFecType {XOR, REED};

    TransportParamsRTPSender();
    virtual bool Patch(const Json::Value& jsData);

    virtual Json::Value GetJson() const;

    void Actualize(const std::string& sSource, const std::string& sDestination);

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



