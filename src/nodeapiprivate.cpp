#include "nodeapiprivate.h"
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
#include "receiverbase.h"
#include "senderbase.h"
#include "is04server.h"
#include "is05server.h"
#include "nmosthread.h"
#include "sdp.h"
#include "utils.h"
#include "nodezcposter.h"
#include <algorithm>
#include <numeric>
#include <memory>
#include "flowaudiocoded.h"
#include "flowaudioraw.h"
#include "flowvideoraw.h"
#include "flowvideocoded.h"
#include "flowdatasdianc.h"
#include "flowdatajson.h"
#include "flowmux.h"
#include "flowsdpcreatornode.h"

using namespace pml::nmos;
using namespace std::placeholders;

response NodeApiPrivate::GetRoot(const query& theQuery, const postData& theData, const url& theUrl, const userName& theUser)
{
    response resp;
    resp.jsonData.append("x-nmos/");
    return resp;
}

response NodeApiPrivate::GetNmosDiscoveryRoot(const query& theQuery, const postData& theData, const url& theUrl, const userName& theUser)
{
    response resp;
    resp.jsonData.append("node/");

    if(m_nConnectionPort == m_nDiscoveryPort)
    {
        resp.jsonData.append("connection/");
    }
    return resp;
}

response NodeApiPrivate::GetNmosConnectionRoot(const query& theQuery, const postData& theData, const url& theUrl, const userName& theUser)
{
    response resp;
    resp.jsonData.append("connection/");

    return resp;
}


response NotFound(const query& theQuery, const postData& theData, const url& theUrl, const userName& theUser)
{
    response resp(404);
    resp.jsonData["code"] = 404;
    resp.jsonData["error"] = theUrl.Get() + " not found";
    resp.jsonData["debug"] = "null";

    return resp;
}


using namespace std;

const std::string NodeApiPrivate::STR_RESOURCE[7] = {"node", "device", "source", "flow", "sender", "receiver", "subscription"};




NodeApiPrivate::NodeApiPrivate() :
m_devices("device"),
m_senders("sender"),
m_receivers("receiver"),
m_sources("source"),
m_flows("flow"),
m_versionRegistration(0,0),
m_pNodeApiPublisher(nullptr),
m_nRegistrationStatus(REG_START),
m_pThread(nullptr),
m_bRun(true),
m_bBrowsing(false),
m_pPoster(0),
m_pZCPoster(make_shared<NodeZCPoster>(*this)),
m_nConnectionPort(0),
m_nDiscoveryPort(0),
m_nHeartbeatTime(5000),
m_activator(*this)
{
    m_mBrowser.insert(std::make_pair("local", std::make_unique<ServiceBrowser>()));
}

NodeApiPrivate::~NodeApiPrivate()
{
    StopServices();
}



void NodeApiPrivate::Init(std::shared_ptr<EventPoster> pPoster, unsigned short nDiscoveryPort, unsigned short nConnectionPort, const string& sLabel, const string& sDescription)
{

    m_nConnectionPort = nConnectionPort;
    m_nDiscoveryPort = nDiscoveryPort;

    m_pPoster = pPoster;

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
                pmlLog(pml::LOG_DEBUG) << "NMOS: " << "Interface: " << temp_addr->ifa_name ;
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
                    pmlLog(pml::LOG_DEBUG) << "NMOS: " << "Base: " << Resource::s_sBase ;
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
            pmlLog(pml::LOG_INFO) << "NMOS: " << "Memory allocation failed for IP_ADAPTER_ADDRESSES struct" ;
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
        pmlLog(pml::LOG_INFO) << "NMOS: " << "Call to GetAdaptersAddresses failed with error: " << dwRetVal ;
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



    m_lstServers.push_back(std::make_shared<RestGoose>());
    m_lstServers.back()->Init("","",nConnectionPort, "", false);

    m_mDiscoveryServers.insert(std::make_pair(ApiVersion(1,1), make_unique<IS04Server>(m_lstServers.back(), ApiVersion(1,1), m_pPoster,*this)));
    m_mDiscoveryServers.insert(std::make_pair(ApiVersion(1,2), make_unique<IS04Server>(m_lstServers.back(), ApiVersion(1,2), m_pPoster,*this)));


    m_lstServers.back()->AddEndpoint(endpoint(RestGoose::GET, url("")), std::bind(&NodeApiPrivate::GetRoot, this,_1,_2,_3,_4));
    m_lstServers.back()->AddEndpoint(endpoint(RestGoose::GET, url("/x-nmos")), std::bind(&NodeApiPrivate::GetNmosDiscoveryRoot,this, _1,_2,_3,_4));
    m_lstServers.back()->AddNotFoundCallback(std::bind(&NotFound, _1,_2,_3,_4));

    //Create another RestGoose server if using different ports
    if(m_nConnectionPort != m_nDiscoveryPort)
    {
         m_lstServers.push_back(std::make_shared<RestGoose>());
         m_lstServers.back()->Init("","",nDiscoveryPort, "", false);
         m_lstServers.back()->AddEndpoint(endpoint(RestGoose::GET, url("")), std::bind(&NodeApiPrivate::GetRoot,this, _1,_2,_3,_4));
         m_lstServers.back()->AddEndpoint(endpoint(RestGoose::GET, url("/x-nmos")), std::bind(&NodeApiPrivate::GetNmosConnectionRoot, this,_1,_2,_3,_4));
         m_lstServers.back()->AddNotFoundCallback(std::bind(&NotFound, _1,_2,_3,_4));
    }

    m_mConnectionServers.insert(std::make_pair(ApiVersion(1,0), make_unique<IS05Server>(m_lstServers.back(), ApiVersion(1,0), m_pPoster,*this)));
    m_mConnectionServers.insert(std::make_pair(ApiVersion(1,1), make_unique<IS05Server>(m_lstServers.back(), ApiVersion(1,1), m_pPoster,*this)));


}


bool NodeApiPrivate::StartHttpServers()
{
    pmlLog(pml::LOG_DEBUG) << "NMOS: " << "Start Http Servers" ;

    for(auto& pServer : m_lstServers)
    {
        pServer->Run(true);
    }

    pmlLog(pml::LOG_DEBUG) << "NMOS: " << "Start Http Servers: Done" ;
    return true;

}

void NodeApiPrivate::StopHttpServers()
{
    pmlLog(pml::LOG_DEBUG) << "NMOS: NodeApi - stop http servers";
    for(auto pServer : m_lstServers)
    {
        pServer->Stop();
    }
    pmlLog(pml::LOG_DEBUG) << "NMOS: NodeApi - stop http servers - done";
}

bool NodeApiPrivate::StartmDNSServer()
{
    StopmDNSServer();
    auto itEndpoint = m_self.GetEndpointsBegin();
    if(itEndpoint != m_self.GetEndpointsEnd())
    {
        pmlLog(pml::LOG_INFO) << "NMOS: " << "Start mDNS Publisher" ;
        m_pNodeApiPublisher.reset(new ServicePublisher(CreateGuid(itEndpoint->sHost), "_nmos-node._tcp", itEndpoint->nPort, itEndpoint->sHost));
        SetmDNSTxt(itEndpoint->bSecure);
        return m_pNodeApiPublisher->Start();
    }
    return false;

}

void NodeApiPrivate::StopmDNSServer()
{
    pmlLog(pml::LOG_DEBUG) << "NMOS: NodeApi - stop dns-sd";
    if(m_pNodeApiPublisher)
    {
        pmlLog(pml::LOG_INFO) << "NMOS: " << "Stop mDNS Publisher" ;
        m_pNodeApiPublisher->Stop();
    }
    pmlLog(pml::LOG_DEBUG) << "NMOS: NodeApi - stop dns-sd - done";
}


bool NodeApiPrivate::StartServices()
{

    m_pThread = std::make_unique<std::thread>(&NodeApiPrivate::Run, this);

    return true;
}

void NodeApiPrivate::StopServices()
{
    if(m_pThread)
    {
        StopRun();
        StopHttpServers();
        StopmDNSServer();
        StopRegistrationBrowser();
    }
}



Self& NodeApiPrivate::GetSelf()
{
    return m_self;
}

const ResourceHolder<Source>& NodeApiPrivate::GetSources()
{
    return m_sources;
}

const ResourceHolder<Device>& NodeApiPrivate::GetDevices()
{
    return m_devices;
}

const ResourceHolder<Flow>& NodeApiPrivate::GetFlows()
{
    return m_flows;
}

const ResourceHolder<Receiver>& NodeApiPrivate::GetReceivers()
{
    return m_receivers;
}

const ResourceHolder<Sender>& NodeApiPrivate::GetSenders()
{
    return m_senders;
}

bool NodeApiPrivate::Commit()
{
    m_mutex.lock();
    pmlLog(pml::LOG_INFO) << "NMOS: " << "Node: Commit" ;
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


void NodeApiPrivate::ModifyTxtRecords()
{
    pmlLog(pml::LOG_TRACE) << "Nmos: NodeApi - ModifyTxRecords";
    auto itEndpoint = m_self.GetEndpointsBegin();
    if(itEndpoint != m_self.GetEndpointsEnd())
    {
        SetmDNSTxt(itEndpoint->bSecure);
    }
}


void NodeApiPrivate::SetmDNSTxt(bool bSecure)
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
        std::string sVersion;
        for(const auto& pairServer : m_mDiscoveryServers)
        {
            if(sVersion.empty() == false)
            {
                sVersion += ',';
            }
            sVersion += pairServer.first.GetVersionAsString();

        }

         m_pNodeApiPublisher->AddTxt("api_ver", sVersion, false);

        if(m_sRegistrationNode.empty())
        {
            pmlLog(pml::LOG_DEBUG) << "NMOS: " << "Peer to peer mode" ;
            m_pNodeApiPublisher->AddTxt("ver_slf", to_string(m_self.GetDnsVersion()),false);
            m_pNodeApiPublisher->AddTxt("ver_src", to_string(m_sources.GetVersion()),false);
            m_pNodeApiPublisher->AddTxt("ver_flw", to_string(m_flows.GetVersion()),false);
            m_pNodeApiPublisher->AddTxt("ver_dvc", to_string(m_devices.GetVersion()),false);
            m_pNodeApiPublisher->AddTxt("ver_snd", to_string(m_senders.GetVersion()),false);
            m_pNodeApiPublisher->AddTxt("ver_rcv", to_string(m_receivers.GetVersion()),false);
        }
        else
        {
            m_pNodeApiPublisher->RemoveTxt("ver_slf", false);
            m_pNodeApiPublisher->RemoveTxt("ver_src", false);
            m_pNodeApiPublisher->RemoveTxt("ver_flw", false);
            m_pNodeApiPublisher->RemoveTxt("ver_dvc", false);
            m_pNodeApiPublisher->RemoveTxt("ver_snd", false);
            m_pNodeApiPublisher->RemoveTxt("ver_rcv", false);
        }
        m_pNodeApiPublisher->Modify();
    }
}


bool NodeApiPrivate::BrowseForRegistrationNode()
{
    if(m_bBrowsing == false)
    {
        for(const auto& pairBrowser : m_mBrowser)
        {
            if(m_mConnectionServers.begin()->first < ApiVersion(1,3))
            {
                pairBrowser.second->AddService("_nmos-registration._tcp", m_pZCPoster);
            }
            if(m_mConnectionServers.rbegin()->first > ApiVersion(1,2))
            {
                pairBrowser.second->AddService("_nmos-register._tcp", m_pZCPoster);
            }

            pairBrowser.second->StartBrowser();
        }

        pmlLog(pml::LOG_INFO) << "NMOS: " << "Browse for register nodes" ;

        m_bBrowsing = true;
    }

    return true;
}

void NodeApiPrivate::SignalBrowse()
{
    m_cvBrowse.notify_one();
}

bool NodeApiPrivate::Wait(unsigned long nMilliseconds)
{
    m_mutex.lock();
    m_eSignal = SIG_NONE;
    m_mutex.unlock();

    unique_lock<mutex> ul(m_mutex);
    return (m_cvCommit.wait_for(ul, chrono::milliseconds(nMilliseconds)) == cv_status::no_timeout);
}

bool NodeApiPrivate::WaitUntil(const std::chrono::system_clock::time_point& timeout_time)
{
    m_mutex.lock();
    m_eSignal = SIG_NONE;
    m_mutex.unlock();

    unique_lock<mutex> ul(m_mutex);
    return (m_cvCommit.wait_until(ul, timeout_time) == cv_status::no_timeout);
}


void NodeApiPrivate::Signal(enumSignal eSignal)
{
    m_mutex.lock();
    m_eSignal = eSignal;
    m_mutex.unlock();

    m_cvCommit.notify_one();
}

void NodeApiPrivate::TargetTaken(const std::string& sInterfaceIp, unsigned short nPort, bool bOk)
{
    SignalServer(nPort, bOk, sInterfaceIp);
}


void NodeApiPrivate::SenderPatchAllowed(unsigned short nPort, bool bOk, const std::string& sId, const std::string& sSourceIp, const std::string& sDestinationIp, const std::string& sSDP)
{
    if(bOk)
    {
        auto pSender = GetSender(sId);
        if(pSender)
        {
            pSender->SetupActivation(sSourceIp, sDestinationIp, sSDP);
        }
        else
        {
            pmlLog(pml::LOG_ERROR) << "NMOS: " << "No Sender found with id='" << sId << "'" ;
            bOk = false;
        }
    }
    SignalServer(nPort, bOk, "");
}

void NodeApiPrivate::ReceiverPatchAllowed(unsigned short nPort, bool bOk,const std::string& sId, const std::string& sInterfaceIp)
{
    if(bOk)
    {
        auto pReceiver = GetReceiver(sId);
        if(pReceiver)
        {
            pReceiver->SetupActivation(sInterfaceIp);
        }
        else
        {
            bOk = false;
        }
    }
    SignalServer(nPort, bOk, "");
}

void NodeApiPrivate::SignalServer(unsigned short nPort, bool bOk, const std::string& sExtra)
{
    auto itServer = std::find_if(m_lstServers.begin(), m_lstServers.end(), [nPort](std::shared_ptr<RestGoose> pGoose){ return pGoose->GetPort() == nPort;});
    if(itServer != m_lstServers.end())
    {
        (*itServer)->Signal(bOk, sExtra);
    }
    else
    {
        pmlLog(pml::LOG_ERROR) << "NMOS: " << "No server with port " << nPort ;
    }
}

void NodeApiPrivate::StopRegistrationBrowser()
{
    pmlLog(pml::LOG_DEBUG) << "NMOS: NodeApi - stop registration browser";
    for(const auto& pairBrowser : m_mBrowser)
    {
        pairBrowser.second->RemoveService("_nmos-registration._tcp");
    }

    pmlLog(pml::LOG_DEBUG) << "NMOS: NodeApi - stop registration browser - done";
}

void NodeApiPrivate::PostRegisterStatus()
{
    if(m_pPoster)
    {
        if(m_sRegistrationNode.empty())
        {
            m_pPoster->_RegistrationChanged(m_sRegistrationNode, EventPoster::NODE_PEER);
        }
        else
        {
            switch(m_nRegistrationStatus)
            {
                case REG_DONE:
                    m_pPoster->_RegistrationChanged(m_sRegistrationNode, EventPoster::NODE_REGISTERED);
                    break;
                case REG_FAILED:
                    m_pPoster->_RegistrationChanged(m_sRegistrationNode, EventPoster::NODE_REGISTER_FAILED);
                    break;
                default:
                    m_pPoster->_RegistrationChanged(m_sRegistrationNode, EventPoster::NODE_REGISTERING);
            }
        }
    }
}

int NodeApiPrivate::RegisterSimple()
{
    PostRegisterStatus();

    if(m_nRegistrationStatus != REG_DONE)
    {
        m_nRegistrationStatus = REG_FAILED;
        if(m_sRegistrationNode.empty() == false)
        {
            long nResponse = RegisterResource("node", m_self.GetJson(m_versionRegistration));
            if(nResponse == 200)
            {   //Node already registered. Unregister and start again
                UnregisterSimple();
                return RegisterSimple();
            }
            else if(nResponse != 201)
            {
                pmlLog(pml::LOG_ERROR) << "NMOS: " << "RegisterResources: Failed" ;
                UnregisterSimple();
                m_nRegistrationStatus = REG_FAILED;
                PostRegisterStatus();
                return m_nRegistrationStatus;
            }

            if(RegisterResources(m_devices) != 201 || RegisterResources(m_sources) != 201 || RegisterResources(m_flows) != 201 || RegisterResources(m_senders) != 201 || RegisterResources(m_receivers) != 201)
            {
                pmlLog(pml::LOG_ERROR) << "NMOS: " << "RegisterResources: Failed" ;
                UnregisterSimple();
                m_nRegistrationStatus = REG_FAILED;
                PostRegisterStatus();
                return m_nRegistrationStatus;
            }
            pmlLog(pml::LOG_INFO) << "NMOS: " << "RegisterResources: Done" ;
            m_nRegistrationStatus = REG_DONE;
        }
    }
    PostRegisterStatus();
    return m_nRegistrationStatus;
}

int NodeApiPrivate::UpdateRegisterSimple()
{
    long nResponse = RegisterResource("node", m_self.GetJson(m_versionRegistration));
    ReregisterResources(m_devices);
    ReregisterResources(m_sources);
    ReregisterResources(m_flows);
    ReregisterResources(m_senders);
    return nResponse;
}

void NodeApiPrivate::HandleInstanceResolved(std::shared_ptr<dnsInstance> pInstance)
{
    auto itPriority = pInstance->mTxt.find("pri");
    auto itVersion = pInstance->mTxt.find("api_ver");
    auto itProto = pInstance->mTxt.find("api_proto");
    if(itPriority != pInstance->mTxt.end() && itVersion != pInstance->mTxt.end() && SdpManager::CheckIpAddress(pInstance->sHostIP) == SdpManager::IP4_UNI && itProto != pInstance->mTxt.end())
    {
        //check if the registration node can handle one of our versions...

        ApiVersion version(0,0);
        //auto vVersions = SplitString(itVersion->second, ',');
        for(auto itServer  = m_mDiscoveryServers.rbegin(); itServer != m_mDiscoveryServers.rend(); ++itServer)
        {
            if(itVersion->second.find(itServer->first.GetVersionAsString()) != string::npos)
            {
                version = itServer->first;
                break;
            }
        }

        if(version.GetMajor() != 0)
        {
            try
            {
                unsigned short nPriority = stoul(itPriority->second);
                stringstream ssUrl;
                ssUrl <<  pInstance->sHostIP << ":" << pInstance->nPort << "/x-nmos/registration/";
                if(itProto->second == "http" || itProto->second == "https")
                {
                    {
                        std::lock_guard<std::mutex> lg(m_mutex);
                        m_mRegNode.insert(make_pair(ssUrl.str(), regnode(nPriority, version)));
                        if(m_pPoster)
                        {
                            m_pPoster->_RegistrationNodeFound(ssUrl.str(), nPriority, version);
                        }
                    }
                    pmlLog(pml::LOG_DEBUG) << "NMOS: " << "NodeApi: Found registration node" << ssUrl.str() << " proto=" << itProto->second << " ver=" << itVersion->second << " priority=" << itPriority->second ;
                }
            }
            catch(invalid_argument& ia)
            {
                pmlLog(pml::LOG_WARN) << "NMOS: " << "NodeApi: Registration node -Invalid argument";
            }
        }
        else
        {
            pmlLog(pml::LOG_DEBUG) << "NMOS: " << "NodeApi: Found registration node but api versions different to ours";
        }
    }
    else
    {

    }

}

void NodeApiPrivate::HandleInstanceRemoved(std::shared_ptr<dnsInstance> pInstance)
{
    stringstream ssUrl;
    ssUrl <<  pInstance->sHostIP << ":" << pInstance->nPort << "/x-nmos/registration/";

    {
        std::lock_guard<std::mutex> lg(m_mutex);
        m_mRegNode.erase(ssUrl.str());
        if(m_pPoster)
        {
            m_pPoster->_RegistrationNodeRemoved(ssUrl.str());
        }
    }

    if(ssUrl.str() == m_sRegistrationNode)
    {
        Signal(SIG_INSTANCE_REMOVED);
    }
}

bool NodeApiPrivate::FindRegistrationNode()
{
    m_mutex.lock();
    string sRegNode;
    ApiVersion version;
    unsigned long nPriority(200);
    for(auto pairNode : m_mRegNode)
    {   //get the registration node with smallest priority number

        pmlLog(pml::LOG_DEBUG) << "NMOS: " << "NodeApi: Checkregistration node" << pairNode.first ;

        if(pairNode.second.bGood && pairNode.second.nPriority < nPriority)
        {//for now only doing v1.2
            nPriority = pairNode.second.nPriority;
            sRegNode = pairNode.first;
            version = pairNode.second.version;
            pmlLog(pml::LOG_INFO) << "NMOS: " << "NodeApi: Register. Found nmos registration node with v1.2 and lower priority: " << sRegNode << " " << nPriority ;
        }

    }
    m_mutex.unlock();

    if(sRegNode.empty())
    {
        if(m_sRegistrationNode.empty() == false)
        {
            pmlLog(pml::LOG_INFO) << "NMOS: " << "NodeApi: Register: No nmos registration nodes found. Go peer-to-peer" ;
            m_sRegistrationNode.clear();
            m_versionRegistration = ApiVersion(0,0);
            nPriority = 0;
            ModifyTxtRecords();
        }
        m_nRegistrationStatus = REG_FAILED;

    }
    else
    {
        if(m_sRegistrationNode.empty())
        {
            m_nRegistrationStatus = REG_START;
        }
        else
        {
            m_nRegistrationStatus = REG_DONE;
        }
        m_sRegistrationNode = sRegNode;
        m_versionRegistration = version;
        ModifyTxtRecords();
    }
    if(m_pPoster)
    {
        PostRegisterStatus();
    }
    return (m_sRegistrationNode.empty() == false);

}


template<class T> long NodeApiPrivate::RegisterResources(ResourceHolder<T>& holder)
{
    for(auto pairResource : holder.GetResources())
    {
        pmlLog(pml::LOG_INFO) << "NMOS: " << "NodeApi: Register. " << holder.GetType() << " : " << pairResource.first ;
        long nResult = RegisterResource(holder.GetType(), pairResource.second->GetJson(m_versionRegistration));
        if(nResult != 201 && nResult != 200)
        {
            return nResult;
        }
    }
    return 201;
}

template<class T> long NodeApiPrivate::ReregisterResources(ResourceHolder<T>& holder)
{
    for(auto pairResource : holder.GetResources())
    {
        pmlLog(pml::LOG_INFO) << "NMOS: " << "NodeApi: Register. " << holder.GetType() << " : " << pairResource.first ;
        long nResult = RegisterResource(holder.GetType(), pairResource.second->GetJson(m_versionRegistration));
        if(nResult != 200 && nResult != 201)
        {
            return nResult;
        }
    }
    return 201;
}

long NodeApiPrivate::RegisterResource(const string& sType, const Json::Value& json)
{
    Json::Value jsonRegister;
    jsonRegister["type"] = sType;
    jsonRegister["data"] = json;
    string sPost(ConvertFromJson(jsonRegister));


    auto resp =  CurlRegister::Post(m_sRegistrationNode+m_versionRegistration.GetVersionAsString()+"/resource", sPost);
    pmlLog(pml::LOG_INFO) << "NMOS: " << "RegisterApi: Register returned [" << resp.nCode << "] " << resp.sResponse ;
    if(resp.nCode == 500)
    {
        MarkRegNodeAsBad();
    }
    return resp.nCode;

}

long NodeApiPrivate::RegistrationHeartbeat()
{
    m_tpHeartbeat = chrono::system_clock::now() + chrono::milliseconds(m_nHeartbeatTime);

    auto resp = CurlRegister::Post(m_sRegistrationNode+m_versionRegistration.GetVersionAsString()+"/health/nodes/"+m_self.GetId(), "");
    pmlLog(pml::LOG_INFO) << "NMOS: " << "RegisterApi: Heartbeat: returned [" << resp.nCode << "] " << resp.sResponse ;

    if(resp.nCode == 500 || resp.nCode == 0)
    {
        MarkRegNodeAsBad();
    }
    return resp.nCode;

}

void NodeApiPrivate::MarkRegNodeAsBad()
{
    std::lock_guard<std::mutex> lg(m_mutex);
    auto itNode = m_mRegNode.find(m_sRegistrationNode);
    if(itNode != m_mRegNode.end())
    {
        itNode->second.bGood = false;
        if(m_pPoster)
        {
            m_pPoster->_RegistrationNodeChanged(itNode->first, itNode->second.nPriority, itNode->second.bGood, itNode->second.version);
        }
    }

}

int NodeApiPrivate::UnregisterSimple()
{
    pmlLog(pml::LOG_INFO) << "NMOS: " << "Unregister: " << m_nRegistrationStatus ;

    if(m_sRegistrationNode.empty() == false)
    {
        UnregisterResource("nodes", m_self.GetId());
        m_nRegistrationStatus = REG_START;
    }

    //PostRegisterStatus();

    return m_nRegistrationStatus;
}



bool NodeApiPrivate::UnregisterResource(const string& sType, const std::string& sId)
{
    auto resp = CurlRegister::Delete(m_sRegistrationNode+m_versionRegistration.GetVersionAsString()+"/resource", sType, sId);
    pmlLog(pml::LOG_INFO) << "NMOS: " << "RegisterApi:Unregister returned [" << resp.nCode << "] " << resp.sResponse ;
    if(resp.nCode == 500)
    {
        MarkRegNodeAsBad();
        return false;
    }
    return true;

}


//bool NodeApiPrivate::FindQueryNode()
//{
//    if(m_sQueryNode.empty())
//    {
//        map<string, std::shared_ptr<dnsService> >::const_iterator itService = m_pRegistrationBrowser->FindService("_nmos-query._tcp");
//        if(itService != m_pRegistrationBrowser->GetServiceEnd())
//        {
//            pmlLog(pml::LOG_INFO) << "NMOS: " << "NodeApi: Query. Found nmos query service." ;
//            shared_ptr<dnsInstance>  pInstance(0);
//            string sApiVersion;
//            for(map<string, shared_ptr<dnsInstance> >::const_iterator itInstance = itService->second->mInstances.begin(); itInstance != itService->second->mInstances.end(); ++itInstance)
//            {   //get the registration node with smallest priority number
//
//                pmlLog(pml::LOG_INFO) << "NMOS: " << "NodeApi: Query. Found nmos query node: " << itInstance->second->sName ;
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
//                        pmlLog(pml::LOG_INFO) << "NMOS: " << "NodeApi: Query. Found nmos query node with v1.2 and low priority: " << itInstance->second->sName << " " << nPriority ;
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

//const ResourceHolder& NodeApiPrivate::GetQueryResults()
//{
//    pmlLog(pml::LOG_INFO) << "NMOS: " << "GetQueryResults: " << m_query.GetResourceCount() << " " << (int)&m_query ;
//    return m_query;
//}

//bool NodeApiPrivate::Query(NodeApiPrivate::enumResource eResource, const std::string& sQuery)
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


bool NodeApiPrivate::IsRunning()
{
    return m_bRun;
}

void NodeApiPrivate::StopRun()
{
    if(m_pThread)
    {
        pmlLog(pml::LOG_DEBUG) << "NMOS: NodeApi - stop run";
        m_bRun = false;
        Signal(SIG_EXIT);
        m_pThread->join();
        m_pThread = nullptr;
        pmlLog(pml::LOG_DEBUG) << "NMOS: NodeApi - stop done";
    }
}


bool NodeApiPrivate::AddControl(const std::string& sDeviceId, const std::string& sApi, const ApiVersion& version, unsigned short nPort, const std::string& sUrn, shared_ptr<NmosServer> pNmosServer)
{
//    map<string, shared_ptr<Device> >::iterator itDevice = m_devices.GetResource(sDeviceId, false);
//    if(itDevice != m_devices.GetStagedResourceEnd())
//    {
//        map<unsigned short, std::unique_ptr<Server> >::iterator itServer = m_mServers.find(nPort);
//        if(itServer == m_mServers.end())
//        {
//            itServer = m_mServers.insert(make_pair(nPort, new MongooseServer(m_pPoster, nPort))).first;
//        }
//
//        itServer->second->AddNmosControl(sApi, pNmosServer);
//
//        //add the control endpoints for the urn
//        for(auto itEndpoint = m_self.GetEndpointsBegin(); itEndpoint != m_self.GetEndpointsEnd(); ++itEndpoint)
//        {
//            stringstream sstr;
//            sstr << "http://" << itEndpoint->sHost << ":" << nPort << "/x-nmos/" << sApi << "/" << version.GetVersionAsString();
//            itDevice->second->AddControl(sUrn, sstr.str());
//        }
//        return true;
//    }
    return false;
}


bool NodeApiPrivate::AddDevice(shared_ptr<Device> pResource)
{
    //make sure the node id agress
    if(pResource->GetParentResourceId() == m_self.GetId())
    {
        m_devices.AddResource(pResource);

        //add the control endpoints for is/05
        for(const auto& pairServer : m_mConnectionServers)
        {
            for(auto itEndpoint = m_self.GetEndpointsBegin(); itEndpoint != m_self.GetEndpointsEnd(); ++itEndpoint)
            {
                stringstream sstr;
                sstr << "http://" << itEndpoint->sHost << ":" << m_nConnectionPort << "/x-nmos/connection/" << pairServer.first.GetVersionAsString();
                pResource->AddControl("urn:x-nmos:control:sr-ctrl/"+pairServer.first.GetVersionAsString(), sstr.str());
            }
        }
        //add the discovery endpoints
        for(const auto& pairServer : m_mDiscoveryServers)
        {
            pairServer.second->AddDeviceEndpoint(pResource->GetId());
        }

        return true;
    }
    return false;
}

bool NodeApiPrivate::AddSource(shared_ptr<Source> pResource)
{
    if(m_devices.ResourceExists(pResource->GetParentResourceId()))
    {
        m_sources.AddResource(pResource);
        //if we've set a clock then
        string sClock = m_self.GetBestClock();
        if(sClock != "")
        {
            pResource->SetClock(sClock);
        }
        //add the discovery endpoints
        for(const auto& pairServer : m_mDiscoveryServers)
        {
            pairServer.second->AddSourceEndpoint(pResource->GetId());
        }

        return true;
    }
    return false;
}

bool NodeApiPrivate::AddFlow(shared_ptr<Flow> pResource)
{
    if(m_devices.ResourceExists(pResource->GetParentResourceId()) && m_sources.ResourceExists(pResource->GetSourceId()))
    {
        m_flows.AddResource(pResource);

        //add the discovery endpoints
        for(const auto& pairServer : m_mDiscoveryServers)
        {
            pairServer.second->AddFlowEndpoint(pResource->GetId());
        }
        return true;
    }
    return false;
}

bool NodeApiPrivate::AddReceiver(shared_ptr<Receiver> pResource)
{
    if(m_devices.ResourceExists(pResource->GetParentResourceId()))
    {
        if(m_receivers.AddResource(pResource))
        {
            //add the discovery endpoints
            for(const auto& pairServer : m_mDiscoveryServers)
            {
                pairServer.second->AddReceiverEndpoint(pResource->GetId());
            }
            for(const auto& pairServer : m_mConnectionServers)
            {
                pairServer.second->AddReceiverEndpoint(pResource->GetId());
            }
            return true;
        }
    }
    return false;
}

bool NodeApiPrivate::AddSender(shared_ptr<Sender> pResource)
{

    if(m_devices.ResourceExists(pResource->GetParentResourceId()))
    {
        if(m_senders.AddResource(pResource))
        {
            //Make the IS-04 manifest href the same as the IS-05 control transportfile endpoint
            if(m_self.GetEndpointsBegin() != m_self.GetEndpointsEnd())
            {
                stringstream sstr;
                sstr << "http://" << m_self.GetEndpointsBegin()->sHost << ":" << m_nConnectionPort << "/x-nmos/connection/v1.0/single/senders/";
                sstr << pResource->GetId() << "/transportfile";
                pResource->SetManifestHref(sstr.str());
            }

            //add the discovery endpoints
            for(const auto& pairServer : m_mDiscoveryServers)
            {
                pairServer.second->AddSenderEndpoint(pResource->GetId());
            }
            for(const auto& pairServer : m_mConnectionServers)
            {
                pairServer.second->AddSenderEndpoint(pResource->GetId());
            }
            Activate(true, pResource);
            return true;
        }
    }
    return false;
}

void NodeApiPrivate::RemoveSender(const std::string& sId)
{
    m_senders.RemoveResource(sId);
    for(const auto& pairServer : m_mDiscoveryServers)
    {
        pairServer.second->RemoveSenderEndpoint(sId);
    }
    for(const auto& pairServer : m_mConnectionServers)
    {
        pairServer.second->RemoveSenderEndpoint(sId);
    }
}

void NodeApiPrivate::RemoveReceiver(const std::string& sId)
{
    m_receivers.RemoveResource(sId);
     for(const auto& pairServer : m_mDiscoveryServers)
    {
        pairServer.second->RemoveReceiverEndpoint(sId);
    }
    for(const auto& pairServer : m_mConnectionServers)
    {
        pairServer.second->RemoveReceiverEndpoint(sId);
    }
}


unsigned short NodeApiPrivate::GetConnectionPort() const
{
    return m_nConnectionPort;
}

unsigned short NodeApiPrivate::GetDiscoveryPort() const
{
    return m_nConnectionPort;
}


shared_ptr<Receiver> NodeApiPrivate::GetReceiver(const std::string& sId)
{
    auto itResource = m_receivers.GetResource(sId);
    if(itResource != m_receivers.GetResourceEnd())
    {
        return itResource->second;
    }
    return NULL;
}

shared_ptr<Sender> NodeApiPrivate::GetSender(const std::string& sId)
{
    auto itResource = m_senders.GetResource(sId);
    if(itResource != m_senders.GetResourceEnd())
    {
        return itResource->second;
    }
    return NULL;
}


NodeApiPrivate::enumSignal NodeApiPrivate::GetSignal() const
{
    return m_eSignal;
}






void NodeApiPrivate::SetHeartbeatTime(unsigned long nMilliseconds)
{
    lock_guard<mutex> lg(m_mutex);
    m_nHeartbeatTime = nMilliseconds;
}

const std::chrono::system_clock::time_point& NodeApiPrivate::GetHeartbeatTime()
{
    lock_guard<mutex> lg(m_mutex);
    return m_tpHeartbeat;
}


void NodeApiPrivate::ReceiverActivated(const std::string& sId)
{
    Commit();
    if(m_pPoster)
    {
        m_pPoster->_ReceiverActivated(sId);
    }
}

void NodeApiPrivate::SenderActivated(const std::string& sId)
{
    Commit();
    if(m_pPoster)
    {
        m_pPoster->_SenderActivated(sId);
    }
}

Json::Value NodeApiPrivate::JsonConnectionVersions() const
{
    Json::Value jsVersion;
    for(const auto& pairServer : m_mConnectionServers)
    {
        jsVersion.append(pairServer.first.GetVersionAsString()+"/");
    }
    return jsVersion;
}



std::map<std::string, NodeApiPrivate::regnode> NodeApiPrivate::GetRegNodes()
{
    std::lock_guard<std::mutex> lg(m_mutex);
    return m_mRegNode;
}

 const std::string NodeApiPrivate::GetRegistrationNode() const
 {
    std::lock_guard<std::mutex> lg(m_mutex);
    return m_sRegistrationNode;
 }

 ApiVersion NodeApiPrivate::GetRegistrationVersion() const
 {
     std::lock_guard<std::mutex> lg(m_mutex);
     return m_versionRegistration;
 }

bool NodeApiPrivate::AddBrowseDomain(const std::string& sDomain)
{
    if(!m_bRun)
    {
        return m_mBrowser.insert(std::make_pair(sDomain, std::make_unique<ServiceBrowser>(sDomain))).second;
    }
    return false;
}
bool NodeApiPrivate::RemoveBrowseDomain(const std::string& sDomain)
{
    if(!m_bRun)
    {
        m_mBrowser.erase(sDomain);
        return true;
    }
    return false;
}


std::string NodeApiPrivate::CreateFlowSdp(const std::string& sId, const TransportParamsRTPSender& tpSender, const std::set<std::string>& setInterfaces)
{
    std::stringstream ssSDP;
    //now put in the flow media information
    auto itFlow = GetFlows().FindNmosResource(sId);
    if(itFlow != GetFlows().GetResourceEnd())
    {
        auto pFlow = std::dynamic_pointer_cast<Flow>(itFlow->second);
        if(pFlow)
        {
            pmlLog(pml::LOG_DEBUG) << "NMOS: " << "CreateSDP Destination Port = " << tpSender.nDestinationPort ;
            unsigned short nPort(tpSender.nDestinationPort);
            if(nPort == 0)
            {
                nPort = 5004;
            }

            ssSDP << CreateFlowSdpLines(*this, pFlow, nPort);


            //put in the destination unicast/multicast block
            switch(SdpManager::CheckIpAddress(tpSender.sDestinationIp))
            {
                case SdpManager::IP4_UNI:
                    ssSDP << "c=IN IP4 " << tpSender.sDestinationIp << "\r\n";
                    ssSDP << "a=type:unicast\r\n";
                    break;
                case SdpManager::IP4_MULTI:
                    ssSDP << "c=IN IP4 " << tpSender.sDestinationIp << "/32\r\n";
                    ssSDP << "a=source-filter:incl IN IP4 " << tpSender.sDestinationIp << " " << tpSender.sSourceIp << "\r\n";
                    ssSDP << "a=type:multicast\r\n";
                    break;
                case SdpManager::IP6_UNI:
                    ssSDP << "c=IN IP6 " << tpSender.sDestinationIp << "\r\n";
                    ssSDP << "a=type:unicast\r\n";
                    break;
                case SdpManager::IP6_MULTI:
                    ssSDP << "c=IN IP6 " << tpSender.sDestinationIp << "\r\n";
                    ssSDP << "a=source-filter:incl IN IP6 " << tpSender.sDestinationIp << " " << tpSender.sSourceIp << "\r\n";
                    ssSDP << "a=type:multicast\r\n";
                    break;
                case SdpManager::SdpManager::IP_INVALID:
                    pmlLog(pml::LOG_WARN) << "NMOS: Sender can't create SDP - destination IP invalid '" << tpSender.sDestinationIp << "'";
                    break;
            }



            if(tpSender.bRtcpEnabled)
            {
                switch(SdpManager::CheckIpAddress(tpSender.sRtcpDestinationIp))
                {
                    case SdpManager::IP4_UNI:
                    case SdpManager::IP4_MULTI:
                        ssSDP << "a=rtcp:" << tpSender.nRtcpDestinationPort << " IN IP4 " << tpSender.sRtcpDestinationIp << "\r\n";
                        break;
                    case SdpManager::IP6_UNI:
                    case SdpManager::IP6_MULTI:
                        ssSDP << "a=rtcp:" << tpSender.nRtcpDestinationPort << " IN IP6 " << tpSender.sRtcpDestinationIp << "\r\n";
                        break;
                    default:
                        break;
                }
            }

            //get clock name from source
            auto itSource = GetSources().FindNmosResource(pFlow->GetSourceId());
            if(itSource != GetSources().GetResourceEnd())
            {
                auto pSource = std::dynamic_pointer_cast<Source>(itSource->second);
                auto sClock = pSource->GetClock();

                //clock information is probably at the media level
                if(setInterfaces.empty() || sClock.empty())
                {
                    ssSDP << GetSelf().CreateClockSdp(sClock, "");
                }
                else
                {
                    ssSDP << GetSelf().CreateClockSdp(sClock, *(setInterfaces.begin())); // @todo should we check all the intefaces for the clock mac address??
                }
            }
        }
    }
    return ssSDP.str();
}


void NodeApiPrivate::CreateSDP(std::shared_ptr<Sender> pSender)
{
    std::stringstream ssSDP;
    ssSDP << "v=0\r\n";
    ssSDP << "o=- " << GetCurrentTaiTime(false) << " " << GetCurrentTaiTime(false) << " IN IP";
    switch(SdpManager::CheckIpAddress(pSender->GetActive().tpSenders[0].sSourceIp))
    {
        case SdpManager::IP4_UNI:
        case SdpManager::IP4_MULTI:
            ssSDP << "4 ";
            break;
        case SdpManager::IP6_UNI:
        case SdpManager::IP6_MULTI:
            ssSDP << "6 ";
            break;
        case SdpManager::IP_INVALID:
            ssSDP << " ";
            break;
    }
    ssSDP << pSender->GetActive().tpSenders[0].sSourceIp << "\r\n";    // @todo should check here if sSourceIp is not set to auto
    ssSDP << "t=0 0 \r\n";

//    std::map<std::string, std::shared_ptr<Device> >::const_iterator itDevice = GetDevices().FindNmosResource(pSender->GetDeviceId());
//    if(itDevice != GetDevices().GetResourceEnd())
//    {
//        ssSDP << "s=" << itDevice->second->GetLabel() << ":";
//    }
//    else
//    {
//        ssSDP << "s=-:";
//    }
    ssSDP << pSender->GetLabel() << "\r\n";

    for(auto tpSender: pSender->GetActive().tpSenders)
    {
        ssSDP << CreateFlowSdp(pSender->GetFlowId(), tpSender, pSender->GetInterfaces());
    }

    pSender->SetTransportFile(ssSDP.str());
}

void NodeApiPrivate::Activate(bool bImmediate, std::shared_ptr<IOResource> pResource)
{
    auto pSender = std::dynamic_pointer_cast<Sender>(pResource);
    if(pSender)
    {
        Activate(bImmediate, pSender);
        return;
    }


    auto pReceiver = std::dynamic_pointer_cast<Receiver>(pResource);
    if(pReceiver)
    {
        Activate(bImmediate, pReceiver);
        return;
    }

}

void NodeApiPrivate::Activate(bool bImmediate, std::shared_ptr<Sender> pSender)
{
    //get the bound to interface source address
    std::string sSourceIp;
    for(auto sInterface : pSender->GetInterfaces())
    {
        auto itDetails = GetSelf().FindInterface(sInterface);
        if(itDetails != GetSelf().GetInterfaceEnd())
        {
            sSourceIp = itDetails->second.sMainIpAddress;
            break;
        }
    }
    pSender->Activate(sSourceIp);

    // create the SDP
    if(pSender->IsActiveMasterEnabled())
    {
       CreateSDP(pSender);
    }
    else
    {
        pSender->SetTransportFile("");
    }


    if(!bImmediate)
    {
        CommitActivation(pSender);
    }

}

void NodeApiPrivate::CommitActivation(std::shared_ptr<Sender> pSender)
{
    pSender->CommitActivation();
    SenderActivated(pSender->GetId());
}

bool NodeApiPrivate::Stage(const connectionSender& conRequest, std::shared_ptr<Sender> pSender)
{
    bool bOk = true;
    switch(pSender->Stage(conRequest))
    {
        case connection::ACT_NULL:
            if(pSender->GetStaged().sActivationTime.empty() == false)
            {
                m_activator.RemoveActivation(pSender->GetStaged().tpActivation, pSender);
                pSender->RemoveStagedActivationTime();
            }
            pSender->SetActivationAllowed(false);
            break;
        case connection::ACT_NOW:
            pSender->SetStagedActivationTimePoint(GetTaiTimeNow());
            pSender->SetActivationAllowed(true);
            Activate(true, pSender);
            break;
        case connection::ACT_ABSOLUTE:
            {
                std::chrono::time_point<std::chrono::high_resolution_clock> tp;
                if(ConvertTaiStringToTimePoint(pSender->GetStaged().sRequestedTime, tp))
                {
                    pSender->SetActivationAllowed(true);
                    pSender->SetStagedActivationTimePoint(tp);

                    pmlLog(pml::LOG_DEBUG) << "NMOS: Sender  - add absolute activation";
                    m_activator.AddActivation(tp, pSender);

                }
                else
                {
                    pmlLog(pml::LOG_ERROR) << "NMOS: " << "Stage Sender: Invalid absolute time" ;
                    bOk = false;
                }
            }
            break;
        case connection::ACT_RELATIVE:
            // start a thread that will sleep for the given time period and then tell the main thread to do its stuff
            {
                auto tp = std::chrono::high_resolution_clock::now();
                if(AddTaiStringToTimePoint(pSender->GetStaged().sRequestedTime, tp))
                {
                    pSender->SetActivationAllowed(true);
                    pSender->SetStagedActivationTimePoint(tp);
                    m_activator.AddActivation(tp, pSender);
                    pmlLog(pml::LOG_DEBUG) << "NMOS: Receiver  - add relative activation: " << ConvertTimeToString(tp);
                }
                else
                {
                    pmlLog(pml::LOG_ERROR) << "NMOS: " << "Stage Receiver: Invalid time" ;
                    bOk = false;
                }

            }
            break;
    }
    return bOk;
}


void NodeApiPrivate::Activate(bool bImmediate, std::shared_ptr<Receiver> pReceiver)
{
    pReceiver->Activate();
    if(!bImmediate)
    {
        CommitActivation(pReceiver);
    }

}

void NodeApiPrivate::CommitActivation(std::shared_ptr<Receiver> pReceiver)
{
    pReceiver->CommitActivation();
    SenderActivated(pReceiver->GetId());
}

bool NodeApiPrivate::Stage(const connectionReceiver& conRequest, std::shared_ptr<Receiver> pReceiver)
{
    bool bOk = true;
    switch(pReceiver->Stage(conRequest))
    {
        case connection::ACT_NULL:
            if(pReceiver->GetStaged().sActivationTime.empty() == false)
            {
                m_activator.RemoveActivation(pReceiver->GetStaged().tpActivation, pReceiver);
                pReceiver->RemoveStagedActivationTime();
            }
            pReceiver->SetActivationAllowed(false);
            break;
        case connection::ACT_NOW:
            pReceiver->SetStagedActivationTimePoint(GetTaiTimeNow());
            pReceiver->SetActivationAllowed(true);
            Activate(true, pReceiver);
            break;
        case connection::ACT_ABSOLUTE:
            {
                std::chrono::time_point<std::chrono::high_resolution_clock> tp;
                if(ConvertTaiStringToTimePoint(pReceiver->GetStaged().sRequestedTime, tp))
                {
                    pReceiver->SetActivationAllowed(true);
                    pReceiver->SetStagedActivationTimePoint(tp);

                    pmlLog(pml::LOG_DEBUG) << "NMOS: Receiver  - add absolute activation";
                    m_activator.AddActivation(tp, pReceiver);
                }
                else
                {
                    pmlLog(pml::LOG_ERROR) << "NMOS: " << "Stage Receiver: Invalid absolute time" ;
                    bOk = false;
                }
            }
            break;
        case connection::ACT_RELATIVE:
            // start a thread that will sleep for the given time period and then tell the main thread to do its stuff
            {
                auto tp = std::chrono::high_resolution_clock::now();
                if(AddTaiStringToTimePoint(pReceiver->GetStaged().sRequestedTime, tp))
                {
                    pReceiver->SetActivationAllowed(true);
                    pReceiver->SetStagedActivationTimePoint(tp);
                    m_activator.AddActivation(tp, pReceiver);
                    pmlLog(pml::LOG_DEBUG) << "NMOS: Receiver  - add relative activation: " << ConvertTimeToString(tp);
                }
                else
                {
                    pmlLog(pml::LOG_ERROR) << "NMOS: " << "Stage Receiver: Invalid time" ;
                    bOk = false;
                }
            }
            break;
        default:
            pmlLog(pml::LOG_ERROR) << "Unexpected patch" ;
    }
    return bOk;
}

void NodeApiPrivate::SetSender(std::shared_ptr<Receiver> pReceiver, const std::string& sSenderId, const std::string& sSdp, const std::string& sInterfaceIp)
{
    pReceiver->SetSender(sSenderId, sSdp, sInterfaceIp);
    Activate(false, pReceiver);
    pReceiver->UpdateVersionTime();
    Commit();
}
