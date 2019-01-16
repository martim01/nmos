#pragma once
#include "eventposter.h"
#include <condition_variable>


class ThreadPoster : public EventPoster
{
    public:
        ThreadPoster(){}
        virtual ~ThreadPoster(){}

        bool Wait(std::chrono::milliseconds ms);
        unsigned int GetReason();

        const std::string& GetString();
        unsigned short GetPort();

        std::shared_ptr<Sender> GetSender();

        void Signal();
        enum {CURL_DONE, INSTANCE_RESOLVED, ALLFORNOW, FINISHED, REGERROR, INSTANCE_REMOVED, TARGET, PATCH_SENDER, PATCH_RECEIVER, ACTIVATE_SENDER, ACTIVATE_RECEIVER};

    protected:

        void CurlDone(unsigned long nResult, const std::string& sResponse, long nType);
        void InstanceResolved(std::shared_ptr<dnsInstance> pInstance);
        void AllForNow(const std::string& sService);
        void Finished();
        void RegistrationNodeError();
        void InstanceRemoved(const std::string& sInstance);
        void Target(const std::string& sReceiverId, std::shared_ptr<Sender> pSender, unsigned short nPort);
        void PatchSender(const std::string& sSenderId, const connectionSender& conPatch, unsigned short nPort);
        void PatchReceiver(const std::string& sReceiverId, const connectionReceiver& conPatch, unsigned short nPort);
        void ActivateSender(const std::string& sSenderId);
        void ActivateReceiver(const std::string& sReceiverId);

        void SetReason(unsigned int nReason);
        void LaunchThread();

        std::mutex m_mutexMain;
        std::condition_variable m_cv;

        std::string m_sString;
        unsigned long m_nLong;
        unsigned short m_nShort;

        unsigned int m_nReason;
        std::shared_ptr<Sender> m_pSender;

};

