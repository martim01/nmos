#include "is04server.h"
#include "nodeapi.h"
#include <sstream>
#include <algorithm>
#include <iostream>
#include "self.h"
#include "device.h"
#include "source.h"
#include "flow.h"
#include "receiver.h"
#include "sender.h"
#include "log.h"
#include "eventposter.h"
#include "utils.h"
#include "curlregister.h"


using namespace std;



IS04Server::IS04Server(shared_ptr<EventPoster> pPoster) : MicroServer(pPoster)
{

}



int IS04Server::GetJsonNmos(string& sReturn, std::string& sContentType)
{
    Json::StyledWriter stw;
    if(m_vPath.size() == SZ_NMOS)
    {
        Json::Value jsNode;
        jsNode.append("node/");
        sReturn = stw.write(jsNode);
        return 200;
    }
    else if(m_vPath[API_TYPE] == "node" && m_nPort != NodeApi::Get().GetConnectionPort())
    {
        return GetJsonNmosNodeApi(sReturn);

    }
    sReturn = stw.write(GetJsonError(404, "API not found"));
    return 404;
}

int IS04Server::GetJsonNmosNodeApi(string& sReturn)
{
    Json::StyledWriter stw;
    int nCode = 200;
    if(m_vPath.size() == SZ_API_TYPE)
    {
        sReturn = stw.write(NodeApi::Get().GetSelf().JsonVersions());
    }
    else
    {

        if(NodeApi::Get().GetSelf().IsVersionSupported(m_vPath[VERSION]))
        {
            ApiVersion version(m_vPath[VERSION]);

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
                sReturn  = stw.write(NodeApi::Get().GetSelf().GetJson(version));
            }
            else if(m_vPath[ENDPOINT] == "sources")
            {
                sReturn  = stw.write(GetJsonSources(version));
            }
            else if(m_vPath[ENDPOINT] == "flows")
            {
                sReturn  = stw.write(GetJsonFlows(version));
            }
            else if(m_vPath[ENDPOINT] == "devices")
            {
                sReturn  = stw.write(GetJsonDevices(version));
            }
            else if(m_vPath[ENDPOINT] == "senders")
            {
                sReturn  = stw.write(GetJsonSenders(version));
            }
            else if(m_vPath[ENDPOINT] == "receivers")
            {
                sReturn  = stw.write(GetJsonReceivers(version));
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

Json::Value IS04Server::GetJsonSources(const ApiVersion& version)
{
    if(m_vPath.size() == SZ_ENDPOINT)
    {
        return NodeApi::Get().GetSources().GetJson(version);
    }
    else
    {
        map<string, std::shared_ptr<Source> >::const_iterator itSource = NodeApi::Get().GetSources().FindNmosResource(m_vPath[RESOURCE]);
        if(itSource != NodeApi::Get().GetSources().GetResourceEnd())
        {
            return itSource->second->GetJson(version);
        }
    }
    return GetJsonError();
}

Json::Value IS04Server::GetJsonDevices(const ApiVersion& version)
{
    if(m_vPath.size() == SZ_ENDPOINT)
    {
        return NodeApi::Get().GetDevices().GetJson(version);
    }
    else
    {
        map<string, shared_ptr<Device> >::const_iterator itDevice = NodeApi::Get().GetDevices().FindNmosResource(m_vPath[RESOURCE]);
        if(itDevice != NodeApi::Get().GetDevices().GetResourceEnd())
        {
            return itDevice->second->GetJson(version);
        }
    }
    return GetJsonError(404, "Device "+m_vPath[RESOURCE]+"does not exist.");
}

Json::Value IS04Server::GetJsonFlows(const ApiVersion& version)
{
    if(m_vPath.size() == SZ_ENDPOINT)
    {
        return NodeApi::Get().GetFlows().GetJson(version);
    }
    else
    {
        map<string, shared_ptr<Flow> >::const_iterator itFlow = NodeApi::Get().GetFlows().FindNmosResource(m_vPath[RESOURCE]);
        if(itFlow != NodeApi::Get().GetFlows().GetResourceEnd())
        {
            return itFlow->second->GetJson(version);
        }
    }
    return GetJsonError(404, "Flow "+m_vPath[RESOURCE]+"does not exist.");
}

Json::Value IS04Server::GetJsonReceivers(const ApiVersion& version)
{
    if(m_vPath.size() == SZ_ENDPOINT)
    {
        return NodeApi::Get().GetReceivers().GetJson(version);
    }
    else
    {
        map<string, shared_ptr<Receiver> >::const_iterator itReceiver = NodeApi::Get().GetReceivers().FindNmosResource(m_vPath[RESOURCE]);
        if(itReceiver != NodeApi::Get().GetReceivers().GetResourceEnd())
        {
            return itReceiver->second->GetJson(version);
        }
    }
    return GetJsonError(404, "Receiver "+m_vPath[RESOURCE]+"does not exist.");
}

Json::Value IS04Server::GetJsonSenders(const ApiVersion& version)
{
    if(m_vPath.size() == SZ_ENDPOINT)
    {
        return NodeApi::Get().GetSenders().GetJson(version);
    }
    else
    {
        map<string, shared_ptr<Sender> >::const_iterator itSender = NodeApi::Get().GetSenders().FindNmosResource(m_vPath[RESOURCE]);
        if(itSender != NodeApi::Get().GetSenders().GetResourceEnd())
        {
            return itSender->second->GetJson(version);
        }
    }
    return GetJsonError(404, "Sender "+m_vPath[RESOURCE]+"does not exist.");
}




int IS04Server::PutJsonNmos(const string& sJson, string& sResponse)
{
    Json::StyledWriter stw;
    int nCode = 500;
    if(m_vPath.size() < SZ_TARGET)
    {
        nCode = 405;
        sResponse = stw.write(GetJsonError(nCode, "Method not allowed here: "));
    }
    else
    {
        if(m_vPath[API_TYPE] == "node" &&  NodeApi::Get().GetSelf().IsVersionSupported(m_vPath[VERSION]) && m_vPath[ENDPOINT] == "receivers" && m_vPath[TARGET] == "target")
        {
            ApiVersion version(m_vPath[VERSION]);
            //does the receiver exist?
            shared_ptr<Receiver> pReceiver  = NodeApi::Get().GetReceiver(m_vPath[RESOURCE]);
            if(!pReceiver)
            {
                nCode = 404;
                sResponse = stw.write(GetJsonError(nCode, "Resource "+m_vPath[RESOURCE]+"does not exist."));
            }
            else
            {
                Log::Get(Log::LOG_DEBUG) << sJson << std::endl;
                Json::Value jsRequest;
                Json::Reader jsReader;
                if(jsReader.parse(sJson, jsRequest) == false)
                {
                    nCode = 400;
                    sResponse = stw.write(GetJsonError(nCode, "Request is ill defined."));
                }
                else
                {
                    //does the sender have a manifest??
                    std::string sSdp("");
                    std::string sSenderId("");
                    std::shared_ptr<Sender> pRemoteSender(0);
                    //Have we been sent a sender??
                    if(jsRequest.isObject() && jsRequest["id"].isString())
                    {
                        pRemoteSender = std::make_shared<Sender>();
                        pRemoteSender->UpdateFromJson(jsRequest);

                        sSenderId = pRemoteSender->GetId();
                        if(pRemoteSender->GetManifestHref().empty() == false)
                        {
                            CurlRegister::Get(pRemoteSender->GetManifestHref(), sSdp);
                            Log::Get(Log::LOG_INFO) << " SDP: " << sSdp << std::endl;
                        }
                    }
                    if(m_pPoster)
                    {
                        PrimeWait();
                        //Tell the main thread to connect
                        m_pPoster->_Target(m_vPath[RESOURCE], sSdp, m_nPort);
                        //Pause the HTTP thread
                        Log::Get(Log::LOG_DEBUG) << "IS04Server: Wait" << std::endl;
                        Wait();

                        if(IsOk())
                        {   //this means the main thread has connected the receiver to the sender
                            nCode = 202;
                            pReceiver->SetSender(sSenderId, sSdp, m_sSignalData);
                            NodeApi::Get().Commit();   //updates the registration node or txt records

                            if(pRemoteSender)
                            {
                                sResponse = stw.write(pRemoteSender->GetJson(version));
                            }
                            else
                            {
                                sResponse = "{}";
                            }
                        }
                        else
                        {
                            nCode = 500;
                            sResponse = stw.write(GetJsonError(nCode, "Request okay but receiver can't connect to sender."));
                        }
                    }
                    else
                    {   //no way of telling main thread to do this so we'll simply assume it's been done
                        nCode = 202;
                        sResponse = stw.write(pRemoteSender->GetJson(version));
                    }
                }
            }
        }
        else
        {
            nCode = 404;
            sResponse = stw.write(GetJsonError(404, "Page not found"));
        }
    }
    return nCode;
}

