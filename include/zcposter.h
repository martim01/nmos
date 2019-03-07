#pragma once
#include <string>
#include <thread>
#include <mutex>

struct dnsInstance;
class Sender;
class connectionSender;
class connectionReceiver;

class ZCPoster
{
    public:
        //EventPoster()=0;
        //virtual ~EventPoster()=0;

        void _RegistrationNodeError();
        void _InstanceResolved(std::shared_ptr<dnsInstance> pInstance);
        void _AllForNow(const std::string& sService);
        void _Finished();
        void _InstanceRemoved(std::shared_ptr<dnsInstance> pInstance);

    protected:


        /** @brief Called by AvahiBrowser when a dns service instance has been resolved
        *   @param pInstance a pointer to the resolved instance.
        *   @note The pointer must not be deleted as it belongs to AvahiBrowser
        *   @note this is a non-blocking event
        **/
        virtual void InstanceResolved(std::shared_ptr<dnsInstance> pInstance)=0;

        /** @brief Called by AvahiBrowser when no more instances of the given service are likely to be found
        *   @param sService the name of the service
        *   @note this is a non-blocking event
        **/
        virtual void AllForNow(const std::string& sService)=0;

        /** @brief Called by AvahiBrowser when it has definitely finished browsing
        *   @note this is a non-blocking event
        **/
        virtual void Finished()=0;

        /** @brief Called by AvahiBrowser when an error occurs
        *   @note this is a non-blocking event
        **/
        virtual void RegistrationNodeError()=0;

        /** @brief Called by AvahiBrowser when a dns service instance has been removed
        *   @param pInstance a pointer to the removed instance.
        *   @param sService the name of the service.
        *   @note this is a non-blocking event
        **/
        virtual void InstanceRemoved(std::shared_ptr<dnsInstance> pInstance)=0;


    private:
        std::mutex m_mutex;
};

