#include "nodeapi.h"
#include <iostream>
#include <sstream>
#include "microserver.h"
#include <algorithm>
#include "avahipublisher.h"
#include "avahibrowser.h"
#include "log.h"
#include "curlregister.h"
#include <thread>

using namespace std;


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
m_pRegistrationBrowser(0),
m_pRegisterCurl(0),
m_nRegisterNext(REG_START)
{
}

NodeApi::~NodeApi()
{
    StopHttpServer();
    StopmDNSServer();
    StopRegistrationBrowser();
    if(m_pRegisterCurl)
    {
        delete m_pRegisterCurl;
    }
}



void NodeApi::Init(string sHostname, string sUrl, string sLabel, string sDescription)
{
    m_self = Self(sHostname, sUrl, sLabel, sDescription);
}

bool NodeApi::StartHttpServer(unsigned short nPort)
{
    StopHttpServer();

    return MicroServer::Get().Init(nPort);
}

void NodeApi::StopHttpServer()
{
    MicroServer::Get().Stop();
}

bool NodeApi::StartmDNSServer(unsigned short nPort)
{
    StopmDNSServer();
    m_pNodeApiPublisher = new ServicePublisher("nodeapi", "_nmos-node._tcp", nPort);

    SetmDNSTxt();


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


bool NodeApi::StartServices(unsigned short nPort,ServiceBrowserEvent* pPoster, CurlEvent* pCurlPoster)
{
    m_pRegisterCurl = new CurlRegister(pCurlPoster);
    return (StartmDNSServer(nPort) && StartHttpServer(nPort) && BrowseForRegistrationNode(pPoster));
}

void NodeApi::StopServices()
{
    StopmDNSServer();
    StopHttpServer();
}

int NodeApi::GetJson(string sPath, string& sReturn)
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

            nCode = GetJsonNmos(sReturn);
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

int NodeApi::GetJsonNmos(string& sReturn)
{
    Json::StyledWriter stw;
    if(m_vPath.size() == SZ_NMOS)
    {
        Json::Value jsNode;
        jsNode.append("node/");
        sReturn = stw.write(jsNode);
        return 200;
    }
    else if(m_vPath[API_TYPE] == "node")
    {
        return GetJsonNmosNodeApi(sReturn);
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


Json::Value NodeApi::GetJsonError(unsigned long nCode, string sError)
{
    Json::Value jsError(Json::objectValue);
    jsError["code"] = (int)nCode;
    jsError["error"] = sError;
    jsError["debug"] = "null";
    return jsError;
}


int NodeApi::PutJson(string sPath, string sJson, string& sResponse)
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

ResourceHolder& NodeApi::GetSources()
{
    return m_sources;
}

ResourceHolder& NodeApi::GetDevices()
{
    return m_devices;
}

ResourceHolder& NodeApi::GetFlows()
{
    return m_flows;
}

ResourceHolder& NodeApi::GetReceivers()
{
    return m_receivers;
}

ResourceHolder& NodeApi::GetSenders()
{
    return m_senders;
}

bool NodeApi::Commit()
{
    Log::Get() << "Node: Commit" << endl;
    bool bChange = m_self.Commit();
    bChange |= m_sources.Commit();
    bChange |= m_devices.Commit();
    bChange |= m_flows.Commit();
    bChange |= m_receivers.Commit();
    bChange |= m_senders.Commit();
    if(bChange && m_pNodeApiPublisher)
    {
        SetmDNSTxt();
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



void NodeApi::SetmDNSTxt()
{
    if(m_pNodeApiPublisher)
    {
        m_pNodeApiPublisher->AddTxt("api_proto", "http");
        m_pNodeApiPublisher->AddTxt("api_ver", "v1.2");
        m_pNodeApiPublisher->AddTxt("ver_slf", to_string(m_self.GetDnsVersion()));
        m_pNodeApiPublisher->AddTxt("ver_src", to_string(m_sources.GetVersion()));
        m_pNodeApiPublisher->AddTxt("ver_flw", to_string(m_flows.GetVersion()));
        m_pNodeApiPublisher->AddTxt("ver_dvc", to_string(m_devices.GetVersion()));
        m_pNodeApiPublisher->AddTxt("ver_snd", to_string(m_senders.GetVersion()));
        m_pNodeApiPublisher->AddTxt("ver_rcv", to_string(m_receivers.GetVersion()));
    }
}


bool NodeApi::BrowseForRegistrationNode(ServiceBrowserEvent* pPoster)
{
    if(m_pRegistrationBrowser == 0)
    {
        m_pRegistrationBrowser = new ServiceBrowser(pPoster);
        set<string> setService;
        setService.insert("_nmos-registration._tcp");
        return m_pRegistrationBrowser->StartBrowser(setService);
    }
    return false;
}


void NodeApi::StopRegistrationBrowser()
{
    if(m_pRegistrationBrowser)
    {
        delete m_pRegistrationBrowser;
        m_pRegistrationBrowser = 0;
//        m_pRegistrationBrowser->Stop();
    }
}

int NodeApi::Register()
{
    switch(m_nRegisterNext)
    {
        case REG_START:
            StartRegistration();
            break;
        case REG_DEVICES:
            RegisterResources(m_devices, m_sources);
            break;
        case REG_SOURCES:
            RegisterResources(m_sources, m_flows);
            break;
        case REG_FLOWS:
            RegisterResources(m_flows, m_senders);
            break;
        case REG_SENDERS:
            RegisterResources(m_senders, m_receivers);
            break;
        case REG_RECEIVERS:
            RegisterResources(m_receivers, m_devices);
            break;


    }
    return m_nRegisterNext;
}

bool NodeApi::StartRegistration()
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

            m_itRegisterNext = m_devices.GetResourceBegin();
            m_nRegisterNext = REG_DEVICES;
            RegisterSelf();

            return true;
        }
    }
    m_sRegistrationNode.clear();
    Log::Get(Log::INFO) << "NodeApi: Register: No nmos registration nodes found. Go peer-to-peer" << endl;
    m_nRegisterNext = REG_FAILED;

    return false;
}


void NodeApi::RegisterResources(ResourceHolder& holder, ResourceHolder& next)
{
    if(m_itRegisterNext != holder.GetResourceEnd())
    {
        Log::Get(Log::INFO) << "NodeApi: Register. " << holder.GetType() << " : " << m_itRegisterNext->first << endl;
        RegisterResource(holder.GetType(), m_itRegisterNext->second->GetJson());
        ++m_itRegisterNext;

    }
    else
    {
        m_nRegisterNext++;
        m_itRegisterNext = next.GetResourceBegin();
        Register();
    }

}


bool NodeApi::RegisterResource(const string& sType, const Json::Value& json)
{
    Json::Value jsonRegister;
    jsonRegister["type"] = sType;
    jsonRegister["data"] = json;
    Json::StyledWriter stw;
    string sPost(stw.write(jsonRegister));

    string sResponse;

    if(m_pRegisterCurl)
    {
        m_pRegisterCurl->Post(m_sRegistrationNode+"/resource", sPost);
        return true;
    }
    return false;
}

bool NodeApi::RegistrationHeartbeat()
{
    string sResponse;
    if(m_pRegisterCurl)
    {
        m_pRegisterCurl->Post(m_sRegistrationNode+"/health/nodes/"+m_self.GetId(), "");
    }
    return true;
}


bool NodeApi::RegisterSelf()
{
    Log::Get(Log::INFO) << "NodeApi: Register. RegisterSelf" << endl;
    return RegisterResource("node", m_self.GetJson());
}

