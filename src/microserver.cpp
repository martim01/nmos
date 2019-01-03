#include "microserver.h"
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

using namespace std;

void MicroServer::RequestCompleted (void *cls, MHD_Connection* pConnection, void **ptr, enum MHD_RequestTerminationCode toe)
{
    Log::Get() << "Request completed" << endl;
    ConnectionInfo *pInfo = reinterpret_cast<ConnectionInfo*>(*ptr);
    if (pInfo)
    {
        delete pInfo;
        *ptr = NULL;
    }
    else
    {
        Log::Get(Log::ERROR) << "Request completed: Failed" << endl;
    }
}

int MicroServer::DoHttpGet(MHD_Connection* pConnection, string sUrl, ConnectionInfo* pInfo)
{
    string sResponse, sContentType;
    int nCode = pInfo->pServer->GetJson(sUrl, sResponse, sContentType);

    Log::Get() << "Response: " << sResponse << endl;

    MHD_Response* pResponse = MHD_create_response_from_buffer (sResponse.length(), (void *) sResponse.c_str(), MHD_RESPMEM_MUST_COPY);
    MHD_add_response_header(pResponse, "Content-Type", sContentType.c_str());
    int ret = MHD_queue_response (pConnection, nCode, pResponse);
    MHD_destroy_response (pResponse);
    return ret;
}

int MicroServer::DoHttpPut(MHD_Connection* pConnection, string sUrl, ConnectionInfo* pInfo)
{
    string sResponse;
    int nCode = pInfo->pServer->PutJson(sUrl, pInfo->pServer->GetPutData(), sResponse);
    pInfo->pServer->ResetPutData();


    MHD_Response* pResponse = MHD_create_response_from_buffer (sResponse.length(), (void *) sResponse.c_str(), MHD_RESPMEM_MUST_COPY);
    MHD_add_response_header(pResponse, "Content-Type", "application/json");
    int ret = MHD_queue_response (pConnection, nCode, pResponse);
    MHD_destroy_response (pResponse);
    return ret;
}

int MicroServer::DoHttpPatch(MHD_Connection* pConnection, string sUrl, ConnectionInfo* pInfo)
{
    Log::Get(Log::DEBUG) << "DoHttpPatch" << std::endl;

    string sResponse;
    int nCode = pInfo->pServer->PatchJson(sUrl, pInfo->pServer->GetPutData(), sResponse);
    pInfo->pServer->ResetPutData();

    MHD_Response* pResponse = MHD_create_response_from_buffer (sResponse.length(), (void *) sResponse.c_str(), MHD_RESPMEM_MUST_COPY);
    MHD_add_response_header(pResponse, "Content-Type", "application/json");
    int ret = MHD_queue_response (pConnection, nCode, pResponse);
    MHD_destroy_response (pResponse);
    return ret;
}

int MicroServer::AnswerToConnection(void *cls, MHD_Connection* pConnection, const char * url, const char * method, const char * sVersion, const char * upload_data, size_t * upload_data_size, void **ptr)
{
    string sMethod(method);
    if (NULL == *ptr)
    {
        Log::Get(Log::DEBUG) << "Initial connection" << endl;
        ConnectionInfo* pInfo = new ConnectionInfo();
        if(pInfo == 0)
        {
            return MHD_NO;
        }
        pInfo->pServer = reinterpret_cast<MicroServer*>(cls);
        if("PUT" == sMethod || "POST" == sMethod || "PATCH" == sMethod)
        {
            Log::Get(Log::DEBUG) << "Initial connection: " << sMethod << endl;
            if("PUT" == sMethod)
            {
                pInfo->nType = ConnectionInfo::PUT;
            }
            else if("POST" == sMethod)
            {
                pInfo->nType = ConnectionInfo::POST;
            }
            else if("PATCH" == sMethod)
            {
                pInfo->nType = ConnectionInfo::PATCH;
            }
        }
        else
        {
            Log::Get(Log::DEBUG) << "Initial connection: GET" << endl;
        }
        *ptr = (void *) pInfo;

        Log::Get(Log::DEBUG) << "Initial connection: return MHD_YES" << endl;
        return MHD_YES;
    }

    Log::Get(Log::DEBUG) << "MicroServer: " << url << endl;
    if("GET" == string(sMethod))
    {
        ConnectionInfo* pInfo = reinterpret_cast<ConnectionInfo*>(*ptr);
        Log::Get(Log::DEBUG) << "Actual connection: GET" << endl;
        return DoHttpGet(pConnection, url, pInfo);
    }
    else if("PUT" == string(sMethod))
    {
        Log::Get(Log::DEBUG) << "Actual connection: PUT" << endl;
        ConnectionInfo* pInfo = reinterpret_cast<ConnectionInfo*>(*ptr);
        if (*upload_data_size != 0)
        {
            pInfo->pServer->AddPutData(upload_data);
            *upload_data_size = 0;
            return MHD_YES;
        }
        else
        {
            return DoHttpPut(pConnection, url, pInfo);
        }
    }
    else if("PATCH" == string(sMethod))
    {
        Log::Get(Log::DEBUG) << "Actual connection: PATCH" << endl;
        ConnectionInfo* pInfo = reinterpret_cast<ConnectionInfo*>(*ptr);
        if (*upload_data_size != 0)
        {
            Log::Get(Log::DEBUG) << "Actual connection: PATCH - more" << endl;
            pInfo->pServer->AddPutData(upload_data);
            *upload_data_size = 0;
            return MHD_YES;
        }
        else
        {
            return DoHttpPatch(pConnection, url, pInfo);
        }
    }

    return MHD_NO;

}






bool MicroServer::Init(unsigned int nPort)
{
    m_nPort = nPort;
    m_pmhd = MHD_start_daemon (MHD_USE_SELECT_INTERNALLY, nPort, NULL, NULL, &MicroServer::AnswerToConnection, this, MHD_OPTION_NOTIFY_COMPLETED, RequestCompleted, NULL, MHD_OPTION_END);
    if(m_pmhd)
    {
        Log::Get() << "MicroServer: " << nPort << " Init: OK" << std::endl;
    }
    else
    {
        Log::Get() << "MicroServer: " << nPort << " Init: Failed" << std::endl;
    }

    return (m_pmhd!=0);
}

void MicroServer::Stop()
{
    if(m_pmhd)
    {
        MHD_stop_daemon (m_pmhd);
        m_pmhd = 0;
    }
}

MicroServer::MicroServer(shared_ptr<EventPoster> pPoster) : m_pPoster(pPoster), m_pmhd(0)
{

}

MicroServer::~MicroServer()
{
    Stop();
}

void MicroServer::AddPutData(string sData)
{
    m_sPut += sData;
}
string MicroServer::GetPutData() const
{
    return m_sPut;
}

void MicroServer::ResetPutData()
{
    m_sPut.clear();
}

void MicroServer::Wait()
{
    Log::Get(Log::DEBUG) << "Microserver: " << m_nPort << " Wait " << this_thread::get_id() << endl;
    std::unique_lock<std::mutex> lk(m_mutex);
    m_cvSync.wait(lk);
}

void MicroServer::Signal(bool bOk)
{
    lock_guard<mutex> lock(m_mutex);
    Log::Get(Log::DEBUG) << "Microserver: " << m_nPort << " Signal " << this_thread::get_id() << endl;
    m_bOk = bOk;
    m_cvSync.notify_one();
}

bool MicroServer::IsOk()
{
    lock_guard<mutex> lock(m_mutex);
    return m_bOk;
}




int MicroServer::GetJson(string sPath, string& sReturn, std::string& sContentType)
{
    transform(sPath.begin(), sPath.end(), sPath.begin(), ::tolower);
    sContentType = "application/json";

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

            nCode = GetJsonNmos(sReturn, sContentType);
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

int MicroServer::GetJsonNmos(string& sReturn, std::string& sContentType)
{
    Json::StyledWriter stw;
    if(m_vPath.size() == SZ_NMOS)
    {
        Json::Value jsNode;
        if(m_nPort == NodeApi::Get().GetConnectionPort())
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
    else if(m_vPath[API_TYPE] == "node" && m_nPort != NodeApi::Get().GetConnectionPort())
    {
        return GetJsonNmosNodeApi(sReturn);

    }
    else if(m_vPath[API_TYPE] == "connection" && NodeApi::Get().GetConnectionPort())
    {
        return GetJsonNmosConnectionApi(sReturn, sContentType);
    }
    sReturn = stw.write(GetJsonError(404, "API not found"));
    return 404;
}

int MicroServer::GetJsonNmosNodeApi(string& sReturn)
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
                sReturn  = stw.write(NodeApi::Get().GetSelf().GetJson());
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

Json::Value MicroServer::GetJsonSources()
{
    if(m_vPath.size() == SZ_ENDPOINT)
    {
        return NodeApi::Get().GetSources().GetJson();
    }
    else
    {
        map<string, Resource*>::const_iterator itSource = NodeApi::Get().GetSources().FindResource(m_vPath[RESOURCE]);
        if(itSource != NodeApi::Get().GetSources().GetResourceEnd())
        {
            return itSource->second->GetJson();
        }
    }
    return GetJsonError();
}

Json::Value MicroServer::GetJsonDevices()
{
    if(m_vPath.size() == SZ_ENDPOINT)
    {
        return NodeApi::Get().GetDevices().GetJson();
    }
    else
    {
        map<string, Resource*>::const_iterator itDevice = NodeApi::Get().GetDevices().FindResource(m_vPath[RESOURCE]);
        if(itDevice != NodeApi::Get().GetDevices().GetResourceEnd())
        {
            return itDevice->second->GetJson();
        }
    }
    return GetJsonError(404, "Device "+m_vPath[RESOURCE]+"does not exist.");
}

Json::Value MicroServer::GetJsonFlows()
{
    if(m_vPath.size() == SZ_ENDPOINT)
    {
        return NodeApi::Get().GetFlows().GetJson();
    }
    else
    {
        map<string, Resource*>::const_iterator itFlow = NodeApi::Get().GetFlows().FindResource(m_vPath[RESOURCE]);
        if(itFlow != NodeApi::Get().GetFlows().GetResourceEnd())
        {
            return itFlow->second->GetJson();
        }
    }
    return GetJsonError(404, "Flow "+m_vPath[RESOURCE]+"does not exist.");
}

Json::Value MicroServer::GetJsonReceivers()
{
    if(m_vPath.size() == SZ_ENDPOINT)
    {
        return NodeApi::Get().GetReceivers().GetJson();
    }
    else
    {
        map<string, Resource*>::const_iterator itReceiver = NodeApi::Get().GetReceivers().FindResource(m_vPath[RESOURCE]);
        if(itReceiver != NodeApi::Get().GetReceivers().GetResourceEnd())
        {
            return itReceiver->second->GetJson();
        }
    }
    return GetJsonError(404, "Receiver "+m_vPath[RESOURCE]+"does not exist.");
}

Json::Value MicroServer::GetJsonSenders()
{
    if(m_vPath.size() == SZ_ENDPOINT)
    {
        return NodeApi::Get().GetSenders().GetJson();
    }
    else
    {
        map<string, Resource*>::const_iterator itSender = NodeApi::Get().GetSenders().FindResource(m_vPath[RESOURCE]);
        if(itSender != NodeApi::Get().GetSenders().GetResourceEnd())
        {
            return itSender->second->GetJson();
        }
    }
    return GetJsonError(404, "Sender "+m_vPath[RESOURCE]+"does not exist.");
}


int MicroServer::GetJsonNmosConnectionApi(string& sReturn, std::string& sContentType)
{
    Json::StyledWriter stw;
    int nCode = 200;
    if(m_vPath.size() == SZ_API_TYPE)
    {
        sReturn = stw.write("v1.0/");
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
                    return GetJsonNmosConnectionSingleApi(sReturn, sContentType);

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

int MicroServer::GetJsonNmosConnectionSingleApi(std::string& sReturn, std::string& sContentType)
{
    int nCode(200);
    Json::StyledWriter stw;
    if(m_vPath.size() == SZC_TYPE)
    {
        Json::Value jsNode;
        jsNode.append("senders/");
        jsNode.append("receivers/");
        sReturn = stw.write(jsNode);
    }
    else if(m_vPath[C_DIRECTION] == "senders")
    {
        return GetJsonNmosConnectionSingleSenders(sReturn, sContentType);
    }
    else if(m_vPath[C_DIRECTION] == "receivers")
    {
        return GetJsonNmosConnectionSingleReceivers(sReturn);
    }
    else
    {
        nCode = 404;
        sReturn = stw.write(GetJsonError(404, "Endpoint not found"));
    }
    return nCode;
}


int MicroServer::GetJsonNmosConnectionSingleSenders(std::string& sReturn, std::string& sContentType)
{
    int nCode(200);
    Json::StyledWriter stw;
    if(m_vPath.size() == SZC_DIRECTION)
    {
        sReturn = stw.write(NodeApi::Get().GetSenders().GetConnectionJson());
    }
    else
    {
        map<string, Resource*>::const_iterator itResource = NodeApi::Get().GetSenders().FindResource(m_vPath[SZC_DIRECTION]);
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
                Sender* pSender = dynamic_cast<Sender*>(itResource->second);
                if(m_vPath[C_LAST] == "constraints")
                {
                    sReturn = stw.write(pSender->GetConnectionConstraintsJson());
                }
                else if(m_vPath[C_LAST] == "staged")
                {
                    sReturn = stw.write(pSender->GetConnectionStagedJson());
                }
                else if(m_vPath[C_LAST] == "active")
                {
                    sReturn = stw.write(pSender->GetConnectionActiveJson());
                }
                else if(m_vPath[C_LAST] == "transportfile")
                {
                    // transportfile get
                    sContentType = "application/sdp";
                    sReturn = pSender->GetTransportFile();
                    nCode = 200;
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

int MicroServer::GetJsonNmosConnectionSingleReceivers(std::string& sReturn)
{
    int nCode(200);
    Json::StyledWriter stw;
    if(m_vPath.size() == SZC_DIRECTION)
    {
        sReturn = stw.write(NodeApi::Get().GetReceivers().GetConnectionJson());
    }
    else
    {
        map<string, Resource*>::const_iterator itResource = NodeApi::Get().GetReceivers().FindResource(m_vPath[SZC_DIRECTION]);
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
                if(m_vPath[C_LAST] == "constraints")
                {
                    sReturn = stw.write(dynamic_cast<Receiver*>(itResource->second)->GetConnectionConstraintsJson());
                }
                else if(m_vPath[C_LAST] == "staged")
                {
                    sReturn = stw.write(dynamic_cast<Receiver*>(itResource->second)->GetConnectionStagedJson());
                }
                else if(m_vPath[C_LAST] == "active")
                {
                    sReturn = stw.write(dynamic_cast<Receiver*>(itResource->second)->GetConnectionActiveJson());
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


int MicroServer::GetJsonNmosConnectionBulkApi(std::string& sReturn)
{
    int nCode(200);
    Json::StyledWriter stw;
    if(m_vPath.size() == SZC_TYPE)
    {
        Json::Value jsNode;
        jsNode.append("senders/");
        jsNode.append("receivers/");
        sReturn = stw.write(jsNode);
    }
    else if(m_vPath[SZC_TYPE] == "senders")
    {
        // @todo bulk get senders
    }
    else if(m_vPath[SZC_TYPE] == "receivers")
    {
        // @todo bulk get receivers
    }
    else
    {
        nCode = 404;
        sReturn = stw.write(GetJsonError(404, "Endpoint not found"));
    }
    return nCode;
}

Json::Value MicroServer::GetJsonError(unsigned long nCode, string sError)
{
    Json::Value jsError(Json::objectValue);
    jsError["code"] = (int)nCode;
    jsError["error"] = sError;
    jsError["debug"] = "null";
    return jsError;
}





int MicroServer::PutJson(string sPath, const string& sJson, string& sResponse)
{
    //make sure path is correct
    transform(sPath.begin(), sPath.end(), sPath.begin(), ::tolower);

    Json::StyledWriter stw;

    int nCode = 202;
    SplitString(m_vPath, sPath, '/');
    if(m_vPath.size() < SZ_TARGET)
    {
        nCode = 405;
        sResponse = stw.write(GetJsonError(nCode, "Method not allowed here."));
    }
    else
    {
        if(m_vPath[NMOS] == "x-nmos" && m_vPath[API_TYPE] == "node" &&  NodeApi::Get().GetSelf().IsVersionSupported(m_vPath[VERSION]) && m_vPath[ENDPOINT] == "receivers" && m_vPath[TARGET] == "target")
        {
            //does the receiver exist?
            Receiver* pReceiver  = NodeApi::Get().GetReceiver(m_vPath[RESOURCE]);
            if(!pReceiver)
            {
                nCode = 404;
                sResponse = stw.write(GetJsonError(nCode, "Resource "+m_vPath[RESOURCE]+"does not exist."));
            }
            else
            {
                Log::Get(Log::DEBUG) << sJson << std::endl;
                Json::Value jsRequest;
                Json::Reader jsReader;
                if(jsReader.parse(sJson, jsRequest) == false)
                {
                    nCode = 400;
                    sResponse = stw.write(GetJsonError(nCode, "Request is ill defined."));
                }
                else
                {
                    //Have we been sent a sender??
                    std::shared_ptr<Sender> pSender = std::make_shared<Sender>(jsRequest);
                    if(pSender->IsOk() == false)
                    {
                        nCode = 400;
                        sResponse = stw.write(GetJsonError(nCode, "Request is ill defined or missing mandatory attributes."));
                    }

                    if(m_pPoster)
                    {
                        //Tell the main thread to connect
                        m_pPoster->_Target(m_vPath[RESOURCE], pSender, m_nPort);
                        //Pause the HTTP thread
                        Wait();

                        if(IsOk())
                        {   //this means the main thread has connected the receiver to the sender
                            nCode = 202;
                            pReceiver->SetSender(pSender);
                            NodeApi::Get().Commit();   //updates the registration node or txt records

                            sResponse = stw.write(pSender->GetJson());
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
                        sResponse = stw.write(pSender->GetJson());
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


int MicroServer::PatchJson(string sPath, const string& sJson, string& sResponse)
{
    Log::Get(Log::DEBUG) << "PatchJson" << std::endl;

    //make sure path is correct
    transform(sPath.begin(), sPath.end(), sPath.begin(), ::tolower);

    Json::StyledWriter stw;

    int nCode = 202;
    SplitString(m_vPath, sPath, '/');

    if(m_vPath.size() < SZC_LAST || NodeApi::Get().GetConnectionPort() != m_nPort || m_vPath[NMOS] != "x-nmos" || m_vPath[API_TYPE] != "connection" || m_vPath[VERSION] != "v1.0" || m_vPath[C_TYPE] != "single" || (m_vPath[C_DIRECTION] != "senders" && m_vPath[C_DIRECTION] != "receivers") || m_vPath[C_LAST] != "staged")
    {
        nCode = 405;
        sResponse = stw.write(GetJsonError(nCode, "Method not allowed here."));
    }
    else
    {
        if(m_vPath[C_DIRECTION] == "senders")
        {
            return PatchJsonSender(sJson, sResponse);
        }
        else
        {
            return PatchJsonReceiver(sJson, sResponse);
        }
    }
    return nCode;
}

int MicroServer::PatchJsonSender(const std::string& sJson, std::string& sResponse)
{
    Log::Get(Log::DEBUG) << "PatchJsonSender" << std::endl;
    int nCode(200);
    Json::StyledWriter stw;
    //does the sender exist?
    Sender* pSender = NodeApi::Get().GetSender(m_vPath[C_ID]);
    if(!pSender)
    {
        nCode = 404;
        sResponse = stw.write(GetJsonError(nCode, "Resource does not exist."));
        Log::Get(Log::DEBUG) << "PatchJsonSender: Resource does not exist." << std::endl;
    }
    else
    {
        Log::Get(Log::DEBUG) << "PatchJsonSender: --------------------------------" << std::endl;
        Log::Get(Log::DEBUG) << sJson << std::endl;
        Log::Get(Log::DEBUG) << "PatchJsonSender: --------------------------------" << std::endl;
        //is the json valid?
        Json::Value jsRequest;
        Json::Reader jsReader;
        if(jsReader.parse(sJson, jsRequest) == false)
        {
            nCode = 400;
            sResponse = stw.write(GetJsonError(nCode, "Request is ill defined."));

            Log::Get(Log::DEBUG) << "PatchJsonSender: Request is ill defined." << std::endl;
        }
        else
        {
            connectionSender conRequest(pSender->GetStaged());
            //can we patch a connection from the json?
            if(conRequest.Patch(jsRequest) == false)
            {
                nCode = 400;
                sResponse = stw.write(GetJsonError(nCode, "Request does not meet schema."));
                Log::Get(Log::DEBUG) << "PatchJsonSender: Request does not meet schema." << std::endl;
            }
            else if(pSender->CheckConstraints(conRequest) == false)
            {//does the patched connection meet the sender constraints?
                nCode = 400;
                sResponse = stw.write(GetJsonError(nCode, "Request does not meet sender's constraints."));
                Log::Get(Log::DEBUG) << "PatchJsonSender: Request does not meet sender's constraints." << std::endl;
            }
            else if(conRequest.eActivate != connection::ACT_NULL && pSender->IsLocked() == true)
            {   //locked by previous staged activation
                nCode = 423;
                sResponse = stw.write(GetJsonError(nCode, "Sender had pending activation."));
                Log::Get(Log::DEBUG) << "PatchJsonSender: Sender had pending activation." << std::endl;
            }
            else if(m_pPoster)
            {   //tell the main thread and wait to see what happens
                //the main thread should check that it can definitely do what the patch says and simply signal true or false
                m_pPoster->_PatchSender(m_vPath[C_ID], conRequest, m_nPort);
                //Pause the HTTP thread
                Wait();

                if(IsOk() && pSender->Stage(conRequest, m_pPoster)) //PATCH the sender
                {
                    nCode = 202;
                    if(conRequest.eActivate == connection::ACT_NULL || conRequest.eActivate == connection::ACT_NOW)
                    {
                        nCode = 200;
                    }
                    sResponse = stw.write(pSender->GetConnectionStagedJson());
                    Log::Get(Log::DEBUG) << sResponse << std::endl;
                }
                else
                {
                    nCode = 500;
                    sResponse = stw.write(GetJsonError(nCode, "Sender unable to stage PATCH as no EventPoster or activation time invalid."));
                    Log::Get(Log::DEBUG) << "PatchJsonSender: Sender unable to stage PATCH as no EventPoster or activation time invalid." << std::endl;
                }
            }
            else
            {   //no way of telling main thread to do this so we'll simply assume it's been done
                if(pSender->Stage(conRequest, m_pPoster)) //PATCH the sender
                {
                    sResponse = stw.write(pSender->GetConnectionStagedJson());
                    Log::Get(Log::DEBUG) << sResponse << std::endl;
                }
                else
                {
                    nCode = 500;
                    sResponse = stw.write(GetJsonError(nCode, "Sender unable to stage PATCH as no EventPoster or activation time invalid."));
                    Log::Get(Log::DEBUG) << "PatchJsonSender: Sender unable to stage PATCH as no EventPoster or activation time invalid." << std::endl;
                }
            }
        }
    }
    return nCode;
}

int MicroServer::PatchJsonReceiver(const std::string& sJson, std::string& sResponse)
{
    int nCode(200);
    Json::StyledWriter stw;
    //does the sender exist?
    Receiver* pReceiver = NodeApi::Get().GetReceiver(m_vPath[C_ID]);
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

            Log::Get(Log::DEBUG) << "PatchJsonReceiver: Request is ill defined." << std::endl;
        }
        else
        {
            connectionReceiver conRequest(pReceiver->GetStaged());
            //can we patch a connection from the json?
            if(conRequest.Patch(jsRequest) == false)
            {
                nCode = 400;
                sResponse = stw.write(GetJsonError(nCode, "Request does not meet schema."));
                Log::Get(Log::DEBUG) << "PatchJsonReceiver: Request does not meet schema." << std::endl;
            }
            else if(pReceiver->CheckConstraints(conRequest) == false)
            {//does the patched connection meet the Receiver constraints?
                nCode = 400;
                sResponse = stw.write(GetJsonError(nCode, "Request does not meet Receiver's constraints."));
                Log::Get(Log::DEBUG) << "PatchJsonReceiver: Request does not meet Receiver's constraints." << std::endl;
            }
            else if(conRequest.eActivate != connection::ACT_NULL && pReceiver->IsLocked() == true)
            {   //locked by previous staged activation
                nCode = 423;
                sResponse = stw.write(GetJsonError(nCode, "Receiver had pending activation."));
                Log::Get(Log::DEBUG) << "PatchJsonReceiver: Receiver had pending activation." << std::endl;
            }
            else if(m_pPoster)
            {   //tell the main thread and wait to see what happens
                //the main thread should check that it can definitely do what the patch says and simply signal true or false
                m_pPoster->_PatchReceiver(m_vPath[C_ID], conRequest, m_nPort);
                //Pause the HTTP thread
                Wait();

                if(IsOk() && pReceiver->Stage(conRequest, m_pPoster)) //PATCH the Receiver
                {
                    nCode = 202;
                    if(conRequest.eActivate == connection::ACT_NULL || conRequest.eActivate == connection::ACT_NOW)
                    {
                        nCode = 200;
                    }
                    sResponse = stw.write(pReceiver->GetConnectionStagedJson());
                    Log::Get(Log::DEBUG) << sResponse << std::endl;
                }
                else
                {
                    nCode = 500;
                    sResponse = stw.write(GetJsonError(nCode, "Receiver unable to stage PATCH as no EventPoster or activation time invalid."));
                    Log::Get(Log::DEBUG) << "PatchJsonReceiver: Receiver unable to stage PATCH as no EventPoster or activation time invalid." << std::endl;
                }
            }
            else
            {   //no way of telling main thread to do this so we'll simply assume it's been done
                if(pReceiver->Stage(conRequest, m_pPoster)) //PATCH the Receiver
                {
                    sResponse = stw.write(pReceiver->GetConnectionStagedJson());
                    Log::Get(Log::DEBUG) << sResponse << std::endl;
                }
                else
                {
                    nCode = 500;
                    sResponse = stw.write(GetJsonError(nCode, "Receiver unable to stage PATCH as no EventPoster or activation time invalid."));
                    Log::Get(Log::DEBUG) << "PatchJsonReceiver: Sender unable to stage PATCH as no EventPoster or activation time invalid." << std::endl;
                }
            }
        }
    }
    return nCode;
}

void MicroServer::SplitString(vector<string>& vSplit, string str, char cSplit)
{

    vSplit.clear();

    istringstream f(str);
    string s;
    while (getline(f, s, cSplit))
    {
        if(s.empty() == false || vSplit.empty())    //we don't want any empty parts apart from the base one
        {
            vSplit.push_back(s);
        }
    }

}
