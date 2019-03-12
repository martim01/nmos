
#include "is05server.h"
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
#include "microserver.h"

using namespace std;



IS05Server::IS05Server() : NmosServer()
{

}



int IS05Server::GetJsonNmos(MicroServer* pServer, string& sReturn, std::string& sContentType)
{
    Json::FastWriter stw;
    if(m_vPath.size() == SZ_NMOS)
    {
        Json::Value jsNode;
        jsNode.append("connection/");
        sReturn = stw.write(jsNode);
        return 200;
    }
    else if(m_vPath[API_TYPE] == "connection" && NodeApi::Get().GetConnectionPort())
    {
        return GetJsonNmosConnectionApi(sReturn, sContentType);
    }
    sReturn = stw.write(GetJsonError(404, "API not found"));
    return 404;
}

int IS05Server::GetJsonNmosConnectionApi(string& sReturn, std::string& sContentType)
{
    Json::FastWriter stw;
    int nCode = 200;
    if(m_vPath.size() == SZ_API_TYPE)
    {
        Json::Value jsNode;
        jsNode.append("v1.0/");
        sReturn = stw.write(jsNode);
    }
    else
    {
        if(m_vPath[VERSION] == "v1.0")
        {
            if(m_vPath.size() == SZ_VERSION)
            {
                //check the version::
                Json::Value jsNode;
                jsNode.append("bulk/");
                jsNode.append("single/");
                sReturn = stw.write(jsNode);
            }
            else
            {
                if(m_vPath[SZ_VERSION] == "bulk")
                {
                    return GetJsonNmosConnectionBulkApi(sReturn);
                }
                else if(m_vPath[SZ_VERSION] == "single")
                {
                    return GetJsonNmosConnectionSingleApi(sReturn, sContentType, ApiVersion(1,0));

                }
                else
                {
                    nCode = 404;
                    sReturn = stw.write(GetJsonError(404, "Type not supported"));
                }
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

int IS05Server::GetJsonNmosConnectionSingleApi(std::string& sReturn, std::string& sContentType, const ApiVersion& version)
{
    int nCode(200);
    Json::FastWriter stw;
    if(m_vPath.size() == SZC_TYPE)
    {
        Json::Value jsNode;
        jsNode.append("senders/");
        jsNode.append("receivers/");
        sReturn = stw.write(jsNode);
    }
    else if(m_vPath[C_DIRECTION] == "senders")
    {
        return GetJsonNmosConnectionSingleSenders(sReturn, sContentType, version);
    }
    else if(m_vPath[C_DIRECTION] == "receivers")
    {
        return GetJsonNmosConnectionSingleReceivers(sReturn, version);
    }
    else
    {
        nCode = 404;
        sReturn = stw.write(GetJsonError(404, "Endpoint not found"));
    }
    return nCode;
}


int IS05Server::GetJsonNmosConnectionSingleSenders(std::string& sReturn, std::string& sContentType, const ApiVersion& version)
{
    int nCode(200);
    Json::FastWriter stw;
    if(m_vPath.size() == SZC_DIRECTION)
    {
        sReturn = stw.write(NodeApi::Get().GetSenders().GetConnectionJson(version));
    }
    else
    {
        map<string, shared_ptr<Sender> >::const_iterator itResource = NodeApi::Get().GetSenders().FindNmosResource(m_vPath[SZC_DIRECTION]);
        if(itResource != NodeApi::Get().GetSenders().GetResourceEnd())
        {
            if(m_vPath.size() == SZC_ID)
            {
                Json::Value jsNode;
                jsNode.append("constraints/");
                jsNode.append("staged/");
                jsNode.append("active/");
                jsNode.append("transportfile/");
                sReturn = stw.write(jsNode);
            }
            else
            {
                shared_ptr<Sender> pSender = dynamic_pointer_cast<Sender>(itResource->second);
                if(m_vPath[C_LAST] == "constraints")
                {
                    sReturn = stw.write(pSender->GetConnectionConstraintsJson(version));
                }
                else if(m_vPath[C_LAST] == "staged")
                {
                    sReturn = stw.write(pSender->GetConnectionStagedJson(version));
                }
                else if(m_vPath[C_LAST] == "active")
                {
                    sReturn = stw.write(pSender->GetConnectionActiveJson(version));
                }
                else if(m_vPath[C_LAST] == "transportfile")
                {
                    if(pSender->GetActive().bMasterEnable)
                    {
                        // transportfile get
                        sContentType = "application/sdp";
                        sReturn = pSender->GetTransportFile();
                        nCode = 200;
                    }
                    else
                    {
                        nCode = 404;
                        sReturn = stw.write(GetJsonError(404, "MasterEnable=false"));
                    }
                }
                else
                {
                    nCode = 404;
                    sReturn = stw.write(GetJsonError(404, "Endpoint not found"));
                }
            }
        }
        else
        {
            nCode = 404;
            sReturn = stw.write(GetJsonError(404, "Sender not found"));
        }
    }
    return nCode;
}

int IS05Server::GetJsonNmosConnectionSingleReceivers(std::string& sReturn, const ApiVersion& version)
{
    int nCode(200);
    Json::FastWriter stw;
    if(m_vPath.size() == SZC_DIRECTION)
    {
        sReturn = stw.write(NodeApi::Get().GetReceivers().GetConnectionJson(version));
    }
    else
    {
        map<string, shared_ptr<Receiver> >::const_iterator itResource = NodeApi::Get().GetReceivers().FindNmosResource(m_vPath[SZC_DIRECTION]);
        if(itResource != NodeApi::Get().GetReceivers().GetResourceEnd())
        {
            if(m_vPath.size() == SZC_ID)
            {
                Json::Value jsNode;
                jsNode.append("constraints/");
                jsNode.append("staged/");
                jsNode.append("active/");
                sReturn = stw.write(jsNode);
            }
            else
            {
                shared_ptr<Receiver> pReceiver(dynamic_pointer_cast<Receiver>(itResource->second));
                if(m_vPath[C_LAST] == "constraints")
                {
                    sReturn = stw.write(pReceiver->GetConnectionConstraintsJson(version));
                }
                else if(m_vPath[C_LAST] == "staged")
                {
                    sReturn = stw.write(pReceiver->GetConnectionStagedJson(version));
                }
                else if(m_vPath[C_LAST] == "active")
                {
                    sReturn = stw.write(pReceiver->GetConnectionActiveJson(version));
                }
                else
                {
                    nCode = 404;
                    sReturn = stw.write(GetJsonError(404, "Endpoint not found"));
                }
            }
        }
        else
        {
            nCode = 404;
            sReturn = stw.write(GetJsonError(404, "Receiver not found"));
        }
    }
    return nCode;
}


int IS05Server::GetJsonNmosConnectionBulkApi(std::string& sReturn)
{
    int nCode(200);
    Json::FastWriter stw;
    if(m_vPath.size() == SZC_TYPE)
    {
        Json::Value jsNode;
        jsNode.append("senders/");
        jsNode.append("receivers/");
        sReturn = stw.write(jsNode);
    }
    else if(m_vPath[SZC_TYPE] == "senders")
    {
        nCode = 405;
        sReturn = stw.write(GetJsonError(404, "Method not allowed"));
    }
    else if(m_vPath[SZC_TYPE] == "receivers")
    {
        nCode = 405;
        sReturn = stw.write(GetJsonError(404, "Method not allowed"));
    }
    else
    {
        nCode = 404;
        sReturn = stw.write(GetJsonError(404, "Endpoint not found"));
    }
    return nCode;
}



int IS05Server::PatchJsonNmos(MicroServer* pServer, const string& sJson, string& sResponse)
{

    Json::FastWriter stw;

    int nCode = 202;

    if(m_vPath.size() < SZC_LAST || NodeApi::Get().GetConnectionPort() != m_nPort || m_vPath[API_TYPE] != "connection" || m_vPath[VERSION] != "v1.0" || m_vPath[C_TYPE] != "single" || (m_vPath[C_DIRECTION] != "senders" && m_vPath[C_DIRECTION] != "receivers") || m_vPath[C_LAST] != "staged")
    {
        nCode = 405;
        sResponse = stw.write(GetJsonError(nCode, "Method not allowed here."));
    }
    else
    {
        ApiVersion version(m_vPath[VERSION]);

        if(m_vPath[C_DIRECTION] == "senders")
        {
            return PatchJsonSender(pServer, sJson, sResponse, version);
        }
        else
        {
            return PatchJsonReceiver(pServer, sJson, sResponse, version);
        }
    }
    return nCode;
}

int IS05Server::PatchJsonSender(MicroServer* pServer, const std::string& sJson, std::string& sResponse, const ApiVersion& version)
{
    Log::Get(Log::LOG_DEBUG) << "PatchJsonSender" << std::endl;
    int nCode(200);
    Json::FastWriter stw;
    //does the sender exist?
    shared_ptr<Sender> pSender = NodeApi::Get().GetSender(m_vPath[C_ID]);
    if(!pSender)
    {
        nCode = 404;
        sResponse = stw.write(GetJsonError(nCode, "Resource does not exist."));
        Log::Get(Log::LOG_DEBUG) << "PatchJsonSender: Resource does not exist." << std::endl;
    }
    else
    {
        Log::Get(Log::LOG_DEBUG) << "PatchJsonSender: --------------------------------" << std::endl;
        Log::Get(Log::LOG_DEBUG) << sJson << std::endl;
        Log::Get(Log::LOG_DEBUG) << "PatchJsonSender: --------------------------------" << std::endl;
        //is the json valid?
        Json::Value jsRequest;
        Json::Reader jsReader;
        if(jsReader.parse(sJson, jsRequest) == false)
        {
            nCode = 400;
            sResponse = stw.write(GetJsonError(nCode, "Request is ill defined."));

            Log::Get(Log::LOG_DEBUG) << "PatchJsonSender: Request is ill defined." << std::endl;
        }
        else
        {
            connectionSender conRequest(pSender->GetStaged());


            //can we patch a connection from the json?
            if(conRequest.Patch(jsRequest) == false)
            {
                nCode = 400;
                sResponse = stw.write(GetJsonError(nCode, "Request does not meet schema."));
                Log::Get(Log::LOG_DEBUG) << "PatchJsonSender: Request does not meet schema." << std::endl;
            }
            else if(pSender->CheckConstraints(conRequest) == false)
            {//does the patched connection meet the sender constraints?
                nCode = 400;
                sResponse = stw.write(GetJsonError(nCode, "Request does not meet sender's constraints."));
                Log::Get(Log::LOG_DEBUG) << "PatchJsonSender: Request does not meet sender's constraints." << std::endl;
            }
            else if(conRequest.eActivate != connection::ACT_NULL && pSender->IsLocked() == true)
            {   //locked by previous staged activation
                nCode = 423;
                sResponse = stw.write(GetJsonError(nCode, "Sender had pending activation."));
                Log::Get(Log::LOG_DEBUG) << "PatchJsonSender: Sender had pending activation." << std::endl;
            }
            else if(m_pPoster)
            {   //tell the main thread and wait to see what happens
                //the main thread should check that it can definitely do what the patch says and simply signal true or false
                pServer->PrimeWait();
                m_pPoster->_PatchSender(m_vPath[C_ID], conRequest, m_nPort);
                //Pause the HTTP thread
                pServer->Wait();


                if(pServer->IsOk() && pSender->Stage(conRequest, m_pPoster)) //PATCH the sender
                {
                    nCode = 202;
                    sResponse = stw.write(pSender->GetConnectionStagedJson(version));

                    if(conRequest.eActivate == connection::ACT_NULL)
                    {
                        nCode = 200;
                    }
                    else if(conRequest.eActivate == connection::ACT_NOW)
                    {
                        nCode = 200;
                        pSender->CommitActivation();
                    }
                    Log::Get(Log::LOG_DEBUG) << sResponse << std::endl;
                }
                else
                {
                    nCode = 500;
                    sResponse = stw.write(GetJsonError(nCode, "Sender unable to stage PATCH as activation time invalid."));
                    Log::Get(Log::LOG_DEBUG) << "PatchJsonSender: Sender unable to stage PATCH as no EventPoster or activation time invalid." << std::endl;
                }
            }
            else
            {   //no way of telling main thread to do this so we'll simply assume it's been done
                if(pSender->Stage(conRequest, m_pPoster)) //PATCH the sender
                {
                    sResponse = stw.write(pSender->GetConnectionStagedJson(version));
                    Log::Get(Log::LOG_DEBUG) << sResponse << std::endl;
                }
                else
                {
                    nCode = 500;
                    sResponse = stw.write(GetJsonError(nCode, "Sender unable to stage PATCH as no EventPoster or activation time invalid."));
                    Log::Get(Log::LOG_DEBUG) << "PatchJsonSender: Sender unable to stage PATCH as no EventPoster or activation time invalid." << std::endl;
                }
            }
        }
    }
    return nCode;
}

int IS05Server::PatchJsonReceiver(MicroServer* pServer, const std::string& sJson, std::string& sResponse, const ApiVersion& version)
{
    Log::Get(Log::LOG_DEBUG) << "IS05Server::PatchJsonReceiver:  " << sJson << std::endl;
    int nCode(200);
    Json::FastWriter stw;
    //does the sender exist?
    shared_ptr<Receiver> pReceiver = NodeApi::Get().GetReceiver(m_vPath[C_ID]);
    if(!pReceiver)
    {
        nCode = 404;
        sResponse = stw.write(GetJsonError(nCode, "Resource does not exist."));
    }
    else
    {
        //is the json valid?
        Json::Value jsRequest;
        Json::Reader jsReader;
        if(jsReader.parse(sJson, jsRequest) == false)
        {
            nCode = 400;
            sResponse = stw.write(GetJsonError(nCode, "Request is ill defined."));

            Log::Get(Log::LOG_DEBUG) << "PatchJsonReceiver: Request is ill defined." << std::endl;
        }
        else
        {
            connectionReceiver conRequest(pReceiver->GetStaged());
            Log::Get(Log::LOG_DEBUG) << "PatchJsonReceiver: Staged SenderId = '" << pReceiver->GetStaged().sSenderId << "' request SebderId = '" << conRequest.sSenderId << "'" <<std::endl;
            //can we patch a connection from the json?
            if(conRequest.Patch(jsRequest) == false)
            {
                nCode = 400;
                sResponse = stw.write(GetJsonError(nCode, "Request does not meet schema."));
                Log::Get(Log::LOG_DEBUG) << "PatchJsonReceiver: Request does not meet schema." << std::endl;
                Log::Get(Log::LOG_DEBUG) << sJson << std::endl;
            }
            else if(pReceiver->CheckConstraints(conRequest) == false)
            {//does the patched connection meet the Receiver constraints?
                nCode = 400;
                sResponse = stw.write(GetJsonError(nCode, "Request does not meet Receiver's constraints."));
                Log::Get(Log::LOG_DEBUG) << "PatchJsonReceiver: Request does not meet Receiver's constraints." << std::endl;
            }
            else if(conRequest.eActivate != connection::ACT_NULL && pReceiver->IsLocked() == true)
            {   //locked by previous staged activation
                nCode = 423;
                sResponse = stw.write(GetJsonError(nCode, "Receiver had pending activation."));
                Log::Get(Log::LOG_DEBUG) << "PatchJsonReceiver: Receiver had pending activation." << std::endl;
            }
            else if(m_pPoster)
            {   //tell the main thread and wait to see what happens
                pServer->PrimeWait();
                //the main thread should check that it can definitely do what the patch says and simply signal true or false
                m_pPoster->_PatchReceiver(m_vPath[C_ID], conRequest, m_nPort);
                //Pause the HTTP thread
                pServer->Wait();

                if(pServer->IsOk() && pReceiver->Stage(conRequest)) //PATCH the Receiver
                {
                    nCode = 202;
                    sResponse = stw.write(pReceiver->GetConnectionStagedJson(version));
                    if(conRequest.eActivate == connection::ACT_NULL)
                    {
                        nCode = 200;
                    }
                    else if(conRequest.eActivate == connection::ACT_NOW)
                    {
                        nCode = 200;
                        pReceiver->CommitActivation();
                    }
                    Log::Get(Log::LOG_DEBUG) << sResponse << std::endl;
                }
                else
                {
                    nCode = 500;
                    sResponse = stw.write(GetJsonError(nCode, "Receiver unable to stage PATCH as no EventPoster or activation time invalid."));
                    Log::Get(Log::LOG_DEBUG) << "PatchJsonReceiver: Receiver unable to stage PATCH as no EventPoster or activation time invalid." << std::endl;
                }
            }
            else
            {   //no way of telling main thread to do this so we'll simply assume it's been done
                if(pReceiver->Stage(conRequest)) //PATCH the Receiver
                {
                    sResponse = stw.write(pReceiver->GetConnectionStagedJson(version));
                    Log::Get(Log::LOG_DEBUG) << sResponse << std::endl;
                }
                else
                {
                    nCode = 500;
                    sResponse = stw.write(GetJsonError(nCode, "Receiver unable to stage PATCH as no EventPoster or activation time invalid."));
                    Log::Get(Log::LOG_DEBUG) << "PatchJsonReceiver: Sender unable to stage PATCH as no EventPoster or activation time invalid." << std::endl;
                }
            }
        }
    }
    return nCode;
}
