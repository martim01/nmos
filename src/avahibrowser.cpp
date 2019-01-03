#ifdef __GNU__
#include "avahibrowser.h"
#include <iostream>
#include "log.h"
#include "eventposter.h"
#include "mdns.h"
#include <mutex>
#include "nodeapi.h"


using namespace std;

void client_callback(AvahiClient * pClient, AvahiClientState state, AVAHI_GCC_UNUSED void * userdata)
{
    ServiceBrowser* pBrowser = reinterpret_cast<ServiceBrowser*>(userdata);
    pBrowser->ClientCallback(pClient, state);
}

void type_callback(AvahiServiceTypeBrowser* stb, AvahiIfIndex interface, AvahiProtocol protocol, AvahiBrowserEvent event, const char* type, const char* domain, AvahiLookupResultFlags flags, void* userdata)
{
    ServiceBrowser* pBrowser = reinterpret_cast<ServiceBrowser*>(userdata);
    pBrowser->TypeCallback(interface, protocol, event, type, domain);
}

void browse_callback(AvahiServiceBrowser *b, AvahiIfIndex interface, AvahiProtocol protocol, AvahiBrowserEvent event, const char *name, const char *type, const char *domain, AVAHI_GCC_UNUSED AvahiLookupResultFlags flags, void* userdata)
{
    ServiceBrowser* pBrowser = reinterpret_cast<ServiceBrowser*>(userdata);
    pBrowser->BrowseCallback(b, interface, protocol, event, name, type, domain);
}

void resolve_callback(AvahiServiceResolver *r, AVAHI_GCC_UNUSED AvahiIfIndex interface, AVAHI_GCC_UNUSED AvahiProtocol protocol, AvahiResolverEvent event,const char *name, const char *type, const char *domain, const char *host_name, const AvahiAddress *address, uint16_t port, AvahiStringList *txt,AvahiLookupResultFlags flags,AVAHI_GCC_UNUSED void* userdata)
{
    ServiceBrowser* pBrowser = reinterpret_cast<ServiceBrowser*>(userdata);
    pBrowser->ResolveCallback(r, event, name, type, domain,host_name, address,port,txt);

}







ServiceBrowser::ServiceBrowser(std::shared_ptr<EventPoster> pPoster) :
    m_pPoster(pPoster),
    m_pThreadedPoll(0),
    m_pClient(0),
    m_pTypeBrowser(0),
    m_bBrowsing(false)

{

}

ServiceBrowser::~ServiceBrowser()
{
    Stop();
    DeleteAllServices();

}

void ServiceBrowser::DeleteAllServices()
{
    lock_guard<mutex> lock(m_mutex);
    for(map<string, dnsService*>::iterator itService = m_mServices.begin(); itService != m_mServices.end(); ++itService)
    {
        delete itService->second;
    }
    m_mServices.clear();
}


bool ServiceBrowser::StartBrowser(const set<string>& setServices)
{
    m_setServices = setServices;
    int error;

    /* Allocate main loop object */
    if (!(m_pThreadedPoll = avahi_threaded_poll_new()))
    {
        Log::Get(Log::ERROR) << "ServiceBrowser: Failed to create Threaded poll object." << endl;
        return false;
    }

    /* Allocate a new client */
    avahi_client_new(avahi_threaded_poll_get(m_pThreadedPoll), AVAHI_CLIENT_NO_FAIL, client_callback, reinterpret_cast<void*>(this), &error);
    avahi_threaded_poll_start(m_pThreadedPoll);

    Log::Get(Log::DEBUG) << "ServiceBrowser: Started" << endl;
    return true;
}

void ServiceBrowser::Stop()
{
    if(m_pThreadedPoll)
    {
        avahi_threaded_poll_stop(m_pThreadedPoll);
    }

    for(set<AvahiServiceBrowser*>::iterator itBrowser = m_setBrowser.begin(); itBrowser != m_setBrowser.end(); ++itBrowser)
    {
        avahi_service_browser_free((*itBrowser));
    }

    if(m_pTypeBrowser)
    {
        avahi_service_type_browser_free(m_pTypeBrowser);
        m_pTypeBrowser = 0;
    }
    if(m_pClient)
    {
        avahi_client_free(m_pClient);
        m_pClient = 0;
    }
    if(m_pThreadedPoll)
    {
        avahi_threaded_poll_free(m_pThreadedPoll);
        m_pThreadedPoll = 0;
        if(m_pPoster)
        {
            m_pPoster->_Finished();
        }
//        if(m_pHandler)
//        {   //only send if we are actually stopping not already stopped and now deleting
//            wxCommandEvent event(wxEVT_BROWSE_FINISHED);
//            wxPostEvent(m_pHandler, event);
//        }
    }
    m_nWaitingOn = 0;
}


bool ServiceBrowser::Start(AvahiClient* pClient)
{
    if(!m_bBrowsing)
    {
        m_pClient = pClient;
        if(!(m_pTypeBrowser = avahi_service_type_browser_new(pClient, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, NULL, (AvahiLookupFlags)0, type_callback, reinterpret_cast<void*>(this))))
        {
            Log::Get(Log::ERROR) << "Failed to create service type browser" << endl;
            return false;
        }
        m_nWaitingOn = 1;

    }
    return true;
}

void ServiceBrowser::ClientCallback(AvahiClient * pClient, AvahiClientState state)
{
    Log::Get(Log::DEBUG) << "ServiceBrowser: ClientCallback" << endl;
    switch (state)
    {
        case AVAHI_CLIENT_FAILURE:
            Log::Get(Log::ERROR) << "ServiceBrowser: ClientCallback: failure" << endl;
            if (avahi_client_errno(pClient) == AVAHI_ERR_DISCONNECTED)
            {
                int error;
                /* We have been disconnected, so let reconnect */
                Log::Get(Log::WARN) << "ServiceBrowser:  Disconnected, reconnecting ..." << endl;

                avahi_client_free(pClient);
                m_pClient = NULL;

                DeleteAllServices();

                if (!(avahi_client_new(avahi_threaded_poll_get(m_pThreadedPoll), AVAHI_CLIENT_NO_FAIL, client_callback, reinterpret_cast<void*>(this), &error)))
                {
                    Log::Get(Log::ERROR) << "ServiceBrowser: Failed to create client object: " << avahi_strerror(avahi_client_errno(m_pClient));// << endl;
                    Stop();

                }
            }
            else
            {
                Log::Get(Log::ERROR) << "ServiceBrowser: Server connection failure" << endl;
                Stop();
            }
            break;
        case AVAHI_CLIENT_S_REGISTERING:
        case AVAHI_CLIENT_S_RUNNING:
        case AVAHI_CLIENT_S_COLLISION:
            Log::Get(Log::DEBUG) << "ServiceBrowser: S_" << endl;
            if (!m_bBrowsing)
            {
                Start(pClient);
            }
            break;
        case AVAHI_CLIENT_CONNECTING:
            Log::Get(Log::DEBUG) << "ServiceBrowser: Waiting for daemon ..." << endl;
            break;
    }

}

void ServiceBrowser::TypeCallback(AvahiIfIndex interface, AvahiProtocol protocol, AvahiBrowserEvent event, const char* type, const char* domain)
{
    switch(event)
    {
        case AVAHI_BROWSER_NEW:
            {
                string sService(type);

                if(m_setServices.find(sService) != m_setServices.end())
                {
                    Log::Get(Log::DEBUG) << "ServiceBrowser: Service '" << type << "' found in domain '" << domain << "'" << endl;
                    m_mutex.lock();
                    if(m_mServices.insert(make_pair(sService, new dnsService(sService))).second)
                    {
                        AvahiServiceBrowser* psb = NULL;
                        /* Create the service browser */
                        if (!(psb = avahi_service_browser_new(m_pClient, interface, protocol, type, domain, (AvahiLookupFlags)0, browse_callback, reinterpret_cast<void*>(this))))
                        {
                            Log::Get(Log::ERROR) << "ServiceBrowser: Failed to create service browser" << endl;
                        }
                        else
                        {
                            Log::Get(Log::DEBUG) << "ServiceBrowser: Service '" << type << "' browse" << endl;
                            m_setBrowser.insert(psb);
                            m_nWaitingOn++;
                        }
                    }
                    m_mutex.unlock();
                }
                else
                {
                    Log::Get(Log::DEBUG) << "ServiceBrowser: Service '" << type << "' not for us" << endl;
                }
            }
            break;
         case AVAHI_BROWSER_REMOVE:
                /* We're dirty and never remove the browser again */
            Log::Get(Log::DEBUG) << "ServiceBrowser: Service '" << type << "' removed" << endl;
                break;
        case AVAHI_BROWSER_FAILURE:
            Log::Get(Log::ERROR) << "ServiceBrowser: service_type_browser failed" << endl;
            Stop();
            break;
        case AVAHI_BROWSER_CACHE_EXHAUSTED:
            Log::Get(Log::DEBUG) << "ServiceBrowser: (TypeBrowser) CACHE_EXHAUSTED" << endl;
            break;
        case AVAHI_BROWSER_ALL_FOR_NOW:
            Log::Get(Log::DEBUG) << "ServiceBrowser: (TypeBrowser) ALL_FOR_NOW" << endl;
            CheckStop();
            break;
    }
}

void ServiceBrowser::BrowseCallback(AvahiServiceBrowser* pBrowser, AvahiIfIndex interface, AvahiProtocol protocol, AvahiBrowserEvent event, const char *name, const char *type, const char *domain)
{
    /* Called whenever a new services becomes available on the LAN or is removed from the LAN */
    switch (event)
    {
        case AVAHI_BROWSER_FAILURE:
            Log::Get(Log::ERROR) << "ServiceBrowser: Browser Failure: " << avahi_strerror(avahi_client_errno(m_pClient)) << endl;
            Stop();
            break;
        case AVAHI_BROWSER_NEW:
            {
                string sService(type);
                string sName(name);

                Log::Get(Log::DEBUG) << "ServiceBrowser: (Browser) NEW: service '" << name << "' of type '" << type << "' in domain '" << domain << "'" << endl;

                /* We ignore the returned resolver object. In the callback
                   function we free it. If the server is terminated before
                   the callback function is called the server will free
                   the resolver for us. */
                if (!(avahi_service_resolver_new(m_pClient, interface, protocol, name, type, domain, AVAHI_PROTO_UNSPEC, (AvahiLookupFlags)0, resolve_callback, reinterpret_cast<void*>(this))))
                {
                    Log::Get(Log::ERROR) << "ServiceBrowser: Failed to resolve service " << name << ": " << avahi_strerror(avahi_client_errno(m_pClient)) << endl;
                }
                else
                {
                    m_nWaitingOn++;
                }
            }
            break;
        case AVAHI_BROWSER_REMOVE:
            Log::Get(Log::DEBUG) << "ServiceBrowser: (Browser) REMOVE: service '" << name << "' of type '" << type << "' in domain '" << domain << "'" << endl;
            RemoveServiceInstance(type, name);
            break;
        case AVAHI_BROWSER_ALL_FOR_NOW:
            {
                Log::Get(Log::DEBUG) << "ServiceBrowser: (Browser) '" << type << "' in domain '" << domain << "' ALL_FOR_NOW" << endl;
                if(m_pPoster)
                {
                    m_pPoster->_AllForNow(type);
                }


               set<AvahiServiceBrowser*>::iterator itBrowser = m_setBrowser.find(pBrowser);
                if(itBrowser != m_setBrowser.end())
                {
                    avahi_service_browser_free((*itBrowser));
                    m_setBrowser.erase(itBrowser);
                }
                CheckStop();
            }
            break;
        case AVAHI_BROWSER_CACHE_EXHAUSTED:
            {
                Log::Get(Log::DEBUG) << "ServiceBrowser: (Browser) CACHE_EXHAUSTED" << endl;
            }
            break;
    }
}

void ServiceBrowser::ResolveCallback(AvahiServiceResolver* pResolver, AvahiResolverEvent event,const char *name, const char *type, const char *domain, const char *host_name, const AvahiAddress *address, uint16_t port, AvahiStringList *txt)
{
    if(pResolver)
    {
        //AvahiLookupResultFlags flags;
        string sName(name);
        string sService(type);
        string sDomain(domain);
        /* Called whenever a service has been resolved successfully or timed out */
        switch (event)
        {
            case AVAHI_RESOLVER_FAILURE:
                Log::Get(Log::ERROR) << "ServiceBrowser: (Resolver) Failed to resolve service '" << name << "' of type '" << type << "' in domain '" << domain << "': " << avahi_strerror(avahi_client_errno(m_pClient)) << endl;
                break;
            case AVAHI_RESOLVER_FOUND:
            {
                char a[AVAHI_ADDRESS_STR_MAX];
                avahi_address_snprint(a, sizeof(a), address);
                m_mutex.lock();
                map<string, dnsService*>::iterator itService = m_mServices.find(sService);
                dnsInstance* pInstance = new dnsInstance(sName);
                pInstance->sHostName = host_name;
                pInstance->nPort = port;
                pInstance->sHostIP = a;
                pInstance->sService = sService;

                for(AvahiStringList* pIterator = txt; pIterator; pIterator = avahi_string_list_get_next(pIterator))
                {
                    string sPair(reinterpret_cast<char*>(avahi_string_list_get_text(pIterator)));
                    size_t nFind = sPair.find("=");
                    if(nFind != string::npos)
                    {
                        pInstance->mTxt.insert(make_pair(sPair.substr(0,nFind), sPair.substr(nFind+1)));
                    }
                }
                itService->second->lstInstances.push_back(pInstance);
                if(m_pPoster)
                {
                    m_pPoster->_InstanceResolved(pInstance);
                }
                m_mutex.unlock();

                Log::Get() << "ServiceBrowser: Instance '" << pInstance->sName << "' resolved at '" << pInstance->sHostIP << "'" << endl;

            }
        }
    }
    avahi_service_resolver_free(pResolver);
    CheckStop();
}

void ServiceBrowser::RemoveServiceInstance(const std::string& sService, const std::string& sInstance)
{
    lock_guard<mutex> lock(m_mutex);

    map<string, dnsService*>::iterator itService = m_mServices.find(sService);
    if(itService != m_mServices.end())
    {
        for(list<dnsInstance*>::iterator itInstance = itService->second->lstInstances.begin(); itInstance != itService->second->lstInstances.end(); )
        {
            if((*itInstance)->sName == sInstance)
            {
                delete (*itInstance);
                list<dnsInstance*>::iterator itDelete(itInstance);
                ++itInstance;
                itService->second->lstInstances.erase(itDelete);
                if(m_pPoster)
                {
                    m_pPoster->_InstanceRemoved(sInstance);
                }
            }
            else
            {
                ++itInstance;
            }
        }
    }
}


void ServiceBrowser::CheckStop()
{
    --m_nWaitingOn;
    if(m_nWaitingOn == 0)
    {
        NodeApi::Get().SignalBrowse();
    }
}

map<string, dnsService*>::const_iterator ServiceBrowser::GetServiceBegin()
{
    lock_guard<mutex> lock(m_mutex);
    return m_mServices.begin();
}

map<string, dnsService*>::const_iterator ServiceBrowser::GetServiceEnd()
{
    lock_guard<mutex> lock(m_mutex);
    return m_mServices.end();
}

map<string, dnsService*>::const_iterator ServiceBrowser::FindService(const string& sService)
{
    lock_guard<mutex> lock(m_mutex);
    return m_mServices.find(sService);
}

#endif // __WXGTK
