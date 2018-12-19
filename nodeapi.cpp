#include "nodeapi.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include "avahipublisher.h"
#include "avahibrowser.h"
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
#include "microserver.h"

#ifdef __GNU__
#include <unistd.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#endif // __GNU__
using namespace std;

const std::string NodeApi::STR_RESOURCE[7] = {"node", "device", "source", "flow", "sender", "receiver", "subscription"};

static bool Reregister(EventPoster* pPoster)
{
    if(NodeApi::Get().RegisterSimple() != NodeApi::REG_DONE)
    {
        //tell the main thread and exit this one
        if(pPoster)
        {
            pPoster->RegistrationNodeError();
        }
        return false;
    }
    return true;
}


static void RegisterThread(EventPoster* pPoster)
{
    if(NodeApi::Get().RegisterSimple() == NodeApi::REG_DONE)
    {
        bool bError(false);
        while(NodeApi::Get().IsRunning())
        {
            this_thread::sleep_for(chrono::seconds(5));
            long nResponse = NodeApi::Get().RegistrationHeartbeat();
            switch(nResponse)
            {
                case 200:
                    //this is ok
                    break;
                case 0:
                    //this means the server has gone
                    //Do we have any other nodes?
                    if(NodeApi::Get().FindRegistrationNode() == false || Reregister(pPoster) == false)
                    {
                        bError = true;
                        NodeApi::Get().StopRun();
                    }
                    break;
                case 404:
                    //this means the node has been garbaged by the server
                    //reregister
                    if(!Reregister(pPoster))
                    {
                        bError = true;
                        NodeApi::Get().StopRun();
                    }
                    break;
            }
        }
        if(!bError)
        {   //still registered
            NodeApi::Get().UnregisterSimple();
        }
    }
}

static void UnregisterThread(EventPoster* pPoster)
{
    NodeApi::Get().UnregisterSimple();
}

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
m_query("query"),
m_pNodeApiPublisher(0),
m_pRegistrationBrowser(0),
m_pRegisterCurl(0),
m_nRegistrationStatus(REG_START),
m_bRun(false),
m_pPoster(0)
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
    if(m_pPoster)
    {
        delete m_pPoster;
    }
}



void NodeApi::Init(unsigned short nDiscoveryPort, unsigned short nConnectionPort, const string& sLabel, const string& sDescription)
{
    char sHost[256];
    gethostname(sHost, 256);

    set<string> setEndpoints;
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
                // Check if interface is en0 which is the wifi connection on the iPhone
                string sAddress(inet_ntoa(((sockaddr_in*)temp_addr->ifa_addr)->sin_addr));
                if(sAddress != "127.0.0.1")
                {
                    setEndpoints.insert(sAddress);
                    m_self.AddEndpoint(sAddress, nDiscoveryPort, false);
                }
            }
            temp_addr = temp_addr->ifa_next;
        }
    }
    // Free memory
    freeifaddrs(interfaces);

    if(setEndpoints.empty() == false)
    {
        stringstream sstr;
        sstr << "http://" << *setEndpoints.begin() << ":" << nDiscoveryPort;

        m_self.Init(sHost, sstr.str(), sLabel, sDescription);
    }

    m_nConnectionPort = nConnectionPort;
}

bool NodeApi::StartHttpServers()
{
    Log::Get(Log::DEBUG) << "Start Http Servers" << endl;
    m_self.StartServers();

    //start the connection servers
    MicroServer* pServer = new MicroServer();
    pServer->Init(m_nConnectionPort);
    m_mConnectionServers.insert(make_pair(m_nConnectionPort, pServer));


    Log::Get(Log::DEBUG) << "Start Http Servers: Done" << endl;
    return true;
}

void NodeApi::StopHttpServers()
{
    m_self.StopServers();
    for(map<unsigned short, MicroServer*>::iterator itServer = m_mConnectionServers.begin(); itServer != m_mConnectionServers.end(); ++itServer)
    {
        itServer->second->Stop();
        delete itServer->second;
    }
    m_mConnectionServers.clear();
}

bool NodeApi::StartmDNSServer()
{
    StopmDNSServer();
    set<endpoint>::const_iterator itEndpoint = m_self.GetEndpointsBegin();
    if(itEndpoint != m_self.GetEndpointsEnd())
    {
        m_pNodeApiPublisher = new ServicePublisher("nodeapi", "_nmos-node._tcp", itEndpoint->nPort);
        SetmDNSTxt(itEndpoint->bSecure);
    }


    return m_pNodeApiPublisher->Start();
}

void NodeApi::StopmDNSServer()
{
    if(m_pNodeApiPublisher)
    {
        m_pNodeApiPublisher->Stop();
        delete m_pNodeApiPublisher;
        m_pNodeApiPublisher = 0;
    }
}


bool NodeApi::StartServices(EventPoster* pPoster)
{
    m_pPoster = pPoster;
    m_pRegisterCurl = new CurlRegister(pPoster);
    return (StartmDNSServer() && StartHttpServers() && BrowseForRegistrationNode());
}

void NodeApi::StopServices()
{
    StopmDNSServer();
    StopHttpServers();
    StopRun();
}

int NodeApi::GetJson(string sPath, string& sReturn, unsigned short nPort)
{
    transform(sPath.begin(), sPath.end(), sPath.begin(), ::tolower);

    int nCode = 200;
    SplitString(m_vPath, sPath, '/');


    if(m_vPath.size() <= SZ_BASE)
    {
        Json::StyledWriter stw;
        Json::Value jsNode;
        jsNode.append("x-nmos/");
        sReturn = stw.write(jsNode);
    }
    else
    {
        if(m_vPath[NMOS] == "x-nmos")
        {   //check on nmos

            nCode = GetJsonNmos(sReturn, nPort);
        }
        else
        {
            Json::StyledWriter stw;
            sReturn = stw.write(GetJsonError(404, "Page not found"));
            nCode = 404;
        }
    }
    return nCode;
}

int NodeApi::GetJsonNmos(string& sReturn, unsigned short nPort)
{
    Json::StyledWriter stw;
    if(m_vPath.size() == SZ_NMOS)
    {
        Json::Value jsNode;
        if(nPort == m_nConnectionPort)
        {
            jsNode.append("connection/");
        }
        else
        {
            jsNode.append("node/");
        }
        sReturn = stw.write(jsNode);
        return 200;
    }
    else if(m_vPath[API_TYPE] == "node")
    {
        if(nPort == m_nConnectionPort)
        {
            GetJsonNmosConnectionApi(sReturn);
        }
        else
        {
            return GetJsonNmosNodeApi(sReturn);
        }
    }
    sReturn = stw.write(GetJsonError(404, "API not found"));
    return 404;
}

int NodeApi::GetJsonNmosNodeApi(string& sReturn)
{
    Json::StyledWriter stw;
    int nCode = 200;
    if(m_vPath.size() == SZ_API_TYPE)
    {
        sReturn = stw.write(m_self.JsonVersions());
    }
    else
    {

        if(m_self.IsVersionSupported(m_vPath[VERSION]))
        {
            if(m_vPath.size() == SZ_VERSION)
            {
                //check the version::

                Json::Value jsNode;
                jsNode.append("self/");
                jsNode.append("sources/");
                jsNode.append("flows/");
                jsNode.append("devices/");
                jsNode.append("senders/");
                jsNode.append("receivers/");
                sReturn = stw.write(jsNode);
            }
            else if(m_vPath[ENDPOINT] == "self")
            {
                sReturn  = stw.write(m_self.GetJson());
            }
            else if(m_vPath[ENDPOINT] == "sources")
            {
                sReturn  = stw.write(GetJsonSources());
            }
            else if(m_vPath[ENDPOINT] == "flows")
            {
                sReturn  = stw.write(GetJsonFlows());
            }
            else if(m_vPath[ENDPOINT] == "devices")
            {
                sReturn  = stw.write(GetJsonDevices());
            }
            else if(m_vPath[ENDPOINT] == "senders")
            {
                sReturn  = stw.write(GetJsonSenders());
            }
            else if(m_vPath[ENDPOINT] == "receivers")
            {
                sReturn  = stw.write(GetJsonReceivers());
            }
            else
            {
                nCode = 404;
                sReturn = stw.write(GetJsonError(404, "Endpoint not found"));
            }
        }
        else
        {
            nCode = 404;
            sReturn = stw.write(GetJsonError(404, "Version not supported"));
        }
    }
    return nCode;
}

Json::Value NodeApi::GetJsonSources()
{
    if(m_vPath.size() == SZ_ENDPOINT)
    {
        return m_sources.GetJson();
    }
    else
    {
        map<string, Resource*>::const_iterator itSource = m_sources.FindResource(m_vPath[RESOURCE]);
        if(itSource != m_sources.GetResourceEnd())
        {
            return itSource->second->GetJson();
        }
    }
    return GetJsonError();
}

Json::Value NodeApi::GetJsonDevices()
{
    if(m_vPath.size() == SZ_ENDPOINT)
    {
        return m_devices.GetJson();
    }
    else
    {
        map<string, Resource*>::const_iterator itDevice = m_devices.FindResource(m_vPath[RESOURCE]);
        if(itDevice != m_devices.GetResourceEnd())
        {
            return itDevice->second->GetJson();
        }
    }
    return GetJsonError();
}

Json::Value NodeApi::GetJsonFlows()
{
    if(m_vPath.size() == SZ_ENDPOINT)
    {
        return m_flows.GetJson();
    }
    else
    {
        map<string, Resource*>::const_iterator itFlow = m_flows.FindResource(m_vPath[RESOURCE]);
        if(itFlow != m_flows.GetResourceEnd())
        {
            return itFlow->second->GetJson();
        }
    }
    return GetJsonError();
}

Json::Value NodeApi::GetJsonReceivers()
{
    if(m_vPath.size() == SZ_ENDPOINT)
    {
        return m_receivers.GetJson();
    }
    else
    {
        map<string, Resource*>::const_iterator itReceiver = m_receivers.FindResource(m_vPath[RESOURCE]);
        if(itReceiver != m_receivers.GetResourceEnd())
        {
            return itReceiver->second->GetJson();
        }
    }
    return GetJsonError();
}

Json::Value NodeApi::GetJsonSenders()
{
    if(m_vPath.size() == SZ_ENDPOINT)
    {
        return m_senders.GetJson();
    }
    else
    {
        map<string, Resource*>::const_iterator itSender = m_senders.FindResource(m_vPath[RESOURCE]);
        if(itSender != m_senders.GetResourceEnd())
        {
            return itSender->second->GetJson();
        }
    }
    return GetJsonError();
}


int NodeApi::GetJsonNmosConnectionApi(string& sReturn)
{
    Json::StyledWriter stw;
    int nCode = 200;
    if(m_vPath.size() == SZ_API_TYPE)
    {
        sReturn = stw.write("v1.0");
    }
    else
    {

        if(m_self.IsVersionSupported(m_vPath[VERSION]))
        {
            if(m_vPath.size() == SZ_VERSION)
            {
                //check the version::
                Json::Value jsNode;
                jsNode.append("bulk/");
                jsNode.append("single/");
                sReturn = stw.write(jsNode);
            }
            else if(m_vPath.size() == SZC_DIRECTION)
            {
                Json::Value jsNode;
                jsNode.append("senders/");
                jsNode.append("receivers/");
                sReturn = stw.write(jsNode);
            }
            else if(m_vPath[SZC_DIRECTION] == "senders")
            {
            }
            else if(m_vPath[SZC_DIRECTION] == "receivers")
            {
            }
            else
            {
                nCode = 404;
                sReturn = stw.write(GetJsonError(404, "Endpoint not found"));
            }
        }
        else
        {
            nCode = 404;
            sReturn = stw.write(GetJsonError(404, "Version not supported"));
        }
    }
    return nCode;
}


Json::Value NodeApi::GetJsonError(unsigned long nCode, string sError)
{
    Json::Value jsError(Json::objectValue);
    jsError["code"] = (int)nCode;
    jsError["error"] = sError;
    jsError["debug"] = "null";
    return jsError;
}





int NodeApi::PutJson(string sPath, string sJson, string& sResponse, unsigned short nPort)
{
    //make sure path is correct
    transform(sPath.begin(), sPath.end(), sPath.begin(), ::tolower);

    Json::StyledWriter stw;

    int nCode = 202;
    SplitString(m_vPath, sPath, '/');
    if(m_vPath.size() <= SZ_ENDPOINT || m_vPath[ENDPOINT] != "receivers")
    {
        nCode = 400;
        sResponse = stw.write(GetJsonError(nCode, "Method not allowed here."));
    }
    else
    {
        //does the receiver exist?
        map<string, Resource*>::const_iterator itReceiver = m_receivers.FindResource(m_vPath[RESOURCE]);
        if(itReceiver == m_receivers.GetResourceEnd())
        {
            nCode = 404;
            sResponse = stw.write(GetJsonError(nCode, "Resource does not exist."));
        }
        else
        {
            Json::Value jsRequest;
            Json::Reader jsReader;
            if(jsReader.parse(sJson, jsRequest) == false)
            {
                nCode = 400;
                sResponse = stw.write(GetJsonError(nCode, "Request is ill defined."));
            }
            else
            {
                //TODO action the PUT request
            }
        }
    }
    return nCode;
}



Self& NodeApi::GetSelf()
{
    return m_self;
}

const ResourceHolder& NodeApi::GetSources()
{
    return m_sources;
}

const ResourceHolder& NodeApi::GetDevices()
{
    return m_devices;
}

const ResourceHolder& NodeApi::GetFlows()
{
    return m_flows;
}

const ResourceHolder& NodeApi::GetReceivers()
{
    return m_receivers;
}

const ResourceHolder& NodeApi::GetSenders()
{
    return m_senders;
}

bool NodeApi::Commit()
{
    m_mutex.lock();
    Log::Get() << "Node: Commit" << endl;
    bool bChange = m_self.Commit();
    bChange |= m_sources.Commit();
    bChange |= m_devices.Commit();
    bChange |= m_flows.Commit();
    bChange |= m_receivers.Commit();
    bChange |= m_senders.Commit();
    m_mutex.unlock();

    if(bChange && m_pNodeApiPublisher)
    {
        set<endpoint>::const_iterator itEndpoint = m_self.GetEndpointsBegin();
        if(itEndpoint != m_self.GetEndpointsEnd())
        {
            SetmDNSTxt(itEndpoint->bSecure);
        }
        m_pNodeApiPublisher->Modify();
    }

    return bChange;
}





void NodeApi::SplitString(vector<string>& vSplit, string str, char cSplit)
{
    vSplit.clear();

    istringstream f(str);
    string s;
    while (getline(f, s, cSplit))
    {
        vSplit.push_back(s);
    }
}



void NodeApi::SetmDNSTxt(bool bSecure)
{
    if(m_pNodeApiPublisher)
    {
        if(bSecure)
        {
            m_pNodeApiPublisher->AddTxt("api_proto", "https");
        }
        else
        {
            m_pNodeApiPublisher->AddTxt("api_proto", "http");
        }
        m_pNodeApiPublisher->AddTxt("api_ver", "v1.2");
        m_pNodeApiPublisher->AddTxt("ver_slf", to_string(m_self.GetDnsVersion()));
        m_pNodeApiPublisher->AddTxt("ver_src", to_string(m_sources.GetVersion()));
        m_pNodeApiPublisher->AddTxt("ver_flw", to_string(m_flows.GetVersion()));
        m_pNodeApiPublisher->AddTxt("ver_dvc", to_string(m_devices.GetVersion()));
        m_pNodeApiPublisher->AddTxt("ver_snd", to_string(m_senders.GetVersion()));
        m_pNodeApiPublisher->AddTxt("ver_rcv", to_string(m_receivers.GetVersion()));
    }
}


bool NodeApi::BrowseForRegistrationNode()
{
    if(m_pRegistrationBrowser != 0)
    {
        StopRegistrationBrowser();
    }

    m_pRegistrationBrowser = new ServiceBrowser(m_pPoster);
    set<string> setService;
    setService.insert("_nmos-registration._tcp");
    setService.insert("_nmos-query._tcp");
    return m_pRegistrationBrowser->StartBrowser(setService);

}


void NodeApi::StopRegistrationBrowser()
{
    if(m_pRegistrationBrowser)
    {
        delete m_pRegistrationBrowser;
        m_pRegistrationBrowser = 0;
    }
}


void NodeApi::RegisterThreaded()
{
    thread th(RegisterThread, m_pPoster);
    th.detach();
}

void NodeApi::UnregisterThreaded()
{
    thread th(UnregisterThread, m_pPoster);
    th.detach();
}

int NodeApi::RegisterSimple()
{
    m_nRegistrationStatus = REG_FAILED;
    if(FindRegistrationNode())
    {
        long nResponse = RegisterResource("node", m_self.GetJson());
        if(nResponse == 200)
        {   //Node already registered. Unregister and start again
            UnregisterSimple();
            RegisterSimple();
        }
        else if(nResponse == 201)
        {
            if(RegisterResources(m_devices) == 201 && RegisterResources(m_sources) == 201 && RegisterResources(m_flows) == 201 && RegisterResources(m_senders) == 201 && RegisterResources(m_receivers))
            {
                m_nRegistrationStatus = REG_DONE;
                m_bRun = true;
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

bool NodeApi::FindRegistrationNode()
{
    Log::Get(Log::INFO) << "NodeApi: Start Registration" << endl;
    map<string, dnsService*>::const_iterator itService = m_pRegistrationBrowser->FindService("_nmos-registration._tcp");
    if(itService != m_pRegistrationBrowser->GetServiceEnd())
    {
        Log::Get(Log::INFO) << "NodeApi: Register. Found nmos registration service." << endl;
        const dnsInstance* pInstance(0);
        string sApiVersion;
        for(list<dnsInstance*>::const_iterator itInstance = itService->second->lstInstances.begin(); itInstance != itService->second->lstInstances.end(); ++itInstance)
        {   //get the registration node with smallest priority number

            Log::Get(Log::INFO) << "NodeApi: Register. Found nmos registration node: " << (*itInstance)->sName << endl;
            //get top priority
            unsigned long nPriority(200);
            map<string, string>::const_iterator itPriority = (*itInstance)->mTxt.find("pri");
            map<string, string>::const_iterator itVersion = (*itInstance)->mTxt.find("api_ver");
            if(itPriority != (*itInstance)->mTxt.end() && itVersion != (*itInstance)->mTxt.end())
            {
                if(stoi(itPriority->second) < nPriority && itVersion->second.find("v1.2") != string::npos)
                {//for now only doing v1.2


                    pInstance = (*itInstance);
                    nPriority = stoi(itPriority->second);

                    Log::Get(Log::INFO) << "NodeApi: Register. Found nmos registration node with v1.2 and low priority: " << (*itInstance)->sName << " " << nPriority << endl;

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

            return true;
        }
    }
    m_sRegistrationNode.clear();
    Log::Get(Log::INFO) << "NodeApi: Register: No nmos registration nodes found. Go peer-to-peer" << endl;
    m_nRegistrationStatus = REG_FAILED;

    return false;
}


long NodeApi::RegisterResources(ResourceHolder& holder)
{
    for(map<string, Resource*>::const_iterator itResource = holder.GetResourceBegin(); itResource != holder.GetResourceEnd(); ++itResource)
    {
        Log::Get(Log::INFO) << "NodeApi: Register. " << holder.GetType() << " : " << itResource->first << endl;
        long nResult = RegisterResource(holder.GetType(), itResource->second->GetJson());
        if(nResult != 201)
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
        Log::Get(Log::INFO) << "RegisterApi: returned [" << nResponse << "] " << sResponse << endl;

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
        Log::Get(Log::INFO) << "RegisterApi: returned [" << nResponse << "] " << sResponse << endl;
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


void NodeApi::UnregisterResources(ResourceHolder& holder)
{
    for(map<string, Resource*>::const_iterator itResource = holder.GetResourceBegin(); itResource != holder.GetResourceEnd(); ++itResource)
    {
        Log::Get(Log::INFO) << "NodeApi: Unregister. " << holder.GetType() << " : " << itResource->first << endl;
        UnregisterResource(holder.GetType()+"s", itResource->first);
    }
}


bool NodeApi::UnregisterResource(const string& sType, const std::string& sId)
{
    if(m_pRegisterCurl)
    {
        string sResponse;
        long nResponse = m_pRegisterCurl->Delete(m_sRegistrationNode+"/resource", sType, sId, sResponse);
        Log::Get(Log::INFO) << "RegisterApi: returned [" << nResponse << "] " << sResponse << endl;
        return true;
    }
    return false;
}


bool NodeApi::FindQueryNode()
{
    if(m_sQueryNode.empty())
    {
        map<string, dnsService*>::const_iterator itService = m_pRegistrationBrowser->FindService("_nmos-query._tcp");
        if(itService != m_pRegistrationBrowser->GetServiceEnd())
        {
            Log::Get(Log::INFO) << "NodeApi: Query. Found nmos query service." << endl;
            const dnsInstance* pInstance(0);
            string sApiVersion;
            for(list<dnsInstance*>::const_iterator itInstance = itService->second->lstInstances.begin(); itInstance != itService->second->lstInstances.end(); ++itInstance)
            {   //get the registration node with smallest priority number

                Log::Get(Log::INFO) << "NodeApi: Query. Found nmos query node: " << (*itInstance)->sName << endl;
                //get top priority
                unsigned long nPriority(200);
                map<string, string>::const_iterator itPriority = (*itInstance)->mTxt.find("pri");
                map<string, string>::const_iterator itVersion = (*itInstance)->mTxt.find("api_ver");
                if(itPriority != (*itInstance)->mTxt.end() && itVersion != (*itInstance)->mTxt.end())
                {
                    if(stoi(itPriority->second) < nPriority && itVersion->second.find("v1.2") != string::npos)
                    {//for now only doing v1.2

                        pInstance = (*itInstance);
                        nPriority = stoi(itPriority->second);

                        Log::Get(Log::INFO) << "NodeApi: Query. Found nmos query node with v1.2 and low priority: " << (*itInstance)->sName << " " << nPriority << endl;

                        //vector<string> vApi;
                        //SplitString(vApi, itVersion->second, ',');
                    }
                }
            }

            if(pInstance)
            {
                //build the registration url
                stringstream ssUrl;
                ssUrl <<  pInstance->sHostIP << ":" << pInstance->nPort << "/x-nmos/query/v1.2";
                m_sQueryNode = ssUrl.str();
                return true;
            }
            return false;
        }
    }
    return true;
}

const ResourceHolder& NodeApi::GetQueryResults()
{
    Log::Get() << "GetQueryResults: " << m_query.GetResourceCount() << " " << (int)&m_query << std::endl;
    return m_query;
}

bool NodeApi::Query(NodeApi::enumResource eResource, const std::string& sQuery)
{
    m_query.RemoveAllResources();
    m_query.SetType(STR_RESOURCE[eResource]);

    if(m_pRegisterCurl && FindQueryNode())
    {
        m_pRegisterCurl->Query(m_sQueryNode, eResource, sQuery, &m_query, CURL_QUERY);
    }
}


bool NodeApi::IsRunning()
{
    return m_bRun;
}

void NodeApi::StopRun()
{
    m_mutex.lock();
    m_bRun = false;
    m_mutex.unlock();
}




bool NodeApi::AddDevice(Device* pResource)
{
    //make sure the node id agress
    if(pResource->GetNodeId() == m_self.GetId())
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

bool NodeApi::AddSource(Source* pResource)
{
    if(m_devices.FindResource(pResource->GetDeviceId()) != m_devices.GetResourceEnd())
    {
        m_sources.AddResource(pResource);
        return true;
    }
    return false;
}

bool NodeApi::AddFlow(Flow* pResource)
{
    if(m_devices.FindResource(pResource->GetDeviceId()) != m_devices.GetResourceEnd() && m_sources.FindResource(pResource->GetSourceId()) != m_sources.GetResourceEnd())
    {
        m_flows.AddResource(pResource);
        return true;
    }
    return false;
}

bool NodeApi::AddReceiver(Receiver* pResource)
{
    if(m_devices.FindResource(pResource->GetDeviceId()) != m_devices.GetResourceEnd() && m_flows.FindResource(pResource->GetFlowId()) != m_flows.GetResourceEnd())
    {
        m_receivers.AddResource(pResource);
        return true;
    }
    return false;
}

bool NodeApi::AddSender(Sender* pResource)
{
    if(m_devices.FindResource(pResource->GetDeviceId()) != m_devices.GetResourceEnd() && m_flows.FindResource(pResource->GetFlowId()) != m_flows.GetResourceEnd())
    {
        m_senders.AddResource(pResource);
        return true;
    }
    return false;
}
