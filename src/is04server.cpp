#include "is04server.h"
#include "nodeapiprivate.h"
#include <sstream>
#include <algorithm>
#include <iostream>
#include "self.h"
#include "device.h"
#include "source.h"
#include "flow.h"
#include "receiverbase.h"
#include "senderbase.h"
#include "log.h"
#include "eventposter.h"
#include "utils.h"
#include "curlregister.h"
#include "utils.h"


using namespace std::placeholders;
using namespace pml::nmos;

const std::string IS04Server::ROOT = "/x-nmos/node/";
const std::string IS04Server::SELF = "/self";
const std::string IS04Server::SOURCES = "/sources";
const std::string IS04Server::FLOWS = "/flows";
const std::string IS04Server::DEVICES = "/devices";
const std::string IS04Server::SENDERS = "/senders";
const std::string IS04Server::RECEIVERS = "/receivers";

IS04Server::IS04Server(std::shared_ptr<RestGoose> pServer, const ApiVersion& version, std::shared_ptr<EventPoster> pPoster, NodeApiPrivate& api) :
    NmosServer(pServer, version,pPoster, api)
{
    AddBaseEndpoints();
}

IS04Server::~IS04Server()
{

}


void IS04Server::AddBaseEndpoints()
{
    m_pServer->AddEndpoint(endpoint(RestGoose::GET, url("/x-nmos/node")), std::bind(&IS04Server::GetNmosRoot, this, _1,_2,_3,_4));
    m_pServer->AddEndpoint(endpoint(RestGoose::GET, url(ROOT+m_version.GetVersionAsString())), std::bind(&IS04Server::GetNmosVersion, this, _1,_2,_3,_4));
    m_pServer->AddEndpoint(endpoint(RestGoose::GET, url(ROOT+m_version.GetVersionAsString()+SELF)), std::bind(&IS04Server::GetNmosNode, this, _1,_2,_3,_4));
    m_pServer->AddEndpoint(endpoint(RestGoose::GET, url(ROOT+m_version.GetVersionAsString()+SOURCES)), std::bind(&IS04Server::GetNmosSources, this, _1,_2,_3,_4));
    m_pServer->AddEndpoint(endpoint(RestGoose::GET, url(ROOT+m_version.GetVersionAsString()+FLOWS)), std::bind(&IS04Server::GetNmosFlows, this, _1,_2,_3,_4));
    m_pServer->AddEndpoint(endpoint(RestGoose::GET, url(ROOT+m_version.GetVersionAsString()+DEVICES)), std::bind(&IS04Server::GetNmosDevices, this, _1,_2,_3,_4));
    m_pServer->AddEndpoint(endpoint(RestGoose::GET, url(ROOT+m_version.GetVersionAsString()+SENDERS)), std::bind(&IS04Server::GetNmosSenders, this, _1,_2,_3,_4));
    m_pServer->AddEndpoint(endpoint(RestGoose::GET, url(ROOT+m_version.GetVersionAsString()+RECEIVERS)), std::bind(&IS04Server::GetNmosReceivers, this, _1,_2,_3,_4));
}

void IS04Server::AddSourceEndpoint(const std::string& sId)
{
    m_pServer->AddEndpoint(endpoint(RestGoose::GET, url(ROOT+m_version.GetVersionAsString()+SOURCES+"/"+sId)), std::bind(&IS04Server::GetNmosSource,this,  _1,_2,_3,_4));
}

void IS04Server::AddFlowEndpoint(const std::string& sId)
{
    m_pServer->AddEndpoint(endpoint(RestGoose::GET, url(ROOT+m_version.GetVersionAsString()+FLOWS+"/"+sId)), std::bind(&IS04Server::GetNmosFlow, this, _1,_2,_3,_4));
}

void IS04Server::AddDeviceEndpoint(const std::string& sId)
{
    m_pServer->AddEndpoint(endpoint(RestGoose::GET, url(ROOT+m_version.GetVersionAsString()+DEVICES+"/"+sId)), std::bind(&IS04Server::GetNmosDevice, this, _1,_2,_3,_4));
}

void IS04Server::AddSenderEndpoint(const std::string& sId)
{
    m_pServer->AddEndpoint(endpoint(RestGoose::GET, url(ROOT+m_version.GetVersionAsString()+SENDERS+"/"+sId)), std::bind(&IS04Server::GetNmosSender, this, _1,_2,_3,_4));
}

void IS04Server::AddReceiverEndpoint(const std::string& sId)
{
    m_pServer->AddEndpoint(endpoint(RestGoose::GET, url(ROOT+m_version.GetVersionAsString()+RECEIVERS+"/"+sId)), std::bind(&IS04Server::GetNmosReceiver, this, _1,_2,_3,_4));
    m_pServer->AddEndpoint(endpoint(RestGoose::PUT, url(ROOT+m_version.GetVersionAsString()+RECEIVERS+"/"+sId+"/target")), std::bind(&IS04Server::PutNmosReceiver, this, _1,_2,_3,_4));
}

void IS04Server::RemoveSenderEndpoint(const std::string& sId)
{
    m_pServer->DeleteEndpoint(endpoint(RestGoose::GET, url(ROOT+m_version.GetVersionAsString()+SENDERS+"/"+sId)));
}

void IS04Server::RemoveReceiverEndpoint(const std::string& sId)
{
    m_pServer->DeleteEndpoint(endpoint(RestGoose::GET, url(ROOT+m_version.GetVersionAsString()+RECEIVERS+"/"+sId)));
    m_pServer->DeleteEndpoint(endpoint(RestGoose::PUT, url(ROOT+m_version.GetVersionAsString()+RECEIVERS+"/"+sId+"/target")));
}


response IS04Server::GetNmosRoot(const query& theQuery, const postData& theData, const url& theUrl, const userName& theUser)
{
    response resp;
    resp.jsonData = m_api.GetSelf().JsonVersions();
    return resp;
}

response IS04Server::GetNmosVersion(const query& theQuery, const postData& theData, const url& theUrl, const userName& theUser)
{
    response resp;
    resp.jsonData.append("self/");
    resp.jsonData.append("sources/");
    resp.jsonData.append("flows/");
    resp.jsonData.append("devices/");
    resp.jsonData.append("senders/");
    resp.jsonData.append("receivers/");

    return resp;
}

response IS04Server::GetNmosNode(const query& theQuery, const postData& theData, const url& theUrl, const userName& theUser)
{
    response resp;
    resp.jsonData = m_api.GetSelf().GetJson(m_version);
    return resp;
}

response IS04Server::GetNmosSources(const query& theQuery, const postData& theData, const url& theUrl, const userName& theUser)
{
    response resp;
    resp.jsonData = m_api.GetSources().GetJson(m_version);
    return resp;
}

response IS04Server::GetNmosFlows(const query& theQuery, const postData& theData, const url& theUrl, const userName& theUser)
{
    response resp;
    resp.jsonData = m_api.GetFlows().GetJson(m_version);
    return resp;
}

response IS04Server::GetNmosDevices(const query& theQuery, const postData& theData, const url& theUrl, const userName& theUser)
{
    response resp;
    resp.jsonData = m_api.GetDevices().GetJson(m_version);
    return resp;
}

response IS04Server::GetNmosSenders(const query& theQuery, const postData& theData, const url& theUrl, const userName& theUser)
{
    response resp;
    resp.jsonData = m_api.GetSenders().GetJson(m_version);
    return resp;
}

response IS04Server::GetNmosReceivers(const query& theQuery, const postData& theData, const url& theUrl, const userName& theUser)
{
    response resp;
    resp.jsonData = m_api.GetReceivers().GetJson(m_version);
    return resp;
}


response IS04Server::GetNmosSource(const query& theQuery, const postData& theData, const url& theUrl, const userName& theUser)
{
    response resp;
    auto vPath = SplitUrl(theUrl);
    auto itResource = m_api.GetSources().FindNmosResource(vPath[RESOURCE]);
    if(itResource != m_api.GetSources().GetResourceEnd())
    {
        resp.jsonData =  itResource->second->GetJson(m_version);
    }
    else
    {
        resp = JsonError(404, "Source "+vPath[RESOURCE]+" does not exist", theUrl.Get());
    }
    return resp;
}

response IS04Server::GetNmosFlow(const query& theQuery, const postData& theData, const url& theUrl, const userName& theUser)
{
    response resp;
    auto vPath = SplitUrl(theUrl);
    auto itResource = m_api.GetFlows().FindNmosResource(vPath[RESOURCE]);
    if(itResource != m_api.GetFlows().GetResourceEnd())
    {
        resp.jsonData =  itResource->second->GetJson(m_version);
    }
    else
    {
        resp = JsonError(404, "Flow "+vPath[RESOURCE]+" does not exist", theUrl.Get());
    }
    return resp;
}

response IS04Server::GetNmosDevice(const query& theQuery, const postData& theData, const url& theUrl, const userName& theUser)
{
    response resp;
    auto vPath = SplitUrl(theUrl);
    auto itResource = m_api.GetDevices().FindNmosResource(vPath[RESOURCE]);
    if(itResource != m_api.GetDevices().GetResourceEnd())
    {
        resp.jsonData =  itResource->second->GetJson(m_version);
    }
    else
    {
        resp = JsonError(404, "Device "+vPath[RESOURCE]+" does not exist", theUrl.Get());
    }
    return resp;
}

response IS04Server::GetNmosSender(const query& theQuery, const postData& theData, const url& theUrl, const userName& theUser)
{
    response resp;
    auto vPath = SplitUrl(theUrl);
    auto itResource = m_api.GetSenders().FindNmosResource(vPath[RESOURCE]);
    if(itResource != m_api.GetSenders().GetResourceEnd())
    {
        resp.jsonData =  itResource->second->GetJson(m_version);
    }
    else
    {
        resp = JsonError(404, "Sender "+vPath[RESOURCE]+" does not exist", theUrl.Get());
    }
    return resp;
}

response IS04Server::GetNmosReceiver(const query& theQuery, const postData& theData, const url& theUrl, const userName& theUser)
{
    response resp;
    auto vPath = SplitUrl(theUrl);
    auto itResource = m_api.GetReceivers().FindNmosResource(vPath[RESOURCE]);
    if(itResource != m_api.GetReceivers().GetResourceEnd())
    {
        resp.jsonData =  itResource->second->GetJson(m_version);
    }
    else
    {
        resp = JsonError(404, "Receiver "+vPath[RESOURCE]+" does not exist", theUrl.Get());
    }
    return resp;
}





response IS04Server::PutNmosReceiver(const query& theQuery, const postData& theData, const url& theUrl, const userName& theUser)
{
    pmlLog(pml::LOG_DEBUG) << "PutNmosReceiver";
    response resp;
    auto vPath = SplitUrl(theUrl);
    //does the receiver exist?
    auto pReceiver  = m_api.GetReceiver(vPath[RESOURCE]);
    if(!pReceiver)
    {
        resp = JsonError(404, "Receiver "+vPath[RESOURCE]+" does not exist", theUrl.Get());
    }
    else
    {
        Json::Value jsRequest = ConvertToJson(theData.Get());
        //does the sender have a manifest??
        std::string sSdp("");
        std::string sSenderId("");
        std::shared_ptr<Sender> pRemoteSender(0);
        //Have we been sent a sender??
        if(jsRequest.isObject() && jsRequest["id"].isString())
        {
            //@todo PUT
//            pRemoteSender = std::make_shared<Sender>();
//            pRemoteSender->UpdateFromJson(jsRequest);
//
//            sSenderId = pRemoteSender->GetId();
//            if(pRemoteSender->GetManifestHref().empty() == false)
//            {
//                auto curlresp = CurlRegister::Get(pRemoteSender->GetManifestHref());
//                if(curlresp.nCode == 200)
//                {
//                    sSdp = curlresp.sResponse;
//                }
//            }
        }
        if(m_pPoster)
        {
            m_pServer->PrimeWait();
            //Tell the main thread to connect
            m_pPoster->_Target(vPath[RESOURCE], sSdp, m_pServer->GetPort());
            //Pause the HTTP thread
            m_pServer->Wait();

            if(m_pServer->IsOk())
            {   //this means the main thread has connected the receiver to the sender
                resp.nHttpCode = 202;
                m_api.SetSender(pReceiver, sSenderId, sSdp, m_pServer->GetSignalData());


                if(pRemoteSender)
                {
                    resp.jsonData = pRemoteSender->GetJson(m_version);
                }
                else
                {
                    resp.jsonData = Json::objectValue;
                }
            }
            else
            {
                resp = JsonError(500, "Request okay but receiver can't connect to sender.", sSenderId);
            }
        }
        else
        {   //no way of telling main thread to do this so we'll simply assume it's been done
            resp.nHttpCode = 202;
            if(pRemoteSender)
            {
                resp.jsonData = pRemoteSender->GetJson(m_version);
            }
            else
            {
                resp.jsonData = Json::objectValue;
            }
            m_api.SetSender(pReceiver, sSenderId, sSdp, "192.168.1.113"); //@todo work out ip address here
        }
    }
    return resp;
}







