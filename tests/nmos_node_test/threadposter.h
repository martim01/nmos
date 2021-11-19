#pragma once
#include "eventposter.h"
#include <condition_variable>

namespace pml
{
    namespace nmos
    {
        class ApiVersion;
    };
};

class ThreadPoster : public pml::nmos::EventPoster
{
    public:
        ThreadPoster(){}
        virtual ~ThreadPoster(){}

        bool Wait(std::chrono::milliseconds ms);
        unsigned int GetReason();

        const std::string& GetString();
        unsigned short GetPort();

        const std::string GetSDP() const
        {
            return m_sSDP;
        }

        void Signal();
        enum {CURL_DONE, INSTANCE_RESOLVED, ALLFORNOW, FINISHED, REGERROR, INSTANCE_REMOVED, TARGET, PATCH_SENDER, PATCH_RECEIVER, ACTIVATE_SENDER, ACTIVATE_RECEIVER,
              REGISTRATION_NODE_FOUND, REGISTRATION_NODE_REMOVED, REGISTRATION_NODE_CHANGED, REGISTRATION_NODE_CHOSEN, REGISTRATION_CHANGED};

    protected:

        void Target(const std::string& sReceiverId, const std::string& sTransportFile, unsigned short nPort) override;
        void PatchSender(const std::string& sSenderId, const pml::nmos::connectionSender& conPatch, unsigned short nPort) override;
        void PatchReceiver(const std::string& sReceiverId, const pml::nmos::connectionReceiver& conPatch, unsigned short nPort) override;
        void SenderActivated(const std::string& sSenderId) override;
        void ReceiverActivated(const std::string& sReceiverId) override;

        void RegistrationNodeFound(const std::string& sUrl, unsigned short nPriority, const pml::nmos::ApiVersion& version) override;
        void RegistrationNodeRemoved(const std::string& sUrl) override;
        void RegistrationNodeChanged(const std::string& sUrl, unsigned short nPriority, bool bGood, const pml::nmos::ApiVersion& version) override;
        void RegistrationChanged(const std::string& sUrl, enumRegState eState) override;


        void SetReason(unsigned int nReason);
        void LaunchThread();

        std::mutex m_mutexMain;
        std::condition_variable m_cv;

        std::string m_sString;
        unsigned long m_nLong;
        unsigned short m_nShort;

        std::string m_sSDP;

        unsigned int m_nReason;
        std::shared_ptr<pml::nmos::Sender> m_pSender;

};

