#include "nodeapi.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#ifdef __GNU__
#include "avahipublisher.h"
#include "avahibrowser.h"
#include <unistd.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#else
#define WINVER 0x0600
#include "bonjourbrowser.h"
#include "bonjourpublisher.h"
#include <winsock2.h>
#include <windows.h>
#include <iphlpapi.h>
#include <iptypes.h>
#endif

#include "log.h"
#include "curlregister.h"
#include "eventposter.h"
#include <thread>
#include <chrono>
#include "mdns.h"
#include "device.h"
#include "source.h"
#include "flow.h"
#include "receiver.h"
#include "sender.h"
#include "is04server.h"
#include "is05server.h"
#include "nmosthread.h"
#include "sdp.h"
#include "utils.h"

using namespace std;

const std::string NodeApi::STR_RESOURCE[7] = {"node", "device", "source", "flow", "sender", "receiver", "subscription"};



NodeApi& NodeApi::Get()
{
    static NodeApi aNode;
    return aNode;
}

NodeApi::NodeApi() :
m_devices("device"),
m_senders("sender"),
m_receivers("receiver"),
m_sources("source"),
m_flows("flow"),
m_pNodeApiPublisher(0),
m_pRegisterCurl(0),
m_nRegistrationStatus(REG_START),
m_bRun(true),
m_bBrowsing(false),
m_pPoster(0),
m_nConnectionPort(0),
m_nHeartbeatTime(5000)
{
}

NodeApi::~NodeApi()
{
    StopHttpServers();
    StopmDNSServer();
    StopRegistrationBrowser();
    if(m_pRegisterCurl)
    {
        delete m_pRegisterCurl;
    }

}



void NodeApi::Init(unsigned short nDiscoveryPort, unsigned short nConnectionPort, const string& sLabel, const string& sDescription)
{
    char sHost[256];
    gethostname(sHost, 256);

    set<string> setEndpoints;
    #ifdef __GNU__
    struct ifaddrs *interfaces = NULL;
    struct ifaddrs *temp_addr = NULL;
    int success = getifaddrs(&interfaces);
    if (success == 0)
    {
        // Loop through linked list of interfaces
        temp_addr = interfaces;
        while(temp_addr != NULL)
        {
            if(temp_addr->ifa_addr->sa_family == AF_INET)
            {
                Log::Get(Log::LOG_DEBUG) << "Interface: " << temp_addr->ifa_name << endl;
                string sInterface(temp_addr->ifa_name);
                string sAddress(inet_ntoa(((sockaddr_in*)temp_addr->ifa_addr)->sin_addr));
                if(sAddress != "127.0.0.1")
                {
                    setEndpoints.insert(sAddress);
                    m_self.AddEndpoint(sAddress, nDiscoveryPort, false);
                }
                if(sInterface == "eth0" && Resource::s_sBase.empty())
                {
                    nodeinterface anInterface;
                    Self::GetAddresses(sInterface, anInterface);
                    Resource::s_sBase = anInterface.sPortMac;
                    Log::Get(Log::LOG_DEBUG) << "Base: " << Resource::s_sBase << endl;
                }
            }
            temp_addr = temp_addr->ifa_next;
        }
    }
    // Free memory
    freeifaddrs(interfaces);
    #else
    DWORD dwRetVal = 0;
    // Set the flags to pass to GetAdaptersAddresses
    ULONG flags = 0;//GAA_FLAG_INCLUDE_PREFIX;
    // default to unspecified address family (both)
    ULONG family = AF_INET;
    PIP_ADAPTER_ADDRESSES pAddresses = NULL;
    ULONG outBufLen = 15000;
    ULONG Iterations = 0;
    PIP_ADAPTER_ADDRESSES pCurrAddresses = NULL;
    PIP_ADAPTER_UNICAST_ADDRESS pUnicast = NULL;
    // Allocate a 15 KB buffer to start with.
    do
    {
        pAddresses = (IP_ADAPTER_ADDRESSES *) malloc(outBufLen);
        if (pAddresses == NULL)
        {
            Log::Get() << "Memory allocation failed for IP_ADAPTER_ADDRESSES struct" << endl;
            return;
        }
        dwRetVal = GetAdaptersAddresses(family, flags, NULL, pAddresses, &outBufLen);
        if (dwRetVal == ERROR_BUFFER_OVERFLOW)
        {
            free(pAddresses);
            pAddresses = NULL;
        }
        else
        {
            break;
        }
        Iterations++;
    } while ((dwRetVal == ERROR_BUFFER_OVERFLOW) && (Iterations < 3));

    if (dwRetVal == NO_ERROR)
    {
        // If successful, output some information from the data we received
        pCurrAddresses = pAddresses;
        while (pCurrAddresses)
        {
            pUnicast = pCurrAddresses->FirstUnicastAddress;
            if (pUnicast != NULL)
            {
                string sAddress = inet_ntoa(((sockaddr_in*)pUnicast->Address.lpSockaddr)->sin_addr);
                if(sAddress.substr(0,3) != "169")
                {
                    m_self.AddEndpoint(sAddress, nDiscoveryPort, false);
                }
                for (int i = 0; pUnicast != NULL; i++)
                    pUnicast = pUnicast->Next;
            }
            pCurrAddresses = pCurrAddresses->Next;
        }
    }
    else
    {
        Log::Get() << "Call to GetAdaptersAddresses failed with error: " << dwRetVal << endl;
    }
    if (pAddresses)
    {
        free(pAddresses);
    }
    #endif // __GNU__

    if(setEndpoints.empty() == false)
    {
        stringstream sstr;
        sstr << "http://" << *setEndpoints.begin() << ":" << nDiscoveryPort;


        m_self.Init(sHost, sstr.str(), sLabel, sDescription);
    }

    IS04Server* pServer = new IS04Server(m_pPoster);
    m_mServers.insert(make_pair(nDiscoveryPort, pServer));

    //start the connection servers
    IS05Server* p05Server = new IS05Server(m_pPoster);
    m_mServers.insert(make_pair(nConnectionPort, p05Server));
    m_nConnectionPort = nConnectionPort;

}

bool NodeApi::StartHttpServers()
{
    Log::Get(Log::LOG_DEBUG) << "Start Http Servers" << endl;

    for(map<unsigned short, MicroServer*>::iterator itServer = m_mServers.begin(); itServer != m_mServers.end(); ++itServer)
    {
        itServer->second->Init(itServer->first);

    }


    Log::Get(Log::LOG_DEBUG) << "Start Http Servers: Done" << endl;
    return true;

}

void NodeApi::StopHttpServers()
{
    for(map<unsigned short, MicroServer*>::iterator itServer = m_mServers.begin(); itServer != m_mServers.end(); ++itServer)
    {
        itServer->second->Stop();
        delete itServer->second;
    }
    m_mServers.clear();
}

bool NodeApi::StartmDNSServer()
{
    StopmDNSServer();
    set<endpoint>::const_iterator itEndpoint = m_self.GetEndpointsBegin();
    if(itEndpoint != m_self.GetEndpointsEnd())
    {
        Log::Get() << "Start mDNS Publisher" << endl;

        m_pNodeApiPublisher = new ServicePublisher(CreateGuid(itEndpoint->sHost), "_nmos-node._tcp", itEndpoint->nPort, itEndpoint->sHost);
        SetmDNSTxt(itEndpoint->bSecure);
        return m_pNodeApiPublisher->Start();
    }
    return false;

}

void NodeApi::StopmDNSServer()
{
    if(m_pNodeApiPublisher)
    {
        Log::Get() << "Stop mDNS Publisher" << endl;
        m_pNodeApiPublisher->Stop();
        delete m_pNodeApiPublisher;
        m_pNodeApiPublisher = 0;
    }
}


bool NodeApi::StartServices(shared_ptr<EventPoster> pPoster)
{

    m_pPoster = pPoster;
    m_pRegisterCurl = new CurlRegister(m_pPoster);

    thread thMain(NodeThread::Main);
    thMain.detach();

    return true;
}

void NodeApi::StopServices()
{
    StopRun();
    StopHttpServers();
    StopmDNSServer();
    StopRegistrationBrowser();
}



Self& NodeApi::GetSelf()
{
    return m_self;
}

const ResourceHolder<Source>& NodeApi::GetSources()
{
    return m_sources;
}

const ResourceHolder<Device>& NodeApi::GetDevices()
{
    return m_devices;
}

const ResourceHolder<Flow>& NodeApi::GetFlows()
{
    return m_flows;
}

const ResourceHolder<Receiver>& NodeApi::GetReceivers()
{
    return m_receivers;
}

const ResourceHolder<Sender>& NodeApi::GetSenders()
{
    return m_senders;
}

bool NodeApi::Commit()
{
    m_mutex.lock();
    Log::Get() << "Node: Commit" << endl;
    bool bChange = m_self.Commit();
    bChange |= m_sources.Commit(m_self.GetApiVersions());
    bChange |= m_devices.Commit(m_self.GetApiVersions());
    bChange |= m_flows.Commit(m_self.GetApiVersions());
    bChange |= m_receivers.Commit(m_self.GetApiVersions());
    bChange |= m_senders.Commit(m_self.GetApiVersions());
    m_mutex.unlock();


    if(bChange)
    {
        if(m_sRegistrationNode.empty())
        {   //update the ver_ text records in peer-to-peer mode
            ModifyTxtRecords();
        }
        else
        {
            //signal the register thread that we need to post resources
            Signal(SIG_COMMIT);
        }
    }

    return bChange;
}


void NodeApi::ModifyTxtRecords()
{
    set<endpoint>::const_iterator itEndpoint = m_self.GetEndpointsBegin();
    if(itEndpoint != m_self.GetEndpointsEnd())
    {
        SetmDNSTxt(itEndpoint->bSecure);
    }
//    if(m_pNodeApiPublisher)
//    {
//        m_pNodeApiPublisher->Modify();
//    }
}


void NodeApi::SetmDNSTxt(bool bSecure)
{
    lock_guard<mutex> guard(m_mutex);
    if(m_pNodeApiPublisher)
    {
        if(bSecure)
        {
            m_pNodeApiPublisher->AddTxt("api_proto", "https", false);
        }
        else
        {
            m_pNodeApiPublisher->AddTxt("api_proto", "http", false);
        }
        m_pNodeApiPublisher->AddTxt("api_ver", "v1.2", false);

        if(m_sRegistrationNode.empty())
        {
            m_pNodeApiPublisher->AddTxt("ver_slf", to_string(m_self.GetDnsVersion()),false);
            m_pNodeApiPublisher->AddTxt("ver_src", to_string(m_sources.GetVersion()),false);
            m_pNodeApiPublisher->AddTxt("ver_flw", to_string(m_flows.GetVersion()),false);
            m_pNodeApiPublisher->AddTxt("ver_dvc", to_string(m_devices.GetVersion()),false);
            m_pNodeApiPublisher->AddTxt("ver_snd", to_string(m_senders.GetVersion()),false);
            m_pNodeApiPublisher->AddTxt("ver_rcv", to_string(m_receivers.GetVersion()),false);
        }
        m_pNodeApiPublisher->Modify();
    }
}


bool NodeApi::BrowseForRegistrationNode()
{
    if(m_bBrowsing == false)
    {
        ServiceBrowser::Get().AddService("_nmos-registration._tcp", m_pPoster);
        Log::Get() << "Browse for register nodes" << endl;
        ServiceBrowser::Get().StartBrowser();
        m_bBrowsing = true;
    }

    return true;
}

void NodeApi::SignalBrowse()
{
    m_cvBrowse.notify_one();
}

bool NodeApi::Wait(unsigned long nMilliseconds)
{
    m_mutex.lock();
    m_eSignal = SIG_NONE;
    m_mutex.unlock();

    unique_lock<mutex> ul(m_mutex);
    return (m_cvCommit.wait_for(ul, chrono::milliseconds(nMilliseconds)) == cv_status::no_timeout);
}

void NodeApi::Signal(enumSignal eSignal)
{
    m_mutex.lock();
    m_eSignal = eSignal;
    m_mutex.unlock();

    m_cvCommit.notify_one();
}

void NodeApi::TargetTaken(const std::string& sInterfaceIp, unsigned short nPort, bool bOk)
{
    SignalServer(nPort, bOk, sInterfaceIp);
}

void NodeApi::SenderPatchAllowed(unsigned short nPort, bool bOk)
{
    SignalServer(nPort, bOk, "");
}

void NodeApi::ReceiverPatchAllowed(unsigned short nPort, bool bOk)
{
    SignalServer(nPort, bOk, "");
}

void NodeApi::SignalServer(unsigned short nPort, bool bOk, const std::string& sExtra)
{
    map<unsigned short, MicroServer*>::iterator itServer = m_mServers.find(nPort);
    if(itServer != m_mServers.end())
    {
        itServer->second->Signal(bOk, sExtra);
    }
    else
    {
        Log::Get(Log::LOG_ERROR) << "No server with port " << nPort << endl;
    }
}

void NodeApi::StopRegistrationBrowser()
{
    ServiceBrowser::Get().RemoveService("_nmos-registration._tcp");
}

int NodeApi::RegisterSimple(const ApiVersion& version)
{
    m_nRegistrationStatus = REG_FAILED;
    if(m_sRegistrationNode.empty() == false)
    {
        long nResponse = RegisterResource("node", m_self.GetJson(version));
        if(nResponse == 200)
        {   //Node already registered. Unregister and start again
            UnregisterSimple();
            RegisterSimple(version);
        }
        else if(nResponse == 201)
        {
            if(RegisterResources(m_devices, version) == 201 && RegisterResources(m_sources, version) == 201 &&
               RegisterResources(m_flows, version) == 201 && RegisterResources(m_senders, version) == 201 && RegisterResources(m_receivers, version))
            {
                m_nRegistrationStatus = REG_DONE;
            }
            else
            {//something gone wrong
                UnregisterSimple();
                m_nRegistrationStatus = REG_FAILED;
            }
        }
        else
        {//something gone wrong
            m_nRegistrationStatus = REG_FAILED;
        }
    }
    return m_nRegistrationStatus;
}

int NodeApi::UpdateRegisterSimple(const ApiVersion& version)
{
    long nResponse = RegisterResource("node", m_self.GetJson(version));
    ReregisterResources(m_devices, version);
    ReregisterResources(m_sources, version);
    ReregisterResources(m_flows, version);
    ReregisterResources(m_senders, version);
    return nResponse;
}

bool NodeApi::FindRegistrationNode()
{
    Log::Get(Log::LOG_INFO) << "NodeApi: Find best registration node" << endl;
    map<string, std::shared_ptr<dnsService> >::const_iterator itService = ServiceBrowser::Get().FindService("_nmos-registration._tcp");
    if(itService != ServiceBrowser::Get().GetServiceEnd())
    {
        Log::Get(Log::LOG_INFO) << "NodeApi: Register. Found nmos registration service." << endl;
        shared_ptr<dnsInstance>  pInstance(0);
        string sApiVersion;
        for(map<string, shared_ptr<dnsInstance> >::const_iterator itInstance = itService->second->mInstances.begin(); itInstance != itService->second->mInstances.end(); ++itInstance)
        {   //get the registration node with smallest priority number

            Log::Get(Log::LOG_INFO) << "NodeApi: Register. Found nmos registration node: " << itInstance->first << endl;
            //get top priority
            unsigned long nPriority(200);
            map<string, string>::const_iterator itPriority = itInstance->second->mTxt.find("pri");
            map<string, string>::const_iterator itVersion = itInstance->second->mTxt.find("api_ver");
            if(itPriority != itInstance->second->mTxt.end() && itVersion != itInstance->second->mTxt.end() && SdpManager::CheckIpAddress(itInstance->second->sHostIP) == SdpManager::IP4_UNI)
            {
                if(stoul(itPriority->second) < nPriority && itVersion->second.find("v1.2") != string::npos)
                {//for now only doing v1.2


                    pInstance = itInstance->second;
                    nPriority = stoi(itPriority->second);

                    Log::Get(Log::LOG_INFO) << "NodeApi: Register. Found nmos registration node with v1.2 and low priority: " << itInstance->second->sName << " " << nPriority << endl;

                    //vector<string> vApi;
                    //SplitString(vApi, itVersion->second, ',');
                }
            }
        }

        if(pInstance)
        {
            //build the registration url
            stringstream ssUrl;
            ssUrl <<  pInstance->sHostIP << ":" << pInstance->nPort << "/x-nmos/registration/v1.2";
            m_sRegistrationNode = ssUrl.str();
            Log::Get(Log::LOG_INFO) << "NodeApi: Registration Url = " << m_sRegistrationNode << endl;
            return true;
        }
    }
    m_sRegistrationNode.clear();
    Log::Get(Log::LOG_INFO) << "NodeApi: Register: No nmos registration nodes found. Go peer-to-peer" << endl;
    m_nRegistrationStatus = REG_FAILED;

    return false;
}


template<class T> long NodeApi::RegisterResources(ResourceHolder<T>& holder, const ApiVersion& version)
{
    for(typename map<string, shared_ptr<T> >::const_iterator itResource = holder.GetResourceBegin(); itResource != holder.GetResourceEnd(); ++itResource)
    {
        Log::Get(Log::LOG_INFO) << "NodeApi: Register. " << holder.GetType() << " : " << itResource->first << endl;
        long nResult = RegisterResource(holder.GetType(), itResource->second->GetJson(version));
        if(nResult != 201)
        {
            return nResult;
        }
    }
    return 201;
}

template<class T> long NodeApi::ReregisterResources(ResourceHolder<T>& holder, const ApiVersion& version)
{
    for(typename map<string, shared_ptr<T> >::const_iterator itResource = holder.GetChangedResourceBegin(); itResource != holder.GetChangedResourceEnd(); ++itResource)
    {
        Log::Get(Log::LOG_INFO) << "NodeApi: Register. " << holder.GetType() << " : " << itResource->first << endl;
        long nResult = RegisterResource(holder.GetType(), itResource->second->GetJson(version));
        if(nResult != 200 && nResult != 201)
        {
            return nResult;
        }
    }
    return 201;
}

long NodeApi::RegisterResource(const string& sType, const Json::Value& json)
{
    Json::Value jsonRegister;
    jsonRegister["type"] = sType;
    jsonRegister["data"] = json;
    Json::StyledWriter stw;
    string sPost(stw.write(jsonRegister));

    string sResponse;

    if(m_pRegisterCurl)
    {
        long nResponse = m_pRegisterCurl->Post(m_sRegistrationNode+"/resource", sPost, sResponse);
        Log::Get(Log::LOG_INFO) << "RegisterApi: returned [" << nResponse << "] " << sResponse << endl;

        return nResponse;
    }
    return 500;
}

long NodeApi::RegistrationHeartbeat()
{
    string sResponse;
    if(m_pRegisterCurl)
    {
        string sResponse;
        long nResponse = m_pRegisterCurl->Post(m_sRegistrationNode+"/health/nodes/"+m_self.GetId(), "", sResponse);
        Log::Get(Log::LOG_INFO) << "RegisterApi: returned [" << nResponse << "] " << sResponse << endl;
        return nResponse;
    }
    return 500;
}

int NodeApi::UnregisterSimple()
{
    Log::Get() << "Unregister: " << m_nRegistrationStatus << endl;

    if(m_sRegistrationNode.empty() == false)
    {
        UnregisterResources(m_receivers);
        UnregisterResources(m_senders);
        UnregisterResources(m_flows);
        UnregisterResources(m_sources);
        UnregisterResources(m_devices);
        UnregisterResource("nodes", m_self.GetId());

        m_nRegistrationStatus = REG_START;
    }
    return m_nRegistrationStatus;
}


template<class T> void NodeApi::UnregisterResources(ResourceHolder<T>& holder)
{
    for(typename map<string, shared_ptr<T> >::const_iterator itResource = holder.GetResourceBegin(); itResource != holder.GetResourceEnd(); ++itResource)
    {
        Log::Get(Log::LOG_INFO) << "NodeApi: Unregister. " << holder.GetType() << " : " << itResource->first << endl;
        UnregisterResource(holder.GetType()+"s", itResource->first);
    }
}


bool NodeApi::UnregisterResource(const string& sType, const std::string& sId)
{
    if(m_pRegisterCurl)
    {
        string sResponse;
        long nResponse = m_pRegisterCurl->Delete(m_sRegistrationNode+"/resource", sType, sId, sResponse);
        Log::Get(Log::LOG_INFO) << "RegisterApi: returned [" << nResponse << "] " << sResponse << endl;
        return true;
    }
    return false;
}


//bool NodeApi::FindQueryNode()
//{
//    if(m_sQueryNode.empty())
//    {
//        map<string, std::shared_ptr<dnsService> >::const_iterator itService = m_pRegistrationBrowser->FindService("_nmos-query._tcp");
//        if(itService != m_pRegistrationBrowser->GetServiceEnd())
//        {
//            Log::Get(Log::LOG_INFO) << "NodeApi: Query. Found nmos query service." << endl;
//            shared_ptr<dnsInstance>  pInstance(0);
//            string sApiVersion;
//            for(map<string, shared_ptr<dnsInstance> >::const_iterator itInstance = itService->second->mInstances.begin(); itInstance != itService->second->mInstances.end(); ++itInstance)
//            {   //get the registration node with smallest priority number
//
//                Log::Get(Log::LOG_INFO) << "NodeApi: Query. Found nmos query node: " << itInstance->second->sName << endl;
//                //get top priority
//                unsigned long nPriority(200);
//                map<string, string>::const_iterator itPriority = itInstance->second->mTxt.find("pri");
//                map<string, string>::const_iterator itVersion = itInstance->second->mTxt.find("api_ver");
//                if(itPriority != itInstance->second->mTxt.end() && itVersion != itInstance->second->mTxt.end())
//                {
//                    if(stoul(itPriority->second) < nPriority && itVersion->second.find("v1.2") != string::npos)
//                    {//for now only doing v1.2
//                        // @todo maybe support other versions?
//                        pInstance = itInstance->second;
//                        nPriority = stoi(itPriority->second);
//
//                        Log::Get(Log::LOG_INFO) << "NodeApi: Query. Found nmos query node with v1.2 and low priority: " << itInstance->second->sName << " " << nPriority << endl;
//
//                    }
//                }
//            }
//
//            if(pInstance)
//            {
//                //build the registration url
//                stringstream ssUrl;
//                ssUrl <<  pInstance->sHostIP << ":" << pInstance->nPort << "/x-nmos/query/v1.2";
//                m_sQueryNode = ssUrl.str();
//                return true;
//            }
//            return false;
//        }
//    }
//    return true;
//}

//const ResourceHolder& NodeApi::GetQueryResults()
//{
//    Log::Get() << "GetQueryResults: " << m_query.GetResourceCount() << " " << (int)&m_query << std::endl;
//    return m_query;
//}

//bool NodeApi::Query(NodeApi::enumResource eResource, const std::string& sQuery)
//{
//    m_query.RemoveAllResources();
//    m_query.SetType(STR_RESOURCE[eResource]);
//
//    if(m_pRegisterCurl && FindQueryNode())
//    {
//        m_pRegisterCurl->Query(m_sQueryNode, eResource, sQuery, &m_query, CURL_QUERY);
//        return true;
//    }
//    return false;
//}


bool NodeApi::IsRunning()
{
    lock_guard<mutex> lg(m_mutex);
    return m_bRun;
}

void NodeApi::StopRun()
{
    lock_guard<mutex> lg(m_mutex);
    m_bRun = false;

}


bool NodeApi::AddControl(const std::string& sDeviceId, const std::string& sEndpointUrl, unsigned short nPort, const std::string& sUrn, MicroServer* pServer)
{
    map<string, shared_ptr<Device> >::iterator itDevice = m_devices.GetResource(sDeviceId, false);
    if(itDevice != m_devices.GetStagedResourceEnd())
    {
        map<unsigned short, MicroServer*>::iterator itServer = m_mServers.find(nPort);
        if(itServer == m_mServers.end())
        {
            if(pServer == NULL)
            {
                return false;
            }
            m_mServers.insert(make_pair(nPort, pServer));
        }

        //add the control endpoints for the urn
        for(set<endpoint>::const_iterator itEndpoint = m_self.GetEndpointsBegin(); itEndpoint != m_self.GetEndpointsEnd(); ++itEndpoint)
        {
            stringstream sstr;
            sstr << "http://" << itEndpoint->sHost << ":" << nPort << "/x-nmos/" << sEndpointUrl;
            itDevice->second->AddControl(sUrn, sstr.str());
        }
        return true;
    }
    return false;
}


bool NodeApi::AddDevice(shared_ptr<Device> pResource)
{
    //make sure the node id agress
    if(pResource->GetParentResourceId() == m_self.GetId())
    {
        m_devices.AddResource(pResource);
        //add the control endpoints for is/05
        for(set<endpoint>::const_iterator itEndpoint = m_self.GetEndpointsBegin(); itEndpoint != m_self.GetEndpointsEnd(); ++itEndpoint)
        {
            stringstream sstr;
            sstr << "http://" << itEndpoint->sHost << ":" << m_nConnectionPort << "/x-nmos/connection/v1.0";
            pResource->AddControl("urn:x-nmos:control:sr-ctrl/v1.0", sstr.str());
        }
        return true;
    }
    return false;
}

bool NodeApi::AddSource(shared_ptr<Source> pResource)
{
    if(m_devices.ResourceExists(pResource->GetParentResourceId()))
    {
        m_sources.AddResource(pResource);
        return true;
    }
    return false;
}

bool NodeApi::AddFlow(shared_ptr<Flow> pResource)
{
    if(m_devices.ResourceExists(pResource->GetParentResourceId()) && m_sources.ResourceExists(pResource->GetSourceId()))
    {
        m_flows.AddResource(pResource);
        return true;
    }
    return false;
}

bool NodeApi::AddReceiver(shared_ptr<Receiver> pResource)
{
    if(m_devices.ResourceExists(pResource->GetParentResourceId()))
    {
        m_receivers.AddResource(pResource);
        return true;
    }
    return false;
}

bool NodeApi::AddSender(shared_ptr<Sender> pResource)
{

    if(m_devices.ResourceExists(pResource->GetParentResourceId()))
    {
        m_senders.AddResource(pResource);

        //Make the IS-04 manifest href the same as the IS-05 control transportfile endpoint
        if(m_self.GetEndpointsBegin() != m_self.GetEndpointsEnd())
        {
            stringstream sstr;
            sstr << "http://" << m_self.GetEndpointsBegin()->sHost << ":" << m_nConnectionPort << "/x-nmos/connection/v1.0/single/senders/";
            sstr << pResource->GetId() << "/transportfile";
            pResource->SetManifestHref(sstr.str());
        }

        return true;
    }
    return false;
}


unsigned short NodeApi::GetConnectionPort() const
{
    return m_nConnectionPort;
}


shared_ptr<Receiver> NodeApi::GetReceiver(const std::string& sId)
{
    map<string, shared_ptr<Receiver> >::iterator itResource = m_receivers.GetResource(sId);
    if(itResource != m_receivers.GetResourceEnd())
    {
        return itResource->second;
    }
    return NULL;
}

shared_ptr<Sender> NodeApi::GetSender(const std::string& sId)
{
    map<string, shared_ptr<Sender> >::iterator itResource = m_senders.GetResource(sId);
    if(itResource != m_senders.GetResourceEnd())
    {
        return itResource->second;
    }
    return NULL;
}


NodeApi::enumSignal NodeApi::GetSignal() const
{
    return m_eSignal;
}


bool NodeApi::ActivateSender(const std::string& sId, const std::string& sSourceIp, const std::string& sDestinationIp, const std::string& sSDP)
{
    shared_ptr<Sender> pSender(GetSender(sId));
    if(pSender)
    {
        pSender->Activate(sSourceIp, sDestinationIp, sSDP);
        Commit();
        return true;
    }
    return false;
}

bool NodeApi::ActivateReceiver(const std::string& sId, const std::string& sInterfaceIp)
{
    shared_ptr<Receiver> pReceiver(GetReceiver(sId));
    if(pReceiver)
    {
        pReceiver->Activate(sInterfaceIp);
        Commit();
        return true;
    }
    return false;
}


void NodeApi::SetHeartbeatTime(unsigned long nMilliseconds)
{
    lock_guard<mutex> lg(m_mutex);
    m_nHeartbeatTime = nMilliseconds;
}

unsigned long NodeApi::GetHeartbeatTime()
{
    lock_guard<mutex> lg(m_mutex);
    return m_nHeartbeatTime;
}
