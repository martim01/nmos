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
#include "utils.h"
#include "curlregister.h"
#include "nmosserver.h"
#ifdef __GNU__
#include <netinet/in.h>
#include <arpa/inet.h>
#endif // __GNU__

using namespace std;

void MicroServer::RequestCompleted (void *cls, MHD_Connection* pConnection, void **ptr, enum MHD_RequestTerminationCode toe)
{
    Log::Get(Log::LOG_DEBUG) << "Request completed" << endl;
    ConnectionInfo *pInfo = reinterpret_cast<ConnectionInfo*>(*ptr);
    if (pInfo)
    {
        delete pInfo;
        *ptr = NULL;
    }
    else
    {
        Log::Get(Log::LOG_ERROR) << "Request completed: Failed" << endl;
    }
}

int MicroServer::DoReply(MHD_Connection* pConnection, int nCode, const std::string& sResponse, const std::string& sContentType)
{
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

int MicroServer::AnswerToConnection(void *cls, MHD_Connection* pConnection, const char * url, const char * method, const char * sVersion, const char * upload_data, size_t * upload_data_size, void **ptr)
{
    sockaddr* pAddr = MHD_get_connection_info(pConnection, MHD_CONNECTION_INFO_CLIENT_ADDRESS)->client_addr;
    char sAddr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, pAddr, sAddr, INET_ADDRSTRLEN);

    Log::Get(Log::LOG_DEBUG) << "AnswerToConnection: " << sAddr << endl;

    string sMethod(method);
    if (NULL == *ptr)
    {
        Log::Get(Log::LOG_DEBUG) << "Initial connection" << endl;
        ConnectionInfo* pInfo = new ConnectionInfo();
        if(pInfo == 0)
        {
            return MHD_NO;
        }
        pInfo->pServer = reinterpret_cast<MicroServer*>(cls);
        if("PUT" == sMethod || "POST" == sMethod || "PATCH" == sMethod)
        {
            Log::Get(Log::LOG_DEBUG) << "Initial connection: " << sMethod << endl;
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
            Log::Get(Log::LOG_DEBUG) << "Initial connection: GET" << endl;
        }
        *ptr = (void *) pInfo;

        Log::Get(Log::LOG_DEBUG) << "Initial connection: return MHD_YES" << endl;
        return MHD_YES;
    }

    Log::Get(Log::LOG_DEBUG) << "MicroServer: " << url << endl;
    if("GET" == string(sMethod))
    {
        ConnectionInfo* pInfo = reinterpret_cast<ConnectionInfo*>(*ptr);
        Log::Get(Log::LOG_DEBUG) << "Actual connection: GET" << endl;
        return DoHttpGet(pConnection, url, pInfo);
    }
    else if("PUT" == string(sMethod))
    {
        Log::Get(Log::LOG_DEBUG) << "Actual connection: PUT" << endl;
        ConnectionInfo* pInfo = reinterpret_cast<ConnectionInfo*>(*ptr);
        if (*upload_data_size != 0)
        {
            Log::Get(Log::LOG_DEBUG) << "Actual connection: PUT: " << upload_data << endl;
            pInfo->ssData << upload_data;
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
        Log::Get(Log::LOG_DEBUG) << "Actual connection: PATCH" << endl;
        ConnectionInfo* pInfo = reinterpret_cast<ConnectionInfo*>(*ptr);
        if (*upload_data_size != 0)
        {
            Log::Get(Log::LOG_DEBUG) << "Actual connection: PATCH - more" << endl;
            pInfo->ssData << upload_data;
            *upload_data_size = 0;
            return MHD_YES;
        }
        else
        {
            return DoHttpPatch(pConnection, url, pInfo);
        }
    }
    else if("POST" == string(sMethod))
    {
        ConnectionInfo* pInfo = reinterpret_cast<ConnectionInfo*>(*ptr);
        Log::Get(Log::LOG_DEBUG) << "Actual connection: POST" << endl;
        if (*upload_data_size != 0)
        {
            Log::Get(Log::LOG_DEBUG) << "Actual connection: POST - more" << endl;
            pInfo->ssData << upload_data;
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




int MicroServer::DoHttpGet(MHD_Connection* pConnection, const std::string& sUrl, ConnectionInfo* pInfo)
{
    string sResponse, sContentType;
    int nCode = pInfo->pServer->GetJson(sUrl, sResponse, sContentType);

    return DoReply(pConnection, nCode, sResponse, sContentType);
}

int MicroServer::DoHttpPut(MHD_Connection* pConnection, const std::string& sUrl, ConnectionInfo* pInfo)
{
    string sResponse;
    int nCode = pInfo->pServer->PutJson(sUrl, pInfo->ssData.str(), sResponse);

    return DoReply(pConnection, nCode, sResponse);
}

int MicroServer::DoHttpPatch(MHD_Connection* pConnection, const std::string& sUrl, ConnectionInfo* pInfo)
{
    string sResponse;
    int nCode = pInfo->pServer->PatchJson(sUrl, pInfo->ssData.str(), sResponse);

    return DoReply(pConnection, nCode, sResponse);
}

int MicroServer::DoHttpPost(MHD_Connection* pConnection, const std::string& sUrl, ConnectionInfo* pInfo)
{
    string sResponse;
    int nCode = pInfo->pServer->PostJson(sUrl, pInfo->ssData.str(), sResponse);

    return DoReply(pConnection, nCode, sResponse);
}







bool MicroServer::Init()
{
    Log::Get() << "MicroServer: " << m_nPort << " Try Init" << std::endl;

    m_pmhd = MHD_start_daemon (MHD_USE_SELECT_INTERNALLY, m_nPort, NULL, NULL, &MicroServer::AnswerToConnection, this, MHD_OPTION_NOTIFY_COMPLETED, RequestCompleted, NULL, MHD_OPTION_END);
    if(m_pmhd)
    {
        Log::Get() << "MicroServer: " << m_nPort << " Init: OK" << std::endl;
    }
    else
    {
        Log::Get() << "MicroServer: " << m_nPort << " Init: Failed" << std::endl;
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

MicroServer::MicroServer(shared_ptr<EventPoster> pPoster, unsigned int nPort) : Server(pPoster, nPort),  m_pmhd(0)
{

}

MicroServer::~MicroServer()
{
    Stop();
}
