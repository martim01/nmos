
#include "is05server.h"
#include "nodeapiprivate.h"
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

using namespace std::placeholders;
using namespace pml::nmos;


const std::string IS05Server::ROOT = "/x-nmos/connection/";
const std::string IS05Server::BULK = "/bulk";
const std::string IS05Server::SINGLE = "/single";
const std::string IS05Server::SENDERS = "/senders";
const std::string IS05Server::RECEIVERS = "/receivers";
const std::string IS05Server::CONSTRAINTS = "/constraints";
const std::string IS05Server::STAGED = "/staged";
const std::string IS05Server::ACTIVE = "/active";
const std::string IS05Server::TRANSPORTFILE = "/transportfile";
const std::string IS05Server::TRANSPORTTYPE = "/transporttype";

IS05Server::IS05Server(std::shared_ptr<RestGoose> pServer, const ApiVersion& version, std::shared_ptr<EventPoster> pPoster, NodeApiPrivate& api) :
    NmosServer(pServer, version, pPoster, api)
{
    AddBaseEndpoints();
}

IS05Server::~IS05Server()
{

}

void IS05Server::AddBaseEndpoints()
{
    m_pServer->AddEndpoint(endpoint(RestGoose::GET, url("/x-nmos/connection")), std::bind(&IS05Server::GetNmosRoot, this, _1,_2,_3,_4));
    m_pServer->AddEndpoint(endpoint(RestGoose::GET, url(ROOT+m_version.GetVersionAsString())), std::bind(&IS05Server::GetNmosVersion, this, _1,_2,_3,_4));
    m_pServer->AddEndpoint(endpoint(RestGoose::GET, url(ROOT+m_version.GetVersionAsString()+BULK)), std::bind(&IS05Server::GetNmosBulk, this, _1,_2,_3,_4));
    m_pServer->AddEndpoint(endpoint(RestGoose::GET, url(ROOT+m_version.GetVersionAsString()+BULK+SENDERS)), std::bind(&IS05Server::GetNmosBulkSenders, this, _1,_2,_3,_4));
    m_pServer->AddEndpoint(endpoint(RestGoose::POST,url(ROOT+m_version.GetVersionAsString()+BULK+SENDERS)), std::bind(&IS05Server::PostNmosBulkSenders, this, _1,_2,_3,_4));
    m_pServer->AddEndpoint(endpoint(RestGoose::GET, url(ROOT+m_version.GetVersionAsString()+BULK+RECEIVERS)), std::bind(&IS05Server::GetNmosBulkReceivers, this, _1,_2,_3,_4));
    m_pServer->AddEndpoint(endpoint(RestGoose::POST,url(ROOT+m_version.GetVersionAsString()+BULK+RECEIVERS)), std::bind(&IS05Server::PostNmosBulkReceivers, this, _1,_2,_3,_4));
    m_pServer->AddEndpoint(endpoint(RestGoose::GET, url(ROOT+m_version.GetVersionAsString()+SINGLE)), std::bind(&IS05Server::GetNmosSingle, this, _1,_2,_3,_4));
    m_pServer->AddEndpoint(endpoint(RestGoose::GET, url(ROOT+m_version.GetVersionAsString()+SINGLE+SENDERS)), std::bind(&IS05Server::GetNmosSingleSenders, this, _1,_2,_3,_4));
    m_pServer->AddEndpoint(endpoint(RestGoose::GET, url(ROOT+m_version.GetVersionAsString()+SINGLE+RECEIVERS)), std::bind(&IS05Server::GetNmosSingleReceivers, this, _1,_2,_3,_4));
}

void IS05Server::AddSenderEndpoint(const std::string& sId)
{
    m_pServer->AddEndpoint(endpoint(RestGoose::GET, url(ROOT+m_version.GetVersionAsString()+SINGLE+SENDERS+"/"+sId)), std::bind(&IS05Server::GetNmosSingleSender, this, _1,_2,_3,_4));
    m_pServer->AddEndpoint(endpoint(RestGoose::GET, url(ROOT+m_version.GetVersionAsString()+SINGLE+SENDERS+"/"+sId+CONSTRAINTS)), std::bind(&IS05Server::GetNmosSingleSenderConstraints, this, _1,_2,_3,_4));
    m_pServer->AddEndpoint(endpoint(RestGoose::GET, url(ROOT+m_version.GetVersionAsString()+SINGLE+SENDERS+"/"+sId+STAGED)), std::bind(&IS05Server::GetNmosSingleSenderStaged, this, _1,_2,_3,_4));
    m_pServer->AddEndpoint(endpoint(RestGoose::GET, url(ROOT+m_version.GetVersionAsString()+SINGLE+SENDERS+"/"+sId+ACTIVE)), std::bind(&IS05Server::GetNmosSingleSenderActive, this, _1,_2,_3,_4));
    m_pServer->AddEndpoint(endpoint(RestGoose::GET, url(ROOT+m_version.GetVersionAsString()+SINGLE+SENDERS+"/"+sId+TRANSPORTFILE)), std::bind(&IS05Server::GetNmosSingleSenderTransportfile, this, _1,_2,_3,_4));
    if(m_version > ApiVersion(1,0))
    {
        m_pServer->AddEndpoint(endpoint(RestGoose::GET, url(ROOT+m_version.GetVersionAsString()+SINGLE+SENDERS+"/"+sId+TRANSPORTTYPE)), std::bind(&IS05Server::GetNmosSingleSenderTransportType, this, _1,_2,_3,_4));
    }
    m_pServer->AddEndpoint(endpoint(RestGoose::PATCH, url(ROOT+m_version.GetVersionAsString()+SINGLE+SENDERS+"/"+sId+STAGED)), std::bind(&IS05Server::PatchNmosSingleSenderStaged, this, _1,_2,_3,_4));
}

void IS05Server::AddReceiverEndpoint(const std::string& sId)
{
    m_pServer->AddEndpoint(endpoint(RestGoose::GET, url(ROOT+m_version.GetVersionAsString()+SINGLE+RECEIVERS+"/"+sId)), std::bind(&IS05Server::GetNmosSingleReceiver, this, _1,_2,_3,_4));
    m_pServer->AddEndpoint(endpoint(RestGoose::GET, url(ROOT+m_version.GetVersionAsString()+SINGLE+RECEIVERS+"/"+sId+CONSTRAINTS)), std::bind(&IS05Server::GetNmosSingleReceiverConstraints, this, _1,_2,_3,_4));
    m_pServer->AddEndpoint(endpoint(RestGoose::GET, url(ROOT+m_version.GetVersionAsString()+SINGLE+RECEIVERS+"/"+sId+STAGED)), std::bind(&IS05Server::GetNmosSingleReceiverStaged, this, _1,_2,_3,_4));
    m_pServer->AddEndpoint(endpoint(RestGoose::GET, url(ROOT+m_version.GetVersionAsString()+SINGLE+RECEIVERS+"/"+sId+ACTIVE)), std::bind(&IS05Server::GetNmosSingleReceiverActive, this, _1,_2,_3,_4));
    if(m_version > ApiVersion(1,0))
    {
        m_pServer->AddEndpoint(endpoint(RestGoose::GET, url(ROOT+m_version.GetVersionAsString()+SINGLE+RECEIVERS+"/"+sId+TRANSPORTTYPE)), std::bind(&IS05Server::GetNmosSingleReceiverTransportType, this, _1,_2,_3,_4));
    }
    m_pServer->AddEndpoint(endpoint(RestGoose::PATCH, url(ROOT+m_version.GetVersionAsString()+SINGLE+RECEIVERS+"/"+sId+STAGED)), std::bind(&IS05Server::PatchNmosSingleReceiverStaged, this, _1,_2,_3,_4));
}

void IS05Server::RemoveSenderEndpoint(const std::string& sId)
{
    m_pServer->DeleteEndpoint(endpoint(RestGoose::GET, url(ROOT+m_version.GetVersionAsString()+SINGLE+SENDERS+"/"+sId)));
    m_pServer->DeleteEndpoint(endpoint(RestGoose::GET, url(ROOT+m_version.GetVersionAsString()+SINGLE+SENDERS+"/"+sId+CONSTRAINTS)));
    m_pServer->DeleteEndpoint(endpoint(RestGoose::GET, url(ROOT+m_version.GetVersionAsString()+SINGLE+SENDERS+"/"+sId+STAGED)));
    m_pServer->DeleteEndpoint(endpoint(RestGoose::GET, url(ROOT+m_version.GetVersionAsString()+SINGLE+SENDERS+"/"+sId+ACTIVE)));
    m_pServer->DeleteEndpoint(endpoint(RestGoose::GET, url(ROOT+m_version.GetVersionAsString()+SINGLE+SENDERS+"/"+sId+TRANSPORTFILE)));
    if(m_version > ApiVersion(1,0))
    {
        m_pServer->DeleteEndpoint(endpoint(RestGoose::GET, url(ROOT+m_version.GetVersionAsString()+SINGLE+SENDERS+"/"+sId+TRANSPORTTYPE)));
    }
    m_pServer->DeleteEndpoint(endpoint(RestGoose::PATCH, url(ROOT+m_version.GetVersionAsString()+SINGLE+SENDERS+"/"+sId+STAGED)));
}

void IS05Server::RemoveReceiverEndpoint(const std::string& sId)
{
    m_pServer->DeleteEndpoint(endpoint(RestGoose::GET, url(ROOT+m_version.GetVersionAsString()+SINGLE+RECEIVERS+"/"+sId)));
    m_pServer->DeleteEndpoint(endpoint(RestGoose::GET, url(ROOT+m_version.GetVersionAsString()+SINGLE+RECEIVERS+"/"+sId+CONSTRAINTS)));
    m_pServer->DeleteEndpoint(endpoint(RestGoose::GET, url(ROOT+m_version.GetVersionAsString()+SINGLE+RECEIVERS+"/"+sId+STAGED)));
    m_pServer->DeleteEndpoint(endpoint(RestGoose::GET, url(ROOT+m_version.GetVersionAsString()+SINGLE+RECEIVERS+"/"+sId+ACTIVE)));
    if(m_version > ApiVersion(1,0))
    {
        m_pServer->DeleteEndpoint(endpoint(RestGoose::GET, url(ROOT+m_version.GetVersionAsString()+SINGLE+RECEIVERS+"/"+sId+TRANSPORTTYPE)));
    }
    m_pServer->DeleteEndpoint(endpoint(RestGoose::PATCH, url(ROOT+m_version.GetVersionAsString()+SINGLE+RECEIVERS+"/"+sId+STAGED)));
}


response IS05Server::GetNmosRoot(const query& theQuery, const postData& theData, const url& theUrl, const userName& theUser)
{
    response resp;
    resp.jsonData = m_api.JsonConnectionVersions();
    return resp;
}

response IS05Server::GetNmosVersion(const query& theQuery, const postData& theData, const url& theUrl, const userName& theUser)
{
    response resp;
    resp.jsonData.append("bulk/");
    resp.jsonData.append("single/");
    return resp;
}

response IS05Server::GetNmosBulk(const query& theQuery, const postData& theData, const url& theUrl, const userName& theUser)
{
    response resp;
    resp.jsonData.append("senders/");
    resp.jsonData.append("receivers/");
    return resp;

}

response IS05Server::GetNmosBulkSenders(const query& theQuery, const postData& theData, const url& theUrl, const userName& theUser)
{
    return JsonError(405, "Method not allowed here", theUrl.Get());
}

response IS05Server::PostNmosBulkSenders(const query& theQuery, const postData& theData, const url& theUrl, const userName& theUser)
{
    return PostJsonSenders(ConvertToJson(theData.Get()));
}

response IS05Server::GetNmosBulkReceivers(const query& theQuery, const postData& theData, const url& theUrl, const userName& theUser)
{
    return JsonError(405, "Method not allowed here", theUrl.Get());
}

response IS05Server::PostNmosBulkReceivers(const query& theQuery, const postData& theData, const url& theUrl, const userName& theUser)
{
    return PostJsonReceivers(ConvertToJson(theData.Get()));
}

response IS05Server::GetNmosSingle(const query& theQuery, const postData& theData, const url& theUrl, const userName& theUser)
{
    response resp;
    resp.jsonData.append("senders/");
    resp.jsonData.append("receivers/");
    return resp;
}

response IS05Server::GetNmosSingleSenders(const query& theQuery, const postData& theData, const url& theUrl, const userName& theUser)
{
    response resp;
    resp.jsonData = m_api.GetSenders().GetConnectionJson(m_version);
    return resp;
}

response IS05Server::GetNmosSingleReceivers(const query& theQuery, const postData& theData, const url& theUrl, const userName& theUser)
{
    response resp;
    resp.jsonData = m_api.GetReceivers().GetConnectionJson(m_version);
    return resp;
}

response IS05Server::GetNmosSingleSender(const query& theQuery, const postData& theData, const url& theUrl, const userName& theUser)
{
    response resp;
    if(GetSender(theUrl) != nullptr)
    {
        resp.jsonData.append("constraints/");
        resp.jsonData.append("staged/");
        resp.jsonData.append("active/");
        resp.jsonData.append("transportfile/");
        if(m_version > ApiVersion(1,0))
        {
            resp.jsonData.append("transporttype/");
        }
    }
    else
    {
        resp = JsonError(404, "Sender does not exist", theUrl.Get());
    }

    return resp;
}

response IS05Server::GetNmosSingleSenderConstraints(const query& theQuery, const postData& theData, const url& theUrl, const userName& theUser)
{
    response resp;
    auto pSender = GetSender(theUrl);
    if(pSender)
    {
        resp.jsonData = pSender->GetConnectionConstraintsJson(m_version);
    }
    else
    {
        resp = JsonError(404, "Sender does not exist", theUrl.Get());
    }
    return resp;
}

response IS05Server::GetNmosSingleSenderStaged(const query& theQuery, const postData& theData, const url& theUrl, const userName& theUser)
{
    response resp;
    auto pSender = GetSender(theUrl);
    if(pSender)
    {
        resp.jsonData = pSender->GetConnectionStagedJson(m_version);
    }
    else
    {
        resp = JsonError(404, "Sender does not exist", theUrl.Get());
    }
    return resp;
}

response IS05Server::GetNmosSingleSenderActive(const query& theQuery, const postData& theData, const url& theUrl, const userName& theUser)
{
    response resp;
    auto pSender = GetSender(theUrl);
    if(pSender)
    {
        resp.jsonData = pSender->GetConnectionActiveJson(m_version);
    }
    else
    {
        resp = JsonError(404, "Sender does not exist", theUrl.Get());
    }
    return resp;
}

response IS05Server::GetNmosSingleSenderTransportfile(const query& theQuery, const postData& theData, const url& theUrl, const userName& theUser)
{
    response resp;
    auto pSender = GetSender(theUrl);
    if(pSender)
    {
        if(pSender->GetActive().bMasterEnable)
        {
            resp.sContentType = "application/sdp";
            resp.sData = pSender->GetTransportFile();
        }
        else
        {
            resp = JsonError(404, "MasterEnable=false", theUrl.Get());
        }

    }
    else
    {
        resp = JsonError(404, "Sender does not exist", theUrl.Get());
    }
    return resp;
}

response IS05Server::GetNmosSingleSenderTransportType(const query& theQuery, const postData& theData, const url& theUrl, const userName& theUser)
{
    response resp;
    auto pSender = GetSender(theUrl);
    if(pSender)
    {
        auto vType = SplitString(pSender->GetTransportType(),'.');
        resp.jsonData = vType[0];
    }
    else
    {
        resp = JsonError(404, "Sender does not exist", theUrl.Get());
    }
    return resp;
}

response IS05Server::PatchNmosSingleSenderStaged(const query& theQuery, const postData& theData, const url& theUrl, const userName& theUser)
{
    auto pSender = GetSender(theUrl);
    if(pSender)
    {
        return PatchSender(pSender, ConvertToJson(theData.Get()));
    }
    else
    {
        return JsonError(404, "Sender does not exist", theUrl.Get());
    }
}

response IS05Server::GetNmosSingleReceiver(const query& theQuery, const postData& theData, const url& theUrl, const userName& theUser)
{
    response resp;
    auto pReceiver = GetReceiver(theUrl);
    if(pReceiver)
    {
        resp.jsonData.append("constraints/");
        resp.jsonData.append("staged/");
        resp.jsonData.append("active/");
        if(m_version > ApiVersion(1,0))
        {
            resp.jsonData.append("transporttype/");
        }
    }
    else
    {
        resp = JsonError(404, "Receiver does not exist", theUrl.Get());
    }
    return resp;
}

response IS05Server::GetNmosSingleReceiverConstraints(const query& theQuery, const postData& theData, const url& theUrl, const userName& theUser)
{
    response resp;
    auto pReceiver = GetReceiver(theUrl);
    if(pReceiver)
    {
        resp.jsonData = pReceiver->GetConnectionConstraintsJson(m_version);
    }
    else
    {
        resp = JsonError(404, "Receiver does not exist", theUrl.Get());
    }
    return resp;
}

response IS05Server::GetNmosSingleReceiverStaged(const query& theQuery, const postData& theData, const url& theUrl, const userName& theUser)
{
    response resp;
    auto pReceiver = GetReceiver(theUrl);
    if(pReceiver)
    {
        resp.jsonData = pReceiver->GetConnectionStagedJson(m_version);
    }
    else
    {
        resp = JsonError(404, "Receiver does not exist", theUrl.Get());
    }
    return resp;
}

response IS05Server::GetNmosSingleReceiverActive(const query& theQuery, const postData& theData, const url& theUrl, const userName& theUser)
{
    response resp;
    auto pReceiver = GetReceiver(theUrl);
    if(pReceiver)
    {
        resp.jsonData = pReceiver->GetConnectionActiveJson(m_version);
    }
    else
    {
        resp = JsonError(404, "Receiver does not exist", theUrl.Get());
    }
    return resp;
}

response IS05Server::GetNmosSingleReceiverTransportType(const query& theQuery, const postData& theData, const url& theUrl, const userName& theUser)
{
    response resp;
    auto pReceiver = GetReceiver(theUrl);
    if(pReceiver)
    {
        auto vType = SplitString(pReceiver->GetTransportType(),'.');
        resp.jsonData = vType[0];
    }
    else
    {
        resp = JsonError(404, "Receiver does not exist", theUrl.Get());
    }
    return resp;
}

response IS05Server::PatchNmosSingleReceiverStaged(const query& theQuery, const postData& theData, const url& theUrl, const userName& theUser)
{
    auto pReceiver = GetReceiver(theUrl);
    if(pReceiver)
    {
        return PatchReceiver(pReceiver, ConvertToJson(theData.Get()));
    }
    else
    {
        return JsonError(404, "Receiver does not exist", theUrl.Get());
    }
}



response IS05Server::PatchSender(std::shared_ptr<Sender> pSender, const Json::Value& jsRequest)
{
    pmlLog(pml::LOG_DEBUG) << "NMOS: " << "PatchSender: " << pSender->GetId() ;
    response resp;


    connectionSender conRequest(pSender->GetStaged());
    //can we patch a connection from the json?
    if(conRequest.Patch(jsRequest) == false)
    {
        pmlLog(pml::LOG_DEBUG) << "NMOS: " << "PatchSender: Request does not meet schema." ;

        std::stringstream ssDebug;
        ssDebug << jsRequest;
        resp = JsonError(400, "Request does not meet schema.", ssDebug.str());
    }
    else if(pSender->CheckConstraints(conRequest) == false)
    {//does the patched connection meet the sender constraints?
        std::stringstream ssDebug;
        ssDebug << jsRequest;
        resp = JsonError(400, "Request does not meet sender's constraints.", ssDebug.str());
        pmlLog(pml::LOG_DEBUG) << "NMOS: " << "PatchSender: Request does not meet sender's constraints." ;
    }
    else if(conRequest.eActivate != connection::ACT_NULL && pSender->IsLocked() == true)
    {   //locked by previous staged activation
        resp = JsonError(423, "Sender had pending activation.");
    }
    else if(m_pPoster)
    {   //tell the main thread and wait to see what happens
        //the main thread should check that it can definitely do what the patch says and simply signal true or false
        m_pServer->PrimeWait();
        m_pPoster->_PatchSender(pSender->GetId(), conRequest, m_pServer->GetPort());
        //Pause the HTTP thread
        m_pServer->Wait();


        if(m_pServer->IsOk() && m_api.Stage(conRequest, pSender)) //PATCH the sender
        {
            resp.nHttpCode = 202;
            resp.jsonData = pSender->GetConnectionStagedJson(m_version);

            if(conRequest.eActivate == connection::ACT_NULL)
            {
                resp.nHttpCode = 200;
            }
            else if(conRequest.eActivate == connection::ACT_NOW)
            {
                resp.nHttpCode = 200;
                m_api.CommitActivation(pSender);
            }
            pmlLog(pml::LOG_DEBUG) << "NMOS: " << resp.jsonData ;
        }
        else
        {
            resp = JsonError(500, "Sender unable to stage PATCH as activation time invalid.");
            pmlLog(pml::LOG_DEBUG) << "NMOS: " << "PatchSender: Sender unable to stage PATCH as activation time invalid." ;
        }
    }
    else
    {   //no way of telling main thread to do this so we'll simply assume it's been done
        if(m_api.Stage(conRequest, pSender)) //PATCH the sender
        {
            resp.nHttpCode = 200;
            resp.jsonData = pSender->GetConnectionStagedJson(m_version);
            pmlLog(pml::LOG_DEBUG) << "NMOS: " << resp.jsonData ;
        }
        else
        {
            resp = JsonError(500, "Sender unable to stage PATCH as no EventPoster or activation time invalid.");
            pmlLog(pml::LOG_DEBUG) << "NMOS: " << "PatchSender: Sender unable to stage PATCH as no EventPoster or activation time invalid." ;
        }
    }
    return resp;
}


response IS05Server::PatchReceiver(std::shared_ptr<Receiver> pReceiver, const Json::Value& jsRequest)
{
    response resp;

    connectionReceiver conRequest(pReceiver->GetStaged());
    pmlLog(pml::LOG_DEBUG) << "NMOS: " << "PatchJsonReceiver: Staged SenderId = '" << pReceiver->GetStaged().sSenderId << "' request SenderId = '" << conRequest.sSenderId << "'";
    //can we patch a connection from the json?
    if(conRequest.Patch(jsRequest) == false)
    {
        resp =JsonError(400, "Request does not meet schema.");
        pmlLog(pml::LOG_DEBUG) << "NMOS: " << "PatchJsonReceiver: Request does not meet schema." ;
    }
    else if(pReceiver->CheckConstraints(conRequest) == false)
    {//does the patched connection meet the Receiver constraints?
        resp =JsonError(400, "Request does not meet Receiver's constraints.");
        pmlLog(pml::LOG_DEBUG) << "NMOS: " << "PatchJsonReceiver: Request does not meet Receiver's constraints." ;
    }
    else if(conRequest.eActivate != connection::ACT_NULL && pReceiver->IsLocked() == true)
    {   //locked by previous staged activation
        resp =JsonError(423, "Receiver had pending activation.");
        pmlLog(pml::LOG_DEBUG) << "NMOS: " << "PatchJsonReceiver: Receiver had pending activation." ;
    }
    else if(m_pPoster)
    {   //tell the main thread and wait to see what happens
        m_pServer->PrimeWait();
        //the main thread should check that it can definitely do what the patch says and simply signal true or false
        m_pPoster->_PatchReceiver(pReceiver->GetId(), conRequest, m_pServer->GetPort());
        //Pause the HTTP thread
        m_pServer->Wait();

        if(m_pServer->IsOk() && m_api.Stage(conRequest, pReceiver)) //PATCH the Receiver
        {
            resp.nHttpCode = 202;
            resp.jsonData = pReceiver->GetConnectionStagedJson(m_version);
            if(conRequest.eActivate == connection::ACT_NULL)
            {
                resp.nHttpCode = 200;
            }
            else if(conRequest.eActivate == connection::ACT_NOW)
            {
                resp.nHttpCode = 200;
                m_api.CommitActivation(pReceiver);
            }
            pmlLog(pml::LOG_DEBUG) << "NMOS: " << resp.jsonData ;
        }
        else
        {
            resp =JsonError(500, "Receiver unable to stage PATCH as no EventPoster or activation time invalid.");
            pmlLog(pml::LOG_DEBUG) << "NMOS: " << "PatchJsonReceiver: Receiver unable to stage PATCH as no EventPoster or activation time invalid." ;
        }
    }
    else
    {   //no way of telling main thread to do this so we'll simply assume it's been done
        if(m_api.Stage(conRequest, pReceiver)) //PATCH the Receiver
        {
            resp.jsonData = pReceiver->GetConnectionStagedJson(m_version);
            pmlLog(pml::LOG_DEBUG) << resp.jsonData ;
        }
        else
        {
            resp =JsonError(500, "Receiver unable to stage PATCH as no EventPoster or activation time invalid.");
            pmlLog(pml::LOG_DEBUG) << "NMOS: " << "PatchJsonReceiver: Sender unable to stage PATCH as no EventPoster or activation time invalid." ;
        }
    }
    return resp;
}


response IS05Server::PostJsonSenders(const Json::Value& jsRequest)
{
    pmlLog(pml::LOG_DEBUG) << "NMOS: " << "IS05Server::PostJsonSenders:  " << jsRequest ;

    response resp;
    //check that each part of the array is correct.
    for(Json::ArrayIndex ai = 0; ai < jsRequest.size(); ai++)
    {
        if(jsRequest[ai]["id"].isString() == false || jsRequest[ai]["params"].isObject() == false || CheckJson(jsRequest[ai], {"id", "params"}) == false)
        {
            resp = JsonError(400, "Request is ill defined.");
            break;
        }
    }


    if(resp.nHttpCode == 200)
    {   //passed the JSON checking

        for(Json::ArrayIndex ai = 0; ai < jsRequest.size(); ai++)
        {
            Json::Value jsResponse(Json::objectValue);
            jsResponse["id"] = jsRequest[ai]["id"];

            auto pSender = m_api.GetSender(jsRequest[ai]["id"].asString());
            if(!pSender)
            {
                jsResponse["code"] = 404;
                jsResponse["error"] = "Resource does not exist.";
                jsResponse["debug"] = "null";
                pmlLog(pml::LOG_DEBUG) << "NMOS: " << "PostJsonSenders: Resource does not exist." ;
            }
            else
            {
                auto patchResponse = PatchSender(pSender, jsRequest[ai]["params"]);
                jsResponse["code"] = patchResponse.nHttpCode;
                if(patchResponse.nHttpCode != 200)
                {
                    jsResponse["error"] = patchResponse.jsonData["error"];
                }
            }
            resp.jsonData.append(jsResponse);
        }

    }
    return resp;
}


response IS05Server::PostJsonReceivers(const Json::Value& jsRequest)
{
    pmlLog(pml::LOG_DEBUG) << "NMOS: " << "IS05Server::PostJsonReceivers:  " << jsRequest ;

    response resp;
    //check that each part of the array is correct.
    for(Json::ArrayIndex ai = 0; ai < jsRequest.size(); ai++)
    {
        if(jsRequest[ai]["id"].isString() == false || jsRequest[ai]["params"].isObject() == false || CheckJson(jsRequest[ai], {"id", "params"}) == false)
        {
            resp = JsonError(400, "Request is ill defined.");
            break;
        }
    }


    if(resp.nHttpCode == 200)
    {   //passed the JSON checking
        for(Json::ArrayIndex ai = 0; ai < jsRequest.size(); ai++)
        {
            Json::Value jsResponse(Json::objectValue);
            jsResponse["id"] = jsRequest[ai]["id"];

            auto pReceiver = m_api.GetReceiver(jsRequest[ai]["id"].asString());
            if(!pReceiver)
            {
                jsResponse["code"] = 404;
                jsResponse["error"] = "Resource does not exist.";
                jsResponse["debug"] = "null";
                pmlLog(pml::LOG_DEBUG) << "NMOS: " << "PostJsonReceivers: Resource does not exist." ;
            }
            else
            {
                auto patchResponse = PatchReceiver(pReceiver, jsRequest[ai]["params"]);
                jsResponse["code"] = patchResponse.nHttpCode;
                if(patchResponse.nHttpCode != 200)
                {
                    jsResponse["error"] = patchResponse.jsonData["error"];
                }
            }
            resp.jsonData.append(jsResponse);
        }
    }
    return resp;
}


std::shared_ptr<Sender> IS05Server::GetSender(const url& theUrl)
{
    auto vPath = SplitUrl(theUrl);
    if(vPath.size() > RESOURCE_ID)
    {
        return m_api.GetSender(vPath[RESOURCE_ID]);
    }
    else
    {
        return nullptr;
    }
}

std::shared_ptr<Receiver> IS05Server::GetReceiver(const url& theUrl)
{
    auto vPath = SplitUrl(theUrl);
    if(vPath.size() > RESOURCE_ID)
    {
        return m_api.GetReceiver(vPath[RESOURCE_ID]);
    }
    else
    {
        return nullptr;
    }
}
