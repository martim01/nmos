#pragma once

#include "transportparams.h"

struct connection
{
    enum enumActivate {ACT_NULL, ACT_NOW, ACT_ABSOLUTE, ACT_RELATIVE};

    connection();

    virtual bool Patch(const Json::Value& jsData);

    virtual Json::Value GetJson() const;

    bool bMasterEnable;
    enumActivate eActivate;
    std::string sRequestedTime;
    std::string sActivationTime;

    static const std::string STR_ACTIVATE[4];
};

struct connectionSender : public connection
{

    connectionSender();
    virtual bool Patch(const Json::Value& jsData);

    virtual Json::Value GetJson() const;

    TransportParamsRTPSender tpSender;
    std::string sReceiverId;


};

//Connection API
struct connectionReceiver : public connection
{
    connectionReceiver();

    virtual bool Patch(const Json::Value& jsData);

    virtual Json::Value GetJson() const;

    TransportParamsRTPReceiver tpReceiver;
    std::string sSenderId;
    std::string sTransportFileType;
    std::string sTransportFileData;
    static const std::string STR_ACTIVATE[4];

};
