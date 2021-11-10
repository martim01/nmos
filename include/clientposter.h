#pragma once
#include "eventposter.h"
#include "zcposter.h"

namespace pml
{
    namespace nmos
    {

        class ClientPoster : public EventPoster
        {
            public:
                ClientPoster(){}
                enum enumCurlType {CURLTYPE_SENDER_STAGED=0, CURLTYPE_SENDER_ACTIVE, CURLTYPE_SENDER_TRANSPORTFILE, CURLTYPE_RECEIVER_STAGED, CURLTYPE_RECEIVER_ACTIVE, CURLTYPE_SENDER_PATCH, CURLTYPE_RECEIVER_PATCH, CURLTYPE_TARGET};
                static const std::string STR_TYPE[5];
            protected:

                virtual void CurlDone(unsigned long nResult, const std::string& sResponse, long nType, const std::string& sResourceId);
                virtual void Target(const std::string& sReceiverId, const std::string& sTransportFile, unsigned short nPort);
                virtual void PatchSender(const std::string& sSenderId, const connectionSender& conPatch, unsigned short nPort);
                virtual void PatchReceiver(const std::string& sReceiverId, const connectionReceiver& conPatch, unsigned short nPort);
                virtual void SenderActivated(const std::string& sSenderId);
                virtual void ReceiverActivated(const std::string& sReceiverId);
        };

        class ClientZCPoster : public ZCPoster
        {
            public:
                ClientZCPoster(){}

            protected:

                virtual void InstanceResolved(std::shared_ptr<dnsInstance> pInstance);
                virtual void AllForNow(const std::string& sService);
                virtual void Finished();
                virtual void RegistrationNodeError();
                virtual void InstanceRemoved(std::shared_ptr<dnsInstance> pInstance);

        };
    };
};
