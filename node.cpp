#include "node.h"
#include <iostream>
#include <sstream>
#include "microserver.h"
#include <algorithm>
#include "avahipublisher.h"


Node& Node::Get()
{
    static Node aNode;
    return aNode;
}

Node::Node() : m_pNodeApi(0)
{
}

Node::~Node()
{
    StopHttpServer();
    StopmDNSServer();
}



void Node::Init(std::string sHostname, std::string sUrl, std::string sLabel, std::string sDescription)
{
    m_self = Self(sHostname, sUrl, sLabel, sDescription);
}

bool Node::StartHttpServer(unsigned short nPort)
{
    StopHttpServer();

    return MicroServer::Get().Init(nPort);
}

void Node::StopHttpServer()
{
    MicroServer::Get().Stop();
}

bool Node::StartmDNSServer(unsigned short nPort)
{
    StopmDNSServer();
    m_pNodeApi = new ServicePublisher("nodeapi", "_nmos-node._tcp", nPort);

    SetmDNSTxt();


    return m_pNodeApi->Start();
}

void Node::StopmDNSServer()
{
    if(m_pNodeApi)
    {
        m_pNodeApi->Stop();
        delete m_pNodeApi;
        m_pNodeApi = 0;
    }
}


bool Node::StartServices(unsigned short nPort)
{
    return (StartmDNSServer(nPort) && StartHttpServer(nPort));
}

void Node::StopServices()
{
    StopmDNSServer();
    StopHttpServer();
}

int Node::GetJson(std::string sPath, std::string& sReturn)
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

int Node::GetJsonNmos(std::string& sReturn)
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

int Node::GetJsonNmosNodeApi(std::string& sReturn)
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
                sReturn  = stw.write(m_self.ToJson());
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

Json::Value Node::GetJsonSources()
{
    if(m_vPath.size() == SZ_ENDPOINT)
    {
        return m_sources.ToJson();
    }
    else
    {
        std::map<std::string, Source*>::const_iterator itSource = m_sources.FindSource(m_vPath[RESOURCE]);
        if(itSource != m_sources.GetSourceEnd())
        {
            return itSource->second->ToJson();
        }
    }
    return GetJsonError();
}

Json::Value Node::GetJsonDevices()
{
    if(m_vPath.size() == SZ_ENDPOINT)
    {
        return m_devices.ToJson();
    }
    else
    {
        std::map<std::string, Device*>::const_iterator itDevice = m_devices.FindDevice(m_vPath[RESOURCE]);
        if(itDevice != m_devices.GetDeviceEnd())
        {
            return itDevice->second->ToJson();
        }
    }
    return GetJsonError();
}

Json::Value Node::GetJsonFlows()
{
    if(m_vPath.size() == SZ_ENDPOINT)
    {
        return m_flows.ToJson();
    }
    else
    {
        std::map<std::string, Flow*>::const_iterator itFlow = m_flows.FindFlow(m_vPath[RESOURCE]);
        if(itFlow != m_flows.GetFlowEnd())
        {
            return itFlow->second->ToJson();
        }
    }
    return GetJsonError();
}

Json::Value Node::GetJsonReceivers()
{
    if(m_vPath.size() == SZ_ENDPOINT)
    {
        return m_receivers.ToJson();
    }
    else
    {
        std::map<std::string, Receiver*>::const_iterator itReceiver = m_receivers.FindReceiver(m_vPath[RESOURCE]);
        if(itReceiver != m_receivers.GetReceiverEnd())
        {
            return itReceiver->second->ToJson();
        }
    }
    return GetJsonError();
}

Json::Value Node::GetJsonSenders()
{
    if(m_vPath.size() == SZ_ENDPOINT)
    {
        return m_senders.ToJson();
    }
    else
    {
        std::map<std::string, Sender*>::const_iterator itSender = m_senders.FindSender(m_vPath[RESOURCE]);
        if(itSender != m_senders.GetSenderEnd())
        {
            return itSender->second->ToJson();
        }
    }
    return GetJsonError();
}


Json::Value Node::GetJsonError(unsigned long nCode, std::string sError)
{
    Json::Value jsError(Json::objectValue);
    jsError["code"] = (int)nCode;
    jsError["error"] = sError;
    jsError["debug"] = "null";
    return jsError;
}


int Node::PutJson(std::string sPath, std::string sJson, std::string& sResponse)
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
        std::map<std::string, Receiver*>::const_iterator itReceiver = m_receivers.FindReceiver(m_vPath[RESOURCE]);
        if(itReceiver == m_receivers.GetReceiverEnd())
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



Self& Node::GetSelf()
{
    return m_self;
}

Sources& Node::GetSources()
{
    return m_sources;
}

Devices& Node::GetDevices()
{
    return m_devices;
}

Flows& Node::GetFlows()
{
    return m_flows;
}

Receivers& Node::GetReceivers()
{
    return m_receivers;
}

Senders& Node::GetSenders()
{
    return m_senders;
}





void Node::SplitPath(std::string str, char cSplit)
{
    m_vPath.clear();

    std::istringstream f(str);
    std::string s;
    while (getline(f, s, cSplit))
    {
        m_vPath.push_back(s);
    }
}


bool Node::ResourceUpdated()
{
    if(m_pNodeApi)
    {
        SetmDNSTxt();
        m_pNodeApi->Modify();
    }
}

void Node::SetmDNSTxt()
{
    if(m_pNodeApi)
    {
        m_pNodeApi->AddTxt("api_proto", "http");
        m_pNodeApi->AddTxt("api_ver", "v1.2");
        m_pNodeApi->AddTxt("ver_slf", std::to_string(m_self.GetVersion()));
        m_pNodeApi->AddTxt("ver_src", std::to_string(m_sources.GetVersion()));
        m_pNodeApi->AddTxt("ver_flw", std::to_string(m_flows.GetVersion()));
        m_pNodeApi->AddTxt("ver_dvc", std::to_string(m_devices.GetVersion()));
        m_pNodeApi->AddTxt("ver_snd", std::to_string(m_senders.GetVersion()));
        m_pNodeApi->AddTxt("ver_rcv", std::to_string(m_receivers.GetVersion()));
    }
}
