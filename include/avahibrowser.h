#pragma once
#ifdef __GNU__
#include <avahi-client/client.h>
#include <avahi-client/lookup.h>
#include <avahi-common/thread-watch.h>
#include <avahi-common/malloc.h>
#include <avahi-common/error.h>
#include <set>
#include <map>
#include <list>
#include <mutex>
#include <memory>
#include "dlldefine.h"


struct dnsService;
struct dnsInstance;
class EventPoster;


static void client_callback(AvahiClient * pClient, AvahiClientState state, AVAHI_GCC_UNUSED void * userdata);
static void type_callback(AvahiServiceTypeBrowser* stb, AvahiIfIndex interface, AvahiProtocol protocol, AvahiBrowserEvent event, const char* type, const char* domain, AvahiLookupResultFlags flags, void* userdata);
static void resolve_callback(AvahiServiceResolver *r, AVAHI_GCC_UNUSED AvahiIfIndex interface, AVAHI_GCC_UNUSED AvahiProtocol protocol, AvahiResolverEvent event,const char *name, const char *type, const char *domain, const char *host_name, const AvahiAddress *address, uint16_t port, AvahiStringList *txt,AvahiLookupResultFlags flags,AVAHI_GCC_UNUSED void* userdata);
static void browse_callback(AvahiServiceBrowser *b, AvahiIfIndex interface, AvahiProtocol protocol, AvahiBrowserEvent event, const char *name, const char *type, const char *domain, AVAHI_GCC_UNUSED AvahiLookupResultFlags flags, void* userdata);
// CServiceBrowserDlg dialog
class ServiceBrowser
{
// Construction
    public:
        ServiceBrowser(std::shared_ptr<EventPoster> pPoster);
        virtual ~ServiceBrowser();


        bool StartBrowser(const std::set<std::string>& setServices);



        void ClientCallback(AvahiClient* pClient, AvahiClientState state);
        void TypeCallback(AvahiIfIndex interface, AvahiProtocol protocol, AvahiBrowserEvent event, const char* type, const char* domain);
        void BrowseCallback(AvahiServiceBrowser* pBrowser, AvahiIfIndex interface, AvahiProtocol protocol, AvahiBrowserEvent event, const char *name, const char *type, const char *domain);
        void ResolveCallback(AvahiServiceResolver* pResolver, AvahiResolverEvent event,const char *name, const char *type, const char *domain, const char *host_name, const AvahiAddress *address, uint16_t port, AvahiStringList *txt);

        void RemoveServiceInstance(const std::string& sService, const std::string& sInstance);

        std::map<std::string, std::shared_ptr<dnsService> >::const_iterator GetServiceBegin();
        std::map<std::string, std::shared_ptr<dnsService> >::const_iterator GetServiceEnd();
        std::map<std::string, std::shared_ptr<dnsService> >::const_iterator FindService(const std::string& sService);


    protected:

        void DeleteAllServices();
        bool Start(AvahiClient* pClient);
        void Stop();
        void CheckStop();

        std::mutex m_mutex;
//        void OnStop(wxCommandEvent& event);
        std::shared_ptr<EventPoster> m_pPoster;

        AvahiThreadedPoll* m_pThreadedPoll;
        AvahiClient * m_pClient;
        AvahiServiceTypeBrowser* m_pTypeBrowser;

        std::set<std::string> m_setServices;
        bool m_bBrowsing;
        unsigned long m_nWaitingOn;
        std::set<AvahiServiceBrowser*> m_setBrowser;
        std::map<std::string, std::shared_ptr<dnsService> > m_mServices;
};


#endif // __WXGNU__
