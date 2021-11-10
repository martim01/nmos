#pragma once
#include <string>
#include "dns_sd.h"
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <set>
#include <list>
#include "mdns.h"
#include <mutex>


struct dnsService;
struct dnsInstance;
class EventPoster;


static void DNSSD_API IterateServiceTypes( DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceIndex, DNSServiceErrorType errorCode, const char *serviceName, const char *regtype,const char *replyDomain, void *context );
static void DNSSD_API IterateServiceInstances( DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceIndex, DNSServiceErrorType errorCode, const char *serviceName, const char *regtype,const char *replyDomain, void *context );
static void DNSSD_API ResolveInstance(  DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceIndex, DNSServiceErrorType errorCode, const char *fullname, const char *hosttarget, uint16_t port, uint16_t txtLen, const unsigned char *txtRecord, void *context );
static void DNSSD_API GetAddress( DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceIndex, DNSServiceErrorType errorCode, const char *hostname, const struct sockaddr *address,uint32_t ttl, void *context );

namespace pml
{
    namespace nmos
    {
        class ServiceBrowser
        {

        public:
            ServiceBrowser(std::shared_ptr<EventPoster> pPoster);
            ~ServiceBrowser();
            bool StartBrowser(const std::set<std::string>& setServices);

            void DNSSD_API IterateTypes( DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceIndex, DNSServiceErrorType errorCode, const char *serviceName, const char *regtype,const char *replyDomain, void *context );
            void DNSSD_API IterateInstances( DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceIndex, DNSServiceErrorType errorCode, const char *serviceName, const char *regtype,const char *replyDomain, void *context );
            void DNSSD_API Resolve(  DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceIndex, DNSServiceErrorType errorCode, const char *fullname, const char *hosttarget, uint16_t port, uint16_t txtLen, const unsigned char *txtRecord, void *context );
            void DNSSD_API Address( DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceIndex, DNSServiceErrorType errorCode, const char *hostname, const struct sockaddr *address,uint32_t ttl, void *context );

            static void RunSelect(ServiceBrowser* pBrowser);

            std::map<std::string, std::shared_ptr<dnsService> >::const_iterator GetServiceBegin();
            std::map<std::string, std::shared_ptr<dnsService> >::const_iterator GetServiceEnd();
            std::map<std::string, std::shared_ptr<dnsService> >::const_iterator FindService(const std::string& sService);

            protected:
                std::shared_ptr<EventPoster> m_pPoster;

                std::map<std::string, std::shared_ptr<dnsService> > m_mServices;

                std::map<DNSServiceRef,int> m_mClientToFd;
                std::map<DNSServiceRef,std::string> m_mServRefToString;
                std::map<DNSServiceRef, std::shared_ptr<dnsInstance> > m_mInstances;
                std::unordered_set<std::string> m_setServiceTypes;
                std::unordered_set<std::string> m_setServiceInstances;


                std::set<std::string> m_setServices;

                std::mutex m_mutex;
        };
    };
};

