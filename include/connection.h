#pragma once
#include "version.h"
#include "transportparams.h"
#include "nmosdlldefine.h"
#include <chrono>

namespace pml
{
    namespace nmos
    {
            struct NMOS_EXPOSE connection
            {
                enum enumActivate {ACT_NULL, ACT_NOW, ACT_ABSOLUTE, ACT_RELATIVE};
                enum {FP_ACTIVATION=1, FP_ENABLE=2, FP_TRANSPORT_PARAMS=4, FP_TRANSPORT_FILE=8, FP_ID=16, FP_ALL=31};
                connection();

                connection(int flagProperties);
                connection(const connection& conReq);
                connection& operator=(const connection& other);
                virtual bool Patch(const Json::Value& jsData);
                virtual Json::Value GetJson(const ApiVersion& version) const;


                bool bMasterEnable;
                enumActivate eActivate;
                std::string sRequestedTime;
                std::string sActivationTime;
                std::chrono::time_point<std::chrono::high_resolution_clock> tpActivation;

                int nProperties;
                static const std::string STR_ACTIVATE[4];
            };

            struct NMOS_EXPOSE connectionSender : public connection
            {

                connectionSender();
                connectionSender(int flagProperties);
                connectionSender(const connectionSender& conReq);
                connectionSender& operator=(const connectionSender& other);
                virtual bool Patch(const Json::Value& jsData);

                virtual Json::Value GetJson(const ApiVersion& version) const;

                // @todo we should have an array of senders for SMPTE2022
                TransportParamsRTPSender tpSender;
                std::string sReceiverId;


            };

            //Connection API
            struct NMOS_EXPOSE connectionReceiver : public connection
            {
                connectionReceiver();
                connectionReceiver(const connectionReceiver& conReq);

                connectionReceiver(int flagProperties);
                connectionReceiver& operator=(const connectionReceiver& other);
                virtual bool Patch(const Json::Value& jsData);

                virtual Json::Value GetJson(const ApiVersion& version) const;

                // @todo we should have an array of TransportParamsRTPReceiver for SMPTE2022
                TransportParamsRTPReceiver tpReceiver;
                std::string sSenderId;
                std::string sTransportFileType;
                std::string sTransportFileData;


        };
    };
};
