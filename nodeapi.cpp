#include "nodeapi.h"
#include <iostream>
#include <sstream>
#include "microserver.h"
#include <algorithm>
#include "avahipublisher.h"


NodeApi& NodeApi::Get()
{
    static NodeApi aNode;
    return aNode;
}

NodeApi::NodeApi() : m_pNodeApiPublisher(0)
{
}

NodeApi::~NodeApi()
{
    StopHttpServer();
    StopmDNSServer();
}



void NodeApi::Init(std::string sHostname, std::string sUrl, std::string sLabel, std::string sDescription)
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


bool NodeApi::StartServices(unsigned short nPort)
{
    return (StartmDNSServer(nPort) && StartHttpServer(nPort));
}

void NodeApi::StopServices()
{
    StopmDNSServer();
    StopHttpServer();
}

int NodeApi::GetJson(std::string sPath, std::string& sReturn)
{
    std::transform(sPath.begin(), sPath.end(), sPath.begin(), ::tolower);

    int nCode = 200;
    SplitPath(sPath, '/');


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

int NodeApi::GetJsonNmos(std::string& sReturn)
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

int NodeApi::GetJsonNmosNodeApi(std::string& sReturn)
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
        std::map<std::string, Resource*>::const_iterator itSource = m_sources.FindResource(m_vPath[RESOURCE]);
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
        std::map<std::string, Resource*>::const_iterator itDevice = m_devices.FindResource(m_vPath[RESOURCE]);
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
        std::map<std::string, Resource*>::const_iterator itFlow = m_flows.FindResource(m_vPath[RESOURCE]);
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
        std::map<std::string, Resource*>::const_iterator itReceiver = m_receivers.FindResource(m_vPath[RESOURCE]);
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
        std::map<std::string, Resource*>::const_iterator itSender = m_senders.FindResource(m_vPath[RESOURCE]);
        if(itSender != m_senders.GetResourceEnd())
        {
            return itSender->second->GetJson();
        }
    }
    return GetJsonError();
}


Json::Value NodeApi::GetJsonError(unsigned long nCode, std::string sError)
{
    Json::Value jsError(Json::objectValue);
    jsError["code"] = (int)nCode;
    jsError["error"] = sError;
    jsError["debug"] = "null";
    return jsError;
}


int NodeApi::PutJson(std::string sPath, std::string sJson, std::string& sResponse)
{
    //make sure path is correct
    std::transform(sPath.begin(), sPath.end(), sPath.begin(), ::tolower);

    Json::StyledWriter stw;

    int nCode = 202;
    SplitPath(sPath, '/');
    if(m_vPath.size() <= SZ_ENDPOINT || m_vPath[ENDPOINT] != "receivers")
    {
        nCode = 400;
        sResponse = stw.write(GetJsonError(nCode, "Method not allowed here."));
    }
    else
    {
        //does the receiver exist?
        std::map<std::string, Resource*>::const_iterator itReceiver = m_receivers.FindResource(m_vPath[RESOURCE]);
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
    std::cout << "Node: Commit" << std::endl;
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





void NodeApi::SplitPath(std::string str, char cSplit)
{
    m_vPath.clear();

    std::istringstream f(str);
    std::string s;
    while (getline(f, s, cSplit))
    {
        m_vPath.push_back(s);
    }
}



void NodeApi::SetmDNSTxt()
{
    if(m_pNodeApiPublisher)
    {
        m_pNodeApiPublisher->AddTxt("api_proto", "http");
        m_pNodeApiPublisher->AddTxt("api_ver", "v1.2");
        m_pNodeApiPublisher->AddTxt("ver_slf", std::to_string(m_self.GetDnsVersion()));
        m_pNodeApiPublisher->AddTxt("ver_src", std::to_string(m_sources.GetVersion()));
        m_pNodeApiPublisher->AddTxt("ver_flw", std::to_string(m_flows.GetVersion()));
        m_pNodeApiPublisher->AddTxt("ver_dvc", std::to_string(m_devices.GetVersion()));
        m_pNodeApiPublisher->AddTxt("ver_snd", std::to_string(m_senders.GetVersion()));
        m_pNodeApiPublisher->AddTxt("ver_rcv", std::to_string(m_receivers.GetVersion()));
    }
}
