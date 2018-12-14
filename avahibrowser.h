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


struct dnsInstance
{
    dnsInstance(){}
    dnsInstance(std::string sN) : sName(sN){}

    std::string sName;
    std::string sHostName;
    std::string sHostIP;
    std::string sService;
    unsigned long nPort;
    std::string sInterface;
    std::map<std::string, std::string> mTxt;

};


struct dnsService
{
    dnsService(){}
    dnsService(std::string ss) : sService(ss){}

    ~dnsService()
    {
        for(std::list<dnsInstance*>::iterator itInstance = lstInstances.begin(); itInstance != lstInstances.end(); ++itInstance)
        {
            delete (*itInstance);
        }
    }

    std::string sService;
    std::list<dnsInstance*> lstInstances;

};




static void client_callback(AvahiClient * pClient, AvahiClientState state, AVAHI_GCC_UNUSED void * userdata);
static void type_callback(AvahiServiceTypeBrowser* stb, AvahiIfIndex interface, AvahiProtocol protocol, AvahiBrowserEvent event, const char* type, const char* domain, AvahiLookupResultFlags flags, void* userdata);
static void resolve_callback(AvahiServiceResolver *r, AVAHI_GCC_UNUSED AvahiIfIndex interface, AVAHI_GCC_UNUSED AvahiProtocol protocol, AvahiResolverEvent event,const char *name, const char *type, const char *domain, const char *host_name, const AvahiAddress *address, uint16_t port, AvahiStringList *txt,AvahiLookupResultFlags flags,AVAHI_GCC_UNUSED void* userdata);
static void browse_callback(AvahiServiceBrowser *b, AvahiIfIndex interface, AvahiProtocol protocol, AvahiBrowserEvent event, const char *name, const char *type, const char *domain, AVAHI_GCC_UNUSED AvahiLookupResultFlags flags, void* userdata);
// CServiceBrowserDlg dialog
class ServiceBrowser
{
// Construction
    public:
        ServiceBrowser();
        ~ServiceBrowser();


        bool StartBrowser(const std::set<std::string>& setServices);

        void ClientCallback(AvahiClient* pClient, AvahiClientState state);
        void TypeCallback(AvahiIfIndex interface, AvahiProtocol protocol, AvahiBrowserEvent event, const char* type, const char* domain);
        void BrowseCallback(AvahiServiceBrowser* pBrowser, AvahiIfIndex interface, AvahiProtocol protocol, AvahiBrowserEvent event, const char *name, const char *type, const char *domain);
        void ResolveCallback(AvahiServiceResolver* pResolver, AvahiResolverEvent event,const char *name, const char *type, const char *domain, const char *host_name, const AvahiAddress *address, uint16_t port, AvahiStringList *txt);


    protected:

        void DeleteAllServices();
        bool Start(AvahiClient* pClient);
        void Stop();
        void CheckStop();
//        void OnStop(wxCommandEvent& event);

        AvahiThreadedPoll* m_pThreadedPoll;
        AvahiClient * m_pClient;
        AvahiServiceTypeBrowser* m_pTypeBrowser;

        std::set<std::string> m_setServices;
        bool m_bBrowsing;
        unsigned long m_nWaitingOn;
        std::set<AvahiServiceBrowser*> m_setBrowser;
        std::map<std::string, dnsService*> m_mServices;
};


#endif // __WXGNU__
