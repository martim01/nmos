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




ServiceBrowser& ServiceBrowser::Get()
{
    static ServiceBrowser sb;
    return sb;
}


ServiceBrowser::ServiceBrowser() :
    m_bFree(false),
    m_pThreadedPoll(0),
    m_pClient(0),
    m_pTypeBrowser(0),
    m_bStarted(false),
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
    m_mServices.clear();
}


bool ServiceBrowser::StartBrowser()
{
    lock_guard<mutex> lock(m_mutex);
    if(!m_bStarted)
    {
        int error;

        Log::Get(Log::LOG_DEBUG) << "ServiceBrowser: Create Threaded poll object." << endl;
        /* Allocate main loop object */
        if (!(m_pThreadedPoll = avahi_threaded_poll_new()))
        {
            Log::Get(Log::LOG_ERROR) << "ServiceBrowser: Failed to create Threaded poll object." << endl;
            return false;
        }

        /* Allocate a new client */
        Log::Get(Log::LOG_DEBUG) << "ServiceBrowser: Allocate a new client." << endl;
        avahi_client_new(avahi_threaded_poll_get(m_pThreadedPoll), AVAHI_CLIENT_NO_FAIL, client_callback, reinterpret_cast<void*>(this), &error);
        avahi_threaded_poll_start(m_pThreadedPoll);
        m_bStarted = true;
    }

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

        set<shared_ptr<EventPoster> > setPoster;
        for(map<string, shared_ptr<EventPoster> >::iterator itPoster = m_mServiceBrowse.begin(); itPoster != m_mServiceBrowse.end(); ++itPoster)
        {
            if(setPoster.insert(itPoster->second).second)
            {
                itPoster->second->_Finished();
            }
        }
    }
    m_nWaitingOn = 0;
}


bool ServiceBrowser::Start(AvahiClient* pClient)
{
    Log::Get(Log::LOG_DEBUG) << "ServiceBrowser:Start" << endl;
    if(!m_bBrowsing)
    {
        //lock_guard<mutex> lg(m_mutex);
        m_bBrowsing = true;
        m_pClient = pClient;
        Browse();
    }
    return true;
}

void ServiceBrowser::Browse()
{
    Log::Get(Log::LOG_DEBUG) << "ServiceBrowser:Browse" << endl;

    for(map<string, shared_ptr<EventPoster> >::iterator itService = m_mServiceBrowse.begin(); itService != m_mServiceBrowse.end(); ++itService)
    {
        if(m_mServices.insert(make_pair((itService->first), make_shared<dnsService>(dnsService((itService->first))))).second)
        {
            AvahiServiceBrowser* psb = NULL;
            /* Create the service browser */
            if (!(psb = avahi_service_browser_new(m_pClient, AVAHI_IF_UNSPEC, AVAHI_PROTO_INET, (itService->first).c_str(), NULL, (AvahiLookupFlags)0, browse_callback, reinterpret_cast<void*>(this))))
            {
                Log::Get(Log::LOG_ERROR) << "ServiceBrowser: Failed to create service browser" << endl;
            }
            else
            {
                Log::Get(Log::LOG_DEBUG) << "ServiceBrowser: Service '" << itService->first << "' browse" << endl;
                m_setBrowser.insert(psb);
                m_nWaitingOn++;
            }
        }
    }
}

void ServiceBrowser::ClientCallback(AvahiClient * pClient, AvahiClientState state)
{
    Log::Get(Log::LOG_DEBUG) << "ServiceBrowser:ClientCallback" << endl;
    switch (state)
    {
        case AVAHI_CLIENT_FAILURE:
            Log::Get(Log::LOG_ERROR) << "ServiceBrowser: ClientCallback: failure" << endl;
            if (avahi_client_errno(pClient) == AVAHI_ERR_DISCONNECTED)
            {
                int error;
                /* We have been disconnected, so let reconnect */
                Log::Get(Log::LOG_WARN) << "ServiceBrowser:  Disconnected, reconnecting ..." << endl;

                avahi_client_free(pClient);
                m_pClient = NULL;

                DeleteAllServices();

                if (!(avahi_client_new(avahi_threaded_poll_get(m_pThreadedPoll), AVAHI_CLIENT_NO_FAIL, client_callback, reinterpret_cast<void*>(this), &error)))
                {
                    Log::Get(Log::LOG_ERROR) << "ServiceBrowser: Failed to create client object: " << avahi_strerror(avahi_client_errno(m_pClient));// << endl;
                    Stop();

                }
            }
            else
            {
                Log::Get(Log::LOG_ERROR) << "ServiceBrowser: Server connection failure" << endl;
                Stop();
            }
            break;
        case AVAHI_CLIENT_S_REGISTERING:
        case AVAHI_CLIENT_S_RUNNING:
        case AVAHI_CLIENT_S_COLLISION:
            Log::Get(Log::LOG_DEBUG) << "ServiceBrowser: Registering/Running ..." << endl;
            if (!m_bBrowsing)
            {
                Start(pClient);
            }
            break;
        case AVAHI_CLIENT_CONNECTING:
            Log::Get(Log::LOG_DEBUG) << "ServiceBrowser: Waiting for daemon ..." << endl;
            break;
        default:
            Log::Get(Log::LOG_DEBUG) << "ServiceBrowser: ClientCallback: " << state << endl;
    }

}

void ServiceBrowser::TypeCallback(AvahiIfIndex interface, AvahiProtocol protocol, AvahiBrowserEvent event, const char* type, const char* domain)
{
    switch(event)
    {
        case AVAHI_BROWSER_NEW:
            {
                string sService(type);
                Log::Get(Log::LOG_DEBUG) << "ServiceBrowser: Service '" << type << "' found in domain '" << domain << "'" << endl;
                map<string, shared_ptr<EventPoster> >::iterator itServiceBrowse = m_mServiceBrowse.find(sService);
                if(itServiceBrowse != m_mServiceBrowse.end())
                {

                    m_mutex.lock();
                    if(m_mServices.insert(make_pair(sService, make_shared<dnsService>(dnsService(sService)))).second)
                    {
                        AvahiServiceBrowser* psb = NULL;
                        /* Create the service browser */
                        if (!(psb = avahi_service_browser_new(m_pClient, interface, protocol, type, domain, (AvahiLookupFlags)0, browse_callback, reinterpret_cast<void*>(this))))
                        {
                            Log::Get(Log::LOG_ERROR) << "ServiceBrowser: Failed to create service browser" << endl;
                        }
                        else
                        {
                            Log::Get(Log::LOG_DEBUG) << "ServiceBrowser: Service '" << type << "' browse" << endl;
                            m_setBrowser.insert(psb);
                            m_nWaitingOn++;
                        }
                    }
                    m_mutex.unlock();
                }
            }
            break;
         case AVAHI_BROWSER_REMOVE:
                /* We're dirty and never remove the browser again */
            Log::Get(Log::LOG_DEBUG) << "ServiceBrowser: Service '" << type << "' removed" << endl;
                break;
        case AVAHI_BROWSER_FAILURE:
            Log::Get(Log::LOG_ERROR) << "ServiceBrowser: service_type_browser failed" << endl;
            Stop();
            break;
        case AVAHI_BROWSER_CACHE_EXHAUSTED:
            Log::Get(Log::LOG_ERROR) << "ServiceBrowser: AVAHI_BROWSER_CACHE_EXHAUSTED" << endl;
            break;
        case AVAHI_BROWSER_ALL_FOR_NOW:
            Log::Get(Log::LOG_ERROR) << "ServiceBrowser: AVAHI_BROWSER_ALL_FOR_NOW" << endl;
            CheckStop();
            break;
        default:
            Log::Get(Log::LOG_DEBUG) << "ServiceBrowser: TypeCallback: " << event << endl;
    }
}

void ServiceBrowser::BrowseCallback(AvahiServiceBrowser* pBrowser, AvahiIfIndex interface, AvahiProtocol protocol, AvahiBrowserEvent event, const char *name, const char *type, const char *domain)
{
    /* Called whenever a new services becomes available on the LAN or is removed from the LAN */
    switch (event)
    {
        case AVAHI_BROWSER_FAILURE:
            Log::Get(Log::LOG_ERROR) << "ServiceBrowser: Browser Failure: " << avahi_strerror(avahi_client_errno(m_pClient)) << endl;
            Stop();
            break;
        case AVAHI_BROWSER_NEW:
            {
                string sService(type);
                string sName(name);

                Log::Get(Log::LOG_DEBUG) << "ServiceBrowser: (Browser) NEW: service '" << name << "' of type '" << type << "' in domain '" << domain << "'" << endl;
                AvahiServiceResolver* pResolver= avahi_service_resolver_new(m_pClient, interface, protocol, name, type, domain, AVAHI_PROTO_INET, (AvahiLookupFlags)0, resolve_callback, reinterpret_cast<void*>(this));
                if(!pResolver)
                {
                    Log::Get(Log::LOG_ERROR) << "ServiceBrowser: Failed to resolve service " << name << ": " << avahi_strerror(avahi_client_errno(m_pClient)) << endl;
                }
                else
                {
                    m_mResolvers.insert(make_pair(string(name)+"__"+string(type), pResolver));
                    m_nWaitingOn++;
                }
            }
            break;
        case AVAHI_BROWSER_REMOVE:
            Log::Get(Log::LOG_DEBUG) << "ServiceBrowser: (Browser) REMOVE: service '" << name << "' of type '" << type << "' in domain '" << domain << "'" << endl;
            RemoveServiceInstance(type, name);
            break;
        case AVAHI_BROWSER_ALL_FOR_NOW:
            {
                Log::Get(Log::LOG_DEBUG) << "ServiceBrowser: (Browser) '" << type << "' in domain '" << domain << "' ALL_FOR_NOW" << endl;
                shared_ptr<EventPoster> pPoster(GetPoster(type));
                if(pPoster)
                {
                    pPoster->_AllForNow(type);
                }

                if(m_bFree)
                {
                   set<AvahiServiceBrowser*>::iterator itBrowser = m_setBrowser.find(pBrowser);
                    if(itBrowser != m_setBrowser.end())
                    {
                        avahi_service_browser_free((*itBrowser));
                        m_setBrowser.erase(itBrowser);
                    }
                    CheckStop();
               }
            }
            break;
        case AVAHI_BROWSER_CACHE_EXHAUSTED:
            {
                Log::Get(Log::LOG_DEBUG) << "ServiceBrowser: (Browser) ' CACHE_EXHAUSTED " << endl;
            }
            break;
        default:
            Log::Get(Log::LOG_DEBUG) << "ServiceBrowser: BrowseCallback: " << event << endl;
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
                Log::Get(Log::LOG_ERROR) << "ServiceBrowser: (Resolver) Failed to resolve service '" << name << "' of type '" << type << "' in domain '" << domain << "': " << avahi_strerror(avahi_client_errno(m_pClient)) << endl;
                m_mResolvers.erase(string(name)+"__"+string(type));
                avahi_service_resolver_free(pResolver);
                break;
            case AVAHI_RESOLVER_FOUND:
            {
                char a[AVAHI_ADDRESS_STR_MAX];
                avahi_address_snprint(a, sizeof(a), address);
                m_mutex.lock();
                map<string, shared_ptr<dnsService> >::iterator itService = m_mServices.find(sService);
                if(itService != m_mServices.end())
                {
                    map<string, shared_ptr<dnsInstance> >::iterator itInstance = itService->second->mInstances.find(sName);
                    if(itInstance == itService->second->mInstances.end())
                    {
                        itInstance = itService->second->mInstances.insert(make_pair(sName, make_shared<dnsInstance>(dnsInstance(sName)))).first;
                    }
                    else
                    {
                        itInstance->second->nUpdate++;
                        itInstance->second->mTxtLast = itInstance->second->mTxt;
                    }

                    itInstance->second->sHostName = host_name;
                    itInstance->second->nPort = port;
                    itInstance->second->sHostIP = a;
                    itInstance->second->sService = sService;

                    for(AvahiStringList* pIterator = txt; pIterator; pIterator = avahi_string_list_get_next(pIterator))
                    {
                        string sPair(reinterpret_cast<char*>(avahi_string_list_get_text(pIterator)));
                        size_t nFind = sPair.find("=");
                        if(nFind != string::npos)
                        {
                            itInstance->second->mTxt[sPair.substr(0,nFind)] = sPair.substr(nFind+1);
                        }
                    }
                    shared_ptr<EventPoster> pPoster(GetPoster(sService));
                    if(pPoster)
                    {
                        pPoster->_InstanceResolved(itInstance->second);
                    }
                    m_mutex.unlock();
                    if(itInstance->second->nUpdate == 0)
                    {
                        Log::Get() << "ServiceBrowser: Instance '" << itInstance->second->sName << "' resolved at '" << itInstance->second->sHostIP << "'" << endl;
                    }
                    else
                    {
                        Log::Get() << "ServiceBrowser: Instance '" << itInstance->second->sName << "' updated at '" << itInstance->second->sHostIP << "'" << endl;
                    }
                }

            }
            break;
        }
    }

    if(m_bFree)
    {
        m_mResolvers.erase(string(name)+"__"+string(type));
        avahi_service_resolver_free(pResolver);
        CheckStop();
    }
}

void ServiceBrowser::RemoveServiceInstance(const std::string& sService, const std::string& sInstance)
{
    lock_guard<mutex> lock(m_mutex);

    map<string, AvahiServiceResolver*>::iterator itResolver = m_mResolvers.find((sInstance+"__"+sService));
    if(itResolver != m_mResolvers.end())
    {
        avahi_service_resolver_free(itResolver->second);
        m_mResolvers.erase(itResolver);
    }


    map<string, shared_ptr<dnsService> >::iterator itService = m_mServices.find(sService);
    if(itService != m_mServices.end())
    {
        shared_ptr<EventPoster> pPoster(GetPoster(sService));

        map<string, shared_ptr<dnsInstance> >::iterator itInstance = itService->second->mInstances.find(sInstance);
        if(itInstance != itService->second->mInstances.end())
        {
            if(pPoster)
            {
                pPoster->_InstanceRemoved(itInstance->second);
            }
            itService->second->mInstances.erase(sInstance);
        }
    }
}


void ServiceBrowser::CheckStop()
{
    --m_nWaitingOn;
    if(m_nWaitingOn == 0)
    {
        //NodeApi::Get().SignalBrowse();

    }
}

map<string, shared_ptr<dnsService> >::const_iterator ServiceBrowser::GetServiceBegin()
{
    lock_guard<mutex> lock(m_mutex);
    return m_mServices.begin();
}

map<string, shared_ptr<dnsService> >::const_iterator ServiceBrowser::GetServiceEnd()
{
    lock_guard<mutex> lock(m_mutex);
    return m_mServices.end();
}

map<string, shared_ptr<dnsService> >::const_iterator ServiceBrowser::FindService(const string& sService)
{
    lock_guard<mutex> lock(m_mutex);
    return m_mServices.find(sService);
}

std::shared_ptr<EventPoster> ServiceBrowser::GetPoster(const std::string& sService)
{
    map<string, shared_ptr<EventPoster> >::iterator itPoster = m_mServiceBrowse.find(sService);
    if(itPoster != m_mServiceBrowse.end())
    {
        return itPoster->second;
    }
    return 0;
}


void ServiceBrowser::AddService(const std::string& sService, std::shared_ptr<EventPoster> pPoster)
{
    lock_guard<mutex> lock(m_mutex);
    m_mServiceBrowse.insert(make_pair(sService, pPoster));
    if(m_bBrowsing)
    {
        Browse();
    }
}

void ServiceBrowser::RemoveService(const std::string& sService)
{
    //@todo RemoveService
}
