#include "microserver.h"
#include "nodeapi.h"

#include <iostream>
#include "log.h"

using namespace std;

int MicroServer::IteratePost (void * ptr, MHD_ValueKind kind, const char *key, const char *filename, const char *content_type, const char *transfer_encoding, const char *data, uint64_t off, size_t size)
{
    ConnectionInfo* pInfo = reinterpret_cast<ConnectionInfo*>(ptr);

    if(size > 0)
    {
        pInfo->pServer->AddPutData(data);
    }
    return MHD_YES;
}

void MicroServer::RequestCompleted (void *cls, MHD_Connection* pConnection, void **ptr, enum MHD_RequestTerminationCode toe)
{
    Log::Get() << "Request completed" << endl;
    ConnectionInfo *pInfo = reinterpret_cast<ConnectionInfo*>(*ptr);
    if (pInfo)
    {
        if (ConnectionInfo::PUT == pInfo->nType)
        {
            MHD_destroy_post_processor (pInfo->pPost);
        }
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
    string sResponse;
    int nCode = NodeApi::Get().GetJson(sUrl, sResponse, pInfo->pServer->GetPort());

    Log::Get() << "Response: " << sResponse << endl;

    MHD_Response* pResponse = MHD_create_response_from_buffer (sResponse.length(), (void *) sResponse.c_str(), MHD_RESPMEM_MUST_COPY);
    MHD_add_response_header(pResponse, "Content-Type", "application/json");
    int ret = MHD_queue_response (pConnection, nCode, pResponse);
    MHD_destroy_response (pResponse);
    return ret;
}

int MicroServer::DoHttpPut(MHD_Connection* pConnection, string sUrl, ConnectionInfo* pInfo)
{
    string sResponse;
    int nCode = NodeApi::Get().PutJson(sUrl, pInfo->pServer->GetPutData(), sResponse, pInfo->pServer->GetPort());
    pInfo->pServer->ResetPutData();


    MHD_Response* pResponse = MHD_create_response_from_buffer (sResponse.length(), (void *) sResponse.c_str(), MHD_RESPMEM_MUST_COPY);
    MHD_add_response_header(pResponse, "Content-Type", "application/json");
    int ret = MHD_queue_response (pConnection, nCode, pResponse);
    MHD_destroy_response (pResponse);
    return ret;
}

int MicroServer::DoHttpPatch(MHD_Connection* pConnection, string sUrl, ConnectionInfo* pInfo)
{
    string sResponse;
    int nCode = NodeApi::Get().PatchJson(sUrl, pInfo->pServer->GetPutData(), sResponse, pInfo->pServer->GetPort());
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
            pInfo->pPost = MHD_create_post_processor (pConnection, 65536, IteratePost, (void *)pInfo);
            if(NULL == pInfo->pPost)
            {
                delete pInfo;
                return MHD_NO;
            }
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
        return MHD_YES;
    }

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
            MHD_post_process (pInfo->pPost, upload_data, *upload_data_size);
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
            MHD_post_process (pInfo->pPost, upload_data, *upload_data_size);
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

MicroServer::MicroServer() : m_pmhd(0)
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
    std::unique_lock<std::mutex> lk(m_mutex);
    m_cvSync.wait(lk);
}

void MicroServer::Signal(unsigned char nCode)
{
    m_nCode = nCode;
    m_cvSync.notify_one();
}

unsigned char MicroServer::GetResponseCode() const
{
    lock_guard<mutex> lock(m_mutex);
    return m_nCode;
}
