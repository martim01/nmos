#pragma once
#include "eventposter.h"

class ClientPoster : public EventPoster
{
    public:
        ClientPoster(){}
        enum {CURLTYPE_TARGET};
    protected:

        virtual void CurlDone(unsigned long nResult, const std::string& sResponse, long nType);
        virtual void InstanceResolved(std::shared_ptr<dnsInstance> pInstance);
        virtual void AllForNow(const std::string& sService);
        virtual void Finished();
        virtual void RegistrationNodeError();
        virtual void InstanceRemoved(std::shared_ptr<dnsInstance> pInstance);
        virtual void Target(const std::string& sReceiverId, const std::string& sTransportFile, unsigned short nPort);
        virtual void PatchSender(const std::string& sSenderId, const connectionSender& conPatch, unsigned short nPort);
        virtual void PatchReceiver(const std::string& sReceiverId, const connectionReceiver& conPatch, unsigned short nPort);
        virtual void ActivateSender(const std::string& sSenderId);
        virtual void ActivateReceiver(const std::string& sReceiverId);
};

