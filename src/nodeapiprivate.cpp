#include "nodeapiprivate.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#ifdef __GNU__
#include <unistd.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#else
#define WINVER 0x0600
#include <winsock2.h>
#include <windows.h>
#include <iphlpapi.h>
#include <iptypes.h>
#endif
#include "dnssd.h"
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
#include <algorithm>
#include "optional.hpp"
#include "threadpool.h"

using namespace pml::nmos;
using namespace std::placeholders;

pml::restgoose::response NodeApiPrivate::GetRoot(const query& theQuery, const postData& theData, const endpoint& theEndpoint, const userName& theUser)
{
    pml::restgoose::response resp;
    resp.jsonData.append("x-nmos/");
    return resp;
}

pml::restgoose::response NodeApiPrivate::GetNmosDiscoveryRoot(const query& theQuery, const postData& theData, const endpoint& theEndpoint, const userName& theUser)
{
    pml::restgoose::response resp;
    resp.jsonData.append("node/");

    if(m_nConnectionPort == m_nDiscoveryPort)
    {
        resp.jsonData.append("connection/");
    }
    return resp;
}

pml::restgoose::response NodeApiPrivate::GetNmosConnectionRoot(const query& theQuery, const postData& theData, const endpoint& theEndpoint, const userName& theUser)
{
    pml::restgoose::response resp;
    resp.jsonData.append("connection/");

    return resp;
}


pml::restgoose::response NotFound(const query& theQuery, const postData& theData, const endpoint& theEndpoint, const userName& theUser)
{
    pml::restgoose::response resp(404);
    resp.jsonData["code"] = 404;
    resp.jsonData["error"] = theEndpoint.Get() + " not found";
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
    m_mBrowser.insert(std::make_pair("local", std::make_unique<pml::dnssd::Browser>()));
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



    m_lstServers.push_back(std::make_shared<pml::restgoose::Server>());
    m_lstServers.back()->Init(fileLocation(""),fileLocation(""),ipAddress("0.0.0.0"),nConnectionPort, endpoint(""), false);

    m_mDiscoveryServers.insert(std::make_pair(ApiVersion(1,1), make_unique<IS04Server>(m_lstServers.back(), ApiVersion(1,1), m_pPoster,*this)));
    m_mDiscoveryServers.insert(std::make_pair(ApiVersion(1,2), make_unique<IS04Server>(m_lstServers.back(), ApiVersion(1,2), m_pPoster,*this)));


    m_lstServers.back()->AddEndpoint(pml::restgoose::GET, endpoint(""), std::bind(&NodeApiPrivate::GetRoot, this,_1,_2,_3,_4));
    m_lstServers.back()->AddEndpoint(pml::restgoose::GET, endpoint("/x-nmos"), std::bind(&NodeApiPrivate::GetNmosDiscoveryRoot,this, _1,_2,_3,_4));
    m_lstServers.back()->AddNotFoundCallback(std::bind(&NotFound, _1,_2,_3,_4));

    //Create another pml::restgoose server if using different ports
    if(m_nConnectionPort != m_nDiscoveryPort)
    {
         m_lstServers.push_back(std::make_shared<pml::restgoose::Server>());
         m_lstServers.back()->Init(fileLocation(""),fileLocation(""),ipAddress("0.0.0.0"),nDiscoveryPort, endpoint(""), false);
         m_lstServers.back()->AddEndpoint(pml::restgoose::GET, endpoint(""), std::bind(&NodeApiPrivate::GetRoot,this, _1,_2,_3,_4));
         m_lstServers.back()->AddEndpoint(pml::restgoose::GET, endpoint("/x-nmos"), std::bind(&NodeApiPrivate::GetNmosConnectionRoot, this,_1,_2,_3,_4));
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

bool NodeApiPrivate::StartmDNSPublisher()
{
    StopmDNSPublisher();
    auto itEndpoint = m_self.GetEndpointsBegin();
    if(itEndpoint != m_self.GetEndpointsEnd())
    {
        pmlLog(pml::LOG_INFO) << "NMOS: " << "Start mDNS Publisher" ;
        m_pNodeApiPublisher = std::make_unique<pml::dnssd::Publisher>(CreateGuid(itEndpoint->sHost), "_nmos-node._tcp", itEndpoint->nPort, itEndpoint->sHost);
        SetmDNSTxt(itEndpoint->bSecure);
        return m_pNodeApiPublisher->Start();
    }
    return false;

}

void NodeApiPrivate::StopmDNSPublisher()
{
    if(m_pNodeApiPublisher)
    {
        pmlLog(pml::LOG_INFO) << "NMOS: " << "Stop mDNS Publisher" ;
        m_pNodeApiPublisher->Stop();
    }
}


bool NodeApiPrivate::StartServices()
{
    m_pThread = std::make_unique<std::thread>(&NodeApiPrivate::Run, this);

    return true;
}

void NodeApiPrivate::StopServices()
{
    StopRun();

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
    bool bChange = m_self.Commit();

    auto lstChanges = m_sources.Commit(m_self.GetApiVersions());
    lstChanges.splice(lstChanges.end(), m_devices.Commit(m_self.GetApiVersions()));
    lstChanges.splice(lstChanges.end(), m_flows.Commit(m_self.GetApiVersions()));
    lstChanges.splice(lstChanges.end(), m_receivers.Commit(m_self.GetApiVersions()));
    lstChanges.splice(lstChanges.end(), m_senders.Commit(m_self.GetApiVersions()));
    m_mutex.unlock();


    if(bChange || lstChanges.empty() == false)
    {
        if(m_sRegistrationNode.empty())
        {   //update the ver_ text records in peer-to-peer mode
            ModifyTxtRecords();
        }
        else
        {
            //signal the register thread that we need to post resources
            //Signal(SIG_COMMIT);
            //ThreadPool::Get().Submit([=]{UpdateRegisterSimple(bNode);});
            UpdateRegisterSimple(bChange, lstChanges);
        }
    }


    return bChange;
}


void NodeApiPrivate::ModifyTxtRecords()
{
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
    pml::restgoose::response resp;
    resp.nHttpCode = bOk ? 202 : 400;
    resp.data = textData(sInterfaceIp);

    SignalServer(nPort, resp);
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
    pml::restgoose::response resp;
    resp.nHttpCode = bOk ? 202 : 400;
    SignalServer(nPort, resp);
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
    pml::restgoose::response resp;
    resp.nHttpCode = bOk ? 202 : 400;
    SignalServer(nPort, resp);
}

void NodeApiPrivate::SignalServer(unsigned short nPort, const pml::restgoose::response& resp)
{
    auto itServer = std::find_if(m_lstServers.begin(), m_lstServers.end(), [nPort](std::shared_ptr<pml::restgoose::Server> pGoose){ return pGoose->GetPort() == nPort;});
    if(itServer != m_lstServers.end())
    {
        (*itServer)->Signal(resp);
    }
    else
    {
        pmlLog(pml::LOG_ERROR) << "NMOS: " << "No server with port " << nPort ;
    }
}

void NodeApiPrivate::StopRegistrationBrowsing()
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
            m_nRegistrationStatus = REG_DONE;
        }
    }
    PostRegisterStatus();
    return m_nRegistrationStatus;
}

void NodeApiPrivate::UpdateRegisterSimple(bool bSelf, std::list<std::shared_ptr<Resource>> lstChanges)
{
    if(bSelf)
    {
        RegisterResource("node", m_self.GetJson(m_versionRegistration));
    }
    for(auto pResource : lstChanges)
    {
        RegisterResource(pResource->GetType(), pResource->GetJson(m_versionRegistration));
    }

}

void NodeApiPrivate::HandleInstanceResolved(std::shared_ptr<pml::dnssd::dnsInstance> pInstance)
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

void NodeApiPrivate::HandleInstanceRemoved(std::shared_ptr<pml::dnssd::dnsInstance> pInstance)
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

        if(pairNode.second.bGood && pairNode.second.nPriority < nPriority)
        {//for now only doing v1.2
            nPriority = pairNode.second.nPriority;
            sRegNode = pairNode.first;
            version = pairNode.second.version;
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

        StopRegistrationBrowsing();
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
                pResource->AddControl(control("urn:x-nmos:control:sr-ctrl/"+pairServer.first.GetVersionAsString()), endpoint(sstr.str()));
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
    auto pDevice = m_devices.GetStagedResource(pResource->GetParentResourceId());
    if(pDevice)
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

            pDevice->AddReceiver(pResource->GetId());

            //actualize the receiver for IS05
            auto itInterface = m_self.GetInterfaceBegin();
            if(itInterface != m_self.GetInterfaceEnd())
            {
                pResource->ActualizeUnitialisedActive(itInterface->second.sMainIpAddress);
            }

            //constrain the interface_ip to those that we have
            std::vector<pairEnum_t> vEnum;
            std::transform(m_self.GetInterfaceBegin(), m_self.GetInterfaceEnd(), std::back_inserter(vEnum),
                           [](const std::pair<std::string, nodeinterface>& interface){ return pairEnum_t(jsondatatype::_STRING, interface.second.sMainIpAddress); });

            pResource->AddConstraint(TransportParamsRTP::INTERFACE_IP, {},{},{},vEnum, {});

            return true;
        }
    }
    return false;
}

bool NodeApiPrivate::AddSender(shared_ptr<Sender> pResource)
{
    auto pDevice = m_devices.GetStagedResource(pResource->GetParentResourceId());
    if(pDevice)
    {
        if(m_senders.AddResource(pResource))
        {
            pDevice->AddSender(pResource->GetId());

            //Make the IS-04 manifest href the same as the IS-05 control transportfile endpoint
            if(m_self.GetEndpointsBegin() != m_self.GetEndpointsEnd())
            {
                stringstream sstr;
                sstr << "http://" << m_self.GetEndpointsBegin()->sHost << ":" << m_nConnectionPort << "/x-nmos/connection/v1.0/single/senders/";
                sstr << pResource->GetId() << "/transportfile";
                pResource->SetManifestHref(sstr.str());
            }
            //actualize the sender for IS05
            auto itInterface = m_self.GetInterfaceBegin();
            if(itInterface != m_self.GetInterfaceEnd())
            {
                // @todo should we create an SDP for those that need one??
                pResource->ActualizeUnitialisedActive(itInterface->second.sMainIpAddress);
            }

            //constrain the source_ip to those that we have
            std::vector<pairEnum_t> vEnum;
            std::transform(m_self.GetInterfaceBegin(), m_self.GetInterfaceEnd(), std::back_inserter(vEnum),
                           [](const std::pair<std::string, nodeinterface>& interface){ return pairEnum_t(jsondatatype::_STRING, interface.second.sMainIpAddress); });

            pResource->AddConstraint(TransportParamsRTP::SOURCE_IP, {},{},{},vEnum, {});

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
    pmlLog(pml::LOG_ERROR) << "NMOS: AddSender - could not find parent device '" << pResource->GetParentResourceId() << "'";
    return false;
}

void NodeApiPrivate::RemoveSender(const std::string& sId)
{
    auto pSender = GetSender(sId);
    if(pSender)
    {
        auto pDevice = m_devices.GetStagedResource(pSender->GetParentResourceId());
        if(pDevice)
        {
            pDevice->RemoveSender(sId);
        }
    }

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
    auto pReceiver = GetReceiver(sId);
    if(pReceiver)
    {
        auto pDevice = m_devices.GetStagedResource(pReceiver->GetParentResourceId());
        if(pDevice)
        {
            pDevice->RemoveReceiver(sId);
        }
    }
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
    return nullptr;
}

shared_ptr<Sender> NodeApiPrivate::GetSender(const std::string& sId)
{
    auto itResource = m_senders.GetResource(sId);
    if(itResource != m_senders.GetResourceEnd())
    {
        return itResource->second;
    }
    return nullptr;
}

shared_ptr<Device> NodeApiPrivate::GetDevice(const std::string& sId)
{
    auto itResource = m_devices.GetResource(sId);
    if(itResource != m_devices.GetResourceEnd())
    {
        return itResource->second;
    }
    return nullptr;
}

shared_ptr<Source> NodeApiPrivate::GetSource(const std::string& sId)
{
    auto itResource = m_sources.GetResource(sId);
    if(itResource != m_sources.GetResourceEnd())
    {
        return itResource->second;
    }
    return nullptr;
}

shared_ptr<Flow> NodeApiPrivate::GetFlow(const std::string& sId)
{
    auto itResource = m_flows.GetResource(sId);
    if(itResource != m_flows.GetResourceEnd())
    {
        return itResource->second;
    }
    return nullptr;
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
        return m_mBrowser.insert(std::make_pair(sDomain, std::make_unique<pml::dnssd::Browser>(sDomain))).second;
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

void NodeApiPrivate::CreateSDP(std::shared_ptr<Sender> pSender)
{
    auto pFlow = GetFlow(pSender->GetFlowId());
    if(pFlow)
    {
        auto pSource = GetSource(pFlow->GetSourceId());
        if(pSource)
        {
            pSender->SetTransportFile(SdpManager::TransportParamsToSdp(GetSelf(), pSender, pFlow, pSource));
        }
    }
}

void NodeApiPrivate::Activate(bool bCommit, std::shared_ptr<IOResource> pResource)
{
    auto pSender = std::dynamic_pointer_cast<Sender>(pResource);
    if(pSender)
    {
        Activate(bCommit, pSender);
        return;
    }


    auto pReceiver = std::dynamic_pointer_cast<Receiver>(pResource);
    if(pReceiver)
    {
        Activate(bCommit, pReceiver);
        return;
    }

}

void NodeApiPrivate::Activate(bool bCommit, std::shared_ptr<Sender> pSender)
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


    if(bCommit)
    {
        CommitActivation(pSender);
    }

}

void NodeApiPrivate::CommitActivation(std::shared_ptr<Sender> pSender)
{
    pSender->CommitActivation();
    SenderActivated(pSender->GetId());
    Commit();
}

bool NodeApiPrivate::Stage(const connectionSender<activationResponse>& conRequest, std::shared_ptr<Sender> pSender)
{
    bool bOk = true;
    switch(pSender->Stage(conRequest))
    {
        case activation::ACT_NULL:
            if(pSender->GetStaged().GetActivation().GetActivationTimePoint())
            {
                m_activator.RemoveActivation(*(pSender->GetStaged().GetActivation().GetActivationTimePoint()), pSender);
                pSender->RemoveStagedActivationTime();
            }
            pSender->SetActivationAllowed(false);
            break;
        case activation::ACT_NOW:
            pSender->SetStagedActivationTimePoint(GetTaiTimeNow());
            pSender->SetActivationAllowed(true);
            Activate(false, pSender);
            break;
        case activation::ACT_ABSOLUTE:
            {
                auto tp = ConvertTaiStringToTimePoint((*pSender->GetStaged().GetActivation().GetRequestedTime()));
                if(tp)
                {
                    pSender->SetActivationAllowed(true);
                    pSender->SetStagedActivationTimePoint(*tp);

                    m_activator.AddActivation(*tp, pSender);

                }
                else
                {
                    pmlLog(pml::LOG_ERROR) << "NMOS: " << "Stage Sender: Invalid absolute time" ;
                    bOk = false;
                }
            }
            break;
        case activation::ACT_RELATIVE:
            // start a thread that will sleep for the given time period and then tell the main thread to do its stuff
            {
                auto tp = std::chrono::high_resolution_clock::now();
                if(AddTaiStringToTimePoint((*pSender->GetStaged().GetActivation().GetRequestedTime()), tp))
                {
                    pSender->SetActivationAllowed(true);
                    pSender->SetStagedActivationTimePoint(tp);
                    m_activator.AddActivation(tp, pSender);
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


void NodeApiPrivate::Activate(bool bCommit, std::shared_ptr<Receiver> pReceiver)
{
    pReceiver->Activate();
    if(bCommit)
    {
        CommitActivation(pReceiver);
    }

}

void NodeApiPrivate::CommitActivation(std::shared_ptr<Receiver> pReceiver)
{
    pReceiver->CommitActivation();
    ReceiverActivated(pReceiver->GetId());
    Commit();
}

bool NodeApiPrivate::Stage(const connectionReceiver<activationResponse>& conRequest, std::shared_ptr<Receiver> pReceiver)
{
    bool bOk = true;
    switch(pReceiver->Stage(conRequest))
    {
        case activation::ACT_NULL:
            if(pReceiver->GetStaged().GetActivation().GetActivationTimePoint())
            {
                m_activator.RemoveActivation((*pReceiver->GetStaged().GetActivation().GetActivationTimePoint()), pReceiver);
                pReceiver->RemoveStagedActivationTime();
            }
            pReceiver->SetActivationAllowed(false);
            break;
        case activation::ACT_NOW:
            pReceiver->SetStagedActivationTimePoint(GetTaiTimeNow());
            pReceiver->SetActivationAllowed(true);
            Activate(false, pReceiver);
            break;
        case activation::ACT_ABSOLUTE:
            {
                auto tp = ConvertTaiStringToTimePoint((*pReceiver->GetStaged().GetActivation().GetRequestedTime()));
                if(tp)
                {
                    pReceiver->SetActivationAllowed(true);
                    pReceiver->SetStagedActivationTimePoint(*tp);

                    m_activator.AddActivation(*tp, pReceiver);
                }
                else
                {
                    pmlLog(pml::LOG_ERROR) << "NMOS: " << "Stage Receiver: Invalid absolute time" ;
                    bOk = false;
                }
            }
            break;
        case activation::ACT_RELATIVE:
            // start a thread that will sleep for the given time period and then tell the main thread to do its stuff
            {
                auto tp = std::chrono::high_resolution_clock::now();
                if(AddTaiStringToTimePoint((*pReceiver->GetStaged().GetActivation().GetRequestedTime()), tp))
                {
                    pReceiver->SetActivationAllowed(true);
                    pReceiver->SetStagedActivationTimePoint(tp);
                    m_activator.AddActivation(tp, pReceiver);
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

void NodeApiPrivate::SubscribeToSender(std::shared_ptr<Receiver> pReceiver, const std::string& sSenderId, const std::string& sSdp, const std::string& sInterfaceIp)
{
    pReceiver->SubscribeToSender(sSenderId, sSdp, sInterfaceIp);
    Activate(true, pReceiver);
    pReceiver->UpdateVersionTime();
    Commit();
}
