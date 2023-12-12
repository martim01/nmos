#ifdef __NMOS_REGISTRY__
#include "registryserver.h"
#include "registryapi.h"
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
#ifdef __GNU__
#include <netinet/in.h>
#include <arpa/inet.h>
#endif // __GNU__

using namespace std;

void RegistryServer::RequestCompleted (void *cls, MHD_Connection* pConnection, void **ptr, enum MHD_RequestTerminationCode toe)
{
    pmlLog(pml::LOG_INFO, "pml::nmos") << "Request completed" ;
    RegistryInfo *pInfo = reinterpret_cast<RegistryInfo*>(*ptr);
    if (pInfo)
    {
        delete pInfo;
        *ptr = NULL;
    }
    else
    {
        pmlLog(pml::LOG_ERROR, "pml::nmos") << "Request completed: Failed" ;
    }
}

int RegistryServer::DoHttpGet(MHD_Connection* pConnection, string sUrl, RegistryInfo* pInfo)
{
    string sResponse, sContentType;
    int nCode = pInfo->pServer->GetJson(sUrl, sResponse, sContentType);

    pmlLog(pml::LOG_INFO, "pml::nmos") << "Response: " << sResponse ;

    MHD_Response* pResponse = MHD_create_response_from_buffer (sResponse.length(), (void *) sResponse.c_str(), MHD_RESPMEM_MUST_COPY);
    MHD_add_response_header(pResponse, "Content-Type", sContentType.c_str());
    MHD_add_response_header(pResponse, "Access-Control-Allow-Origin:", "*");
    MHD_add_response_header(pResponse, "Access-Control-Allow-Methods:", "GET, PUT, POST, HEAD, OPTIONS, DELETE");
    MHD_add_response_header(pResponse, "Access-Control-Allow-Headers:", "Content-Type, Accept");
    MHD_add_response_header(pResponse, "Access-Control-Max-Age:", "3600");
    int ret = MHD_queue_response (pConnection, nCode, pResponse);
    MHD_destroy_response (pResponse);
    return ret;
}

int RegistryServer::DoHttpPost(MHD_Connection* pConnection, string sUrl, RegistryInfo* pInfo)
{
    string sResponse, sLocation;
    int nCode = pInfo->pServer->PostJson(sUrl, pInfo->pServer->GetPutData(), sResponse, sLocation);
    pInfo->pServer->ResetPutData();


    MHD_Response* pResponse = MHD_create_response_from_buffer (sResponse.length(), (void *) sResponse.c_str(), MHD_RESPMEM_MUST_COPY);
    MHD_add_response_header(pResponse, "Content-Type", "application/json");
    if(sLocation.empty() == false)
    {
        MHD_add_response_header(pResponse, "Location", sLocation.c_str());
    }
    MHD_add_response_header(pResponse, "Access-Control-Allow-Origin:", "*");
    MHD_add_response_header(pResponse, "Access-Control-Allow-Methods:", "GET, PUT, POST, HEAD, OPTIONS, DELETE");
    MHD_add_response_header(pResponse, "Access-Control-Allow-Headers:", "Content-Type, Accept");
    MHD_add_response_header(pResponse, "Access-Control-Max-Age:", "3600");
    int ret = MHD_queue_response (pConnection, nCode, pResponse);
    MHD_destroy_response (pResponse);
    return ret;
}

int RegistryServer::DoHttpDelete(MHD_Connection* pConnection, string sUrl, RegistryInfo* pInfo)
{
    string sResponse;
    int nCode = pInfo->pServer->DeleteJson(sUrl, sResponse);

    pmlLog(pml::LOG_INFO, "pml::nmos") << "Response: " << sResponse ;

    MHD_Response* pResponse = MHD_create_response_from_buffer (sResponse.length(), (void *) sResponse.c_str(), MHD_RESPMEM_MUST_COPY);
    MHD_add_response_header(pResponse, "Content-Type", "application/json");
    MHD_add_response_header(pResponse, "Access-Control-Allow-Origin:", "*");
    MHD_add_response_header(pResponse, "Access-Control-Allow-Methods:", "GET, PUT, POST, HEAD, OPTIONS, DELETE");
    MHD_add_response_header(pResponse, "Access-Control-Allow-Headers:", "Content-Type, Accept");
    MHD_add_response_header(pResponse, "Access-Control-Max-Age:", "3600");
    int ret = MHD_queue_response (pConnection, nCode, pResponse);
    MHD_destroy_response (pResponse);
    return ret;
}

int RegistryServer::AnswerToConnection(void *cls, MHD_Connection* pConnection, const char * url, const char * method, const char * sVersion, const char * upload_data, size_t * upload_data_size, void **ptr)
{
    sockaddr_in* pAddr = (sockaddr_in*)MHD_get_connection_info(pConnection, MHD_CONNECTION_INFO_CLIENT_ADDRESS)->client_addr;
    char sAddr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(pAddr->sin_addr), sAddr, INET_ADDRSTRLEN);

    pmlLog(pml::LOG_DEBUG, "pml::nmos") << "AnswerToConnection: " << sAddr ;

    string sMethod(method);
    if (NULL == *ptr)
    {
        pmlLog(pml::LOG_DEBUG, "pml::nmos") << "Initial connection" ;
        RegistryInfo* pInfo = new RegistryInfo();
        if(pInfo == 0)
        {
            return MHD_NO;
        }
        pInfo->pServer = reinterpret_cast<RegistryServer*>(cls);
        if("POST" == sMethod)
        {
            pmlLog(pml::LOG_DEBUG, "pml::nmos") << "Initial connection: " << sMethod ;
            if("POST" == sMethod)
            {
                pInfo->nType = RegistryInfo::POST;
            }
        }
        else if("DELETE" == sMethod)
        {
            pInfo->nType = RegistryInfo::DEL;
            pmlLog(pml::LOG_DEBUG, "pml::nmos") << "Initial connection: " << sMethod ;
        }
        else
        {
            pmlLog(pml::LOG_DEBUG, "pml::nmos") << "Initial connection: GET" ;
        }
        *ptr = (void *) pInfo;

        pmlLog(pml::LOG_DEBUG, "pml::nmos") << "Initial connection: return MHD_YES" ;
        return MHD_YES;
    }

    pmlLog(pml::LOG_DEBUG, "pml::nmos") << "RegistryServer: " << url ;
    if("GET" == string(sMethod))
    {
        RegistryInfo* pInfo = reinterpret_cast<RegistryInfo*>(*ptr);
        pmlLog(pml::LOG_DEBUG, "pml::nmos") << "Actual connection: GET" ;
        return DoHttpGet(pConnection, url, pInfo);
    }
    else if("DELETE" == string(sMethod))
    {
        RegistryInfo* pInfo = reinterpret_cast<RegistryInfo*>(*ptr);
        pmlLog(pml::LOG_DEBUG, "pml::nmos") << "Actual connection: DELETE" ;
        return DoHttpDelete(pConnection, url, pInfo);
    }
    else if("POST" == string(sMethod))
    {
        pmlLog(pml::LOG_DEBUG, "pml::nmos") << "Actual connection: POST" ;
        RegistryInfo* pInfo = reinterpret_cast<RegistryInfo*>(*ptr);
        if (*upload_data_size != 0)
        {
            pInfo->pServer->AddPutData(upload_data);
            *upload_data_size = 0;
            return MHD_YES;
        }
        else
        {
            return DoHttpPost(pConnection, url, pInfo);
        }
    }

    return MHD_NO;

}






bool RegistryServer::Init(unsigned int nPort)
{
    m_nPort = nPort;
    m_pmhd = MHD_start_daemon (MHD_USE_SELECT_INTERNALLY
#if EPOLL_SUPPORT
| MHD_USE_EPOLL_INTERNALLY_LINUX_ONLY
#endif // EPOLL_SUPPORT,
, nPort, NULL, NULL, &RegistryServer::AnswerToConnection, this, MHD_OPTION_NOTIFY_COMPLETED, RequestCompleted, NULL, MHD_OPTION_THREAD_POOL_SIZE,4, MHD_OPTION_END);
    if(m_pmhd)
    {
        pmlLog(pml::LOG_INFO, "pml::nmos") << "RegistryServer: " << nPort << " Init: OK" ;
    }
    else
    {
        pmlLog(pml::LOG_INFO, "pml::nmos") << "RegistryServer: " << nPort << " Init: Failed" ;
    }

    return (m_pmhd!=0);
}

void RegistryServer::Stop()
{
    if(m_pmhd)
    {
        MHD_stop_daemon (m_pmhd);
        m_pmhd = 0;
    }
}

RegistryServer::RegistryServer() : m_pmhd(0)
{

}

RegistryServer::~RegistryServer()
{
    Stop();
}

void RegistryServer::AddPutData(string sData)
{
    m_sPut += sData;
}
string RegistryServer::GetPutData() const
{
    return m_sPut;
}

void RegistryServer::ResetPutData()
{
    m_sPut.clear();
}





int RegistryServer::GetJson(string sPath, string& sReturn, std::string& sContentType)
{
    transform(sPath.begin(), sPath.end(), sPath.begin(), ::tolower);
    sContentType = "application/json";

    int nCode = 200;
    SplitString(m_vPath, sPath, '/');


    if(m_vPath.size() <= SZ_BASE)
    {
        Json::Value jsNode;
        jsNode.append("x-nmos/");
        sReturn = ConvertFromJson(jsNode);
    }
    else
    {
        if(m_vPath[NMOS] == "x-nmos")
        {   //check on nmos

            nCode = GetJsonNmos(sReturn, sContentType);
        }
        else
        {
            sReturn = ConvertFromJson(GetJsonError(404, "Page not found"));
            nCode = 404;
        }
    }
    return nCode;
}

int RegistryServer::GetJsonNmos(string& sReturn, std::string& sContentType)
{
    if(m_vPath.size() == SZ_NMOS)
    {
        Json::Value jsNode;
        jsNode.append("registration/");
        sReturn = ConvertFromJson(jsNode);
        return 200;
    }
    else if(m_vPath[API] == "registration")
    {
        if(m_vPath.size() == SZ_API)
        {
            Json::Value jsNode;
            jsNode.append("v1.2/");
            sReturn = ConvertFromJson(jsNode);
            return 200;
        }
        else if(m_vPath[VERSION] == "v1.2")
        {
            if(m_vPath.size() == SZ_VERSION)
            {
                Json::Value jsNode;
                jsNode.append("resource/");
                jsNode.append("health/");
                sReturn = ConvertFromJson(jsNode);
                return 200;
            }
            else if(m_vPath[RES_HEALTH] == "resource")
            {
                return GetJsonNmosResource(sReturn, ApiVersion(1,2));
            }
            else if(m_vPath[API] == "health")
            {
                return GetJsonNmosHealth(sReturn);
            }
        }
        else
        {
            sReturn = ConvertFromJson(GetJsonError(404, "Version not supported"));
            return 404;
        }
    }
    sReturn = ConvertFromJson(GetJsonError(404, "API not found"));
    return 404;
}

int RegistryServer::GetJsonNmosResource(string& sReturn, const ApiVersion& version)
{
    int nCode = 200;
    if(m_vPath.size() < SZ_ID)
    {
        //no GET allowed here
        nCode = 405;
        sReturn = ConvertFromJson(GetJsonError(nCode, "Method not allowed here."));
    }
    else
    {
        const shared_ptr<Resource> pResource = RegistryApi::Get().FindNmosResource(m_vPath[RESOURCE_TYPE], m_vPath[RESOURCE_ID]);
        if(pResource)
        {
            nCode = 200;
            sReturn = ConvertFromJson(pResource->GetJson(version));
        }
        else
        {
            nCode = 404;
            sReturn = ConvertFromJson(GetJsonError(404, "Resource not found"));
        }
    }
    return nCode;
}

int RegistryServer::GetJsonNmosHealth(string& sReturn)
{
    int nCode = 200;
    if(m_vPath.size() < SZ_ID)
    {
        //no GET allowed here
        nCode = 405;
        sReturn = ConvertFromJson(GetJsonError(nCode, "Method not allowed here."));
    }
    else
    {
        if(m_vPath[HEALTH_NODES] == "nodes")
        {
            const shared_ptr<Resource> pResource = RegistryApi::Get().FindNmosResource("node", m_vPath[HEALTH_ID]);
            if(pResource)
            {
                nCode = 200;
                Json::Value jsNode;
                jsNode["health"] = to_string(pResource->GetLastHeartbeat());
                sReturn = ConvertFromJson(jsNode);
            }
            else
            {
                nCode = 404;
                sReturn = ConvertFromJson(GetJsonError(404, "Resource not found"));
            }
        }
        else
        {
            nCode = 400;
            sReturn = ConvertFromJson(GetJsonError(400, "Page not found"));
        }
    }
    return nCode;
}


Json::Value RegistryServer::GetJsonError(unsigned long nCode, string sError)
{
    Json::Value jsError(Json::objectValue);
    jsError["code"] = (int)nCode;
    jsError["error"] = sError;
    jsError["debug"] = "null";
    return jsError;
}



int RegistryServer::PostJson(string sPath, const string& sJson, string& sResponse, std::string& sLocation)
{
    pmlLog(pml::LOG_INFO, "pml::nmos") << "RegistyServer: " << sPath << " " << sJson ;
    //make sure path is correct
    transform(sPath.begin(), sPath.end(), sPath.begin(), ::tolower);


    int nCode = 202;
    SplitString(m_vPath, sPath, '/');
    if(m_vPath.size() < SZ_RES_HEALTH || m_vPath[NMOS] != "x-nmos" || m_vPath[API]!="registration" || m_vPath[VERSION] != "v1.2")
    {
        nCode = 405;
        sResponse = ConvertFromJson(GetJsonError(nCode, "Method not allowed here."));
    }
    else if(m_vPath[RES_HEALTH] == "resource" && m_vPath.size()==SZ_RES_HEALTH)
    {
        //Add a new/update a resource
        nCode = PostJsonNmosResource(sJson, sResponse, sLocation);
    }
    else if(m_vPath.size()==SZ_ID && m_vPath[NMOS] == "x-nmos" && m_vPath[RES_HEALTH] == "health" && m_vPath[HEALTH_NODES] == "nodes")
    {
        nCode = PostJsonNmosHealth(sResponse);
    }
    else
    {
        nCode = 405;
        sResponse = ConvertFromJson(GetJsonError(nCode, "Method not allowed here."));
    }
    pmlLog(pml::LOG_INFO, "pml::nmos") << "Response: " << sResponse << " Code:" << nCode ;
    return nCode;
}


int RegistryServer::PostJsonNmosResource(const std::string& sJson, std::string& sReturn, std::string& sLocation)
{
    int nCode(200);
    //Is sJson Json
    Json::Value jsRequest;
    Json::Reader jsReader;
    if(jsReader.parse(sJson, jsRequest) == false || jsRequest["type"].isString()==false || jsRequest["data"].isObject()==false || jsRequest["data"]["id"].isString() == false)
    {
        nCode = 400;
        sReturn = ConvertFromJson(GetJsonError(nCode, "Request is ill defined."));
        return nCode;
    }

    //does a resource already exist with the given id??
    string sError;
    nCode = RegistryApi::Get().AddUpdateResource(jsRequest["type"].asString(), jsRequest["data"], sError);

    pmlLog(pml::LOG_INFO, "pml::nmos") << "RESOURCE TYPE: " << jsRequest["type"].asString() ;

    if(nCode == 200 || nCode == 201)
    {
        sReturn = ConvertFromJson(jsRequest["data"]);
        stringstream sLoc;
        sLoc << "/x-nmos/registration/v1.2/resource/" << jsRequest["type"].asString() << "/" << jsRequest["data"]["id"].asString() << "/";
        sLocation = sLoc.str();
    }
    else
    {
        sReturn = ConvertFromJson(GetJsonError(nCode, sError));
    }

    return nCode;

}

int RegistryServer::PostJsonNmosHealth(std::string& sReturn)
{
    unsigned short nCode;

    size_t nHeatbeat = RegistryApi::Get().Heartbeat(m_vPath[HEALTH_ID]);
    if(nHeatbeat != 0)
    {
        Json::Value jsNode;
        jsNode["health"] = to_string(nHeatbeat);
        sReturn = ConvertFromJson(jsNode);
        nCode = 200;
    }
    else
    {
        nCode = 404;
        sReturn = ConvertFromJson(GetJsonError(404, "Resource not found"));
    }
    return nCode;
}


int RegistryServer::DeleteJson(string sPath, string& sResponse)
{
    //make sure path is correct
    transform(sPath.begin(), sPath.end(), sPath.begin(), ::tolower);


    int nCode = 204;
    SplitString(m_vPath, sPath, '/');
    if(m_vPath.size() < SZ_ID || m_vPath[NMOS] != "x-nmos" || m_vPath[API]!="registration" || m_vPath[VERSION] != "v1.2" || m_vPath[RES_HEALTH] != "resource")
    {
        nCode = 405;
        sResponse = ConvertFromJson(GetJsonError(nCode, "Method not allowed here."));
    }
    else
    {
        if(RegistryApi::Get().DeleteResource(m_vPath[RESOURCE_TYPE], m_vPath[RESOURCE_ID]))
        {
            nCode = 204;
            sResponse.clear();

        }
        else
        {
            nCode = 404;
            sResponse = ConvertFromJson-(GetJsonError(nCode, "No resource found with this id."));
        }
    }

    return nCode;
}
#endif //__NMOS_REGISTRY__
