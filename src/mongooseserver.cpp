#include "mongooseserver.h"
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

static mg_serve_http_opts s_ServerOpts;

static int is_websocket(const struct mg_connection *nc)
{
    return nc->flags & MG_F_IS_WEBSOCKET;
}

static void ev_handler(mg_connection *pConnection, int nEvent, void* pData)
{
    if(nEvent == 0)
    {
        return;
    }

    MongooseServer* pThread = reinterpret_cast<MongooseServer*>(pConnection->user_data);
    pThread->HandleEvent(pConnection, nEvent, pData);
}

static mg_str file_upload(mg_connection*, mg_str file_name)
{
    Log::Get(Log::LOG_DEBUG) << "upload" << endl;
    char* filename= reinterpret_cast<char*>(malloc(strlen("/tmp/ns_temp.src")+1));
    strcpy(filename, "/tmp/ns_temp.src");
    return mg_mk_str(filename);
}

void MongooseServer::HandleEvent(mg_connection *pConnection, int nEvent, void* pData)
{

    switch (nEvent)
    {
    case MG_EV_WEBSOCKET_HANDSHAKE_DONE:
    {
        Log::Get(Log::LOG_DEBUG) << "Event Websocket" << endl;
    //    EventWebsocket(pConnection, nEvent, pData);
        Log::Get(Log::LOG_DEBUG) << "Done" << endl;
    }
    break;
    case MG_EV_HTTP_REQUEST:
    {
        Log::Get(Log::LOG_DEBUG) << "Event HTTP" << endl;
        EventHttp(pConnection, nEvent, pData);
        Log::Get(Log::LOG_DEBUG) << "Done" << endl;
    }
    break;
    case MG_EV_TIMER:
        /* Perform session maintenance. */
    {
        //mg_set_timer(pConnection, mg_time() + SESSION_CHECK_INTERVAL);
    }
    break;
    case MG_EV_CLOSE:
    {
        Log::Get(Log::LOG_DEBUG) << "Event Close" << endl;
        if (is_websocket(pConnection))
        {
            pConnection->user_data = 0;
        }
        Log::Get(Log::LOG_DEBUG) << "Done" << endl;
    }
    break;
    case MG_EV_HTTP_MULTIPART_REQUEST:
        StartUpload(reinterpret_cast<http_message*>(pData));
        break;
    case MG_EV_HTTP_PART_BEGIN:
        Log::Get(Log::LOG_DEBUG) << "MG_EV_HTTP_PART_BEGIN" << endl;
        mg_file_upload_handler(pConnection, nEvent, pData, file_upload);
        break;
    case MG_EV_HTTP_PART_DATA:
        Log::Get(Log::LOG_DEBUG) << "MG_EV_HTTP_PART_DATA" << endl;
        mg_file_upload_handler(pConnection, nEvent, pData, file_upload);
        break;
    case MG_EV_HTTP_PART_END:
        Log::Get(Log::LOG_DEBUG) << "MG_EV_HTTP_PART_END" << endl;
        mg_file_upload_handler(pConnection, nEvent, pData, file_upload);
        EndUpload(pConnection);
    case 0:
        break;
    }
}

void MongooseServer::EventHttp(mg_connection *pConnection, int nEvent, void* pData)
{

    //lock_guard<mutex> lg(m_mutex);
    http_message* pMessage = reinterpret_cast<http_message*>(pData);
    string sMethod(pMessage->method.p);
    string sUri;
    sUri.assign(pMessage->uri.p, pMessage->uri.len);

    cout << "METHOD=" << sMethod << std::endl;
    if(mg_vcmp(&pMessage->method, "GET") == 0)
    {
        Log::Get(Log::LOG_DEBUG) << "Actual connection: GET" << endl;
        DoHttpGet(pConnection, sUri);
    }
    else if(mg_vcmp(&pMessage->method, "PUT") == 0)
    {
        Log::Get(Log::LOG_DEBUG) << "Actual connection: PUT" << endl;
         DoHttpPut(pConnection, sUri, pMessage);
    }
    else if(mg_vcmp(&pMessage->method, "PATCH") == 0)
    {
        Log::Get(Log::LOG_DEBUG) << "Actual connection: PATCH" << endl;
        DoHttpPatch(pConnection, sUri, pMessage);
    }
    else if(mg_vcmp(&pMessage->method, "POST") == 0)
    {
        Log::Get(Log::LOG_DEBUG) << "Actual connection: POST" << endl;
        DoHttpPost(pConnection, sUri, pMessage);
    }

}


int MongooseServer::DoReply(mg_connection* pConnection, int nCode, const std::string& sResponse, const std::string& sContentType)
{
    Log::Get() << "MongooseServer::DoReply "<< sResponse << endl;
    /* Send headers */
    stringstream ssHeaders;
    ssHeaders << "HTTP/1.1 " << nCode << "\r\n"
              << "Transfer-Encoding: chunked\r\n"
              << "Content-Type: " << sContentType << "\r\n"
              << "Access-Control-Allow-Origin:*\r\n"
              << "Access-Control-Allow-Methods:GET, PUT, POST, HEAD, OPTIONS, DELETE\r\n"
              << "Access-Control-Allow-Headers:Content-Type, Accept\r\n"
              << "Access-Control-Max-Age:3600\r\n\r\n";

    mg_printf(pConnection, "%s", ssHeaders.str().c_str());

    mg_printf_http_chunk(pConnection, sResponse.c_str());
    mg_send_http_chunk(pConnection, "", 0); /* Send empty chunk, the end of response */

    return nCode;
}


int MongooseServer::DoHttpGet(mg_connection* pConnection, const std::string& sUrl)
{
    string sResponse, sContentType;
    int nCode = GetJson(sUrl, sResponse, sContentType);

    return DoReply(pConnection, nCode, sResponse, sContentType);
}

int MongooseServer::DoHttpPut(mg_connection* pConnection, const std::string& sUrl, http_message* pMessage)
{
    string sResponse;
    int nCode = PutJson(sUrl, string(pMessage->body.p), sResponse);

    return DoReply(pConnection, nCode, sResponse);
}

int MongooseServer::DoHttpPatch(mg_connection* pConnection, const std::string& sUrl, http_message* pMessage)
{
    string sResponse;
    int nCode = PatchJson(sUrl, pMessage->body.p, sResponse);

    return DoReply(pConnection, nCode, sResponse);
}

int MongooseServer::DoHttpPost(mg_connection* pConnection, const std::string& sUrl, http_message* pMessage)
{
    string sResponse;
    int nCode = PostJson(sUrl, pMessage->body.p, sResponse);

    return DoReply(pConnection, nCode, sResponse);
}



bool MongooseServer::Init()
{

    Log::Get() << "MongooseServer: " << m_nPort << " Try Init" << std::endl;

    s_ServerOpts.document_root = "/";
    s_ServerOpts.enable_directory_listing = "no";
    //s_ServerOpts.url_rewrites=ssRewrite.str().c_str();
    s_ServerOpts.extra_headers="X-Frame-Options: sameorigin\r\nCache-Control: no-cache\r\nStrict-Transport-Security: max-age=31536000; includeSubDomains\r\nX-Content-Type-Options: nosniff\r\nReferrer-Policy: no-referrer\r\nServer: unknown";
    mg_mgr_init(&m_mgr, NULL);

    stringstream ss;
    ss << m_nPort;

    m_pConnection = mg_bind(&m_mgr, ss.str().c_str(), ev_handler);


    if(m_pConnection)
    {
        m_pConnection->user_data = reinterpret_cast<void*>(this);

        thread th(&MongooseServer::Loop, this);
        th.detach();

        Log::Get() << m_pConnection->user_data << endl;
        Log::Get() << s_ServerOpts.document_root << endl;
        Log::Get() << "--------------------------" << endl;
        return true;
    }
    else
    {
        Log::Get() << "Could not start webserver" << endl;
        return false;
    }

}

void MongooseServer::Loop()
{
    if(m_pConnection)
    {
        m_bRun = true;
        mg_set_protocol_http_websocket(m_pConnection);
        while (m_bRun)
        {
            mg_mgr_poll(&m_mgr, 50);
            //SendQueue();
        }
        mg_mgr_free(&m_mgr);
    }
}

void MongooseServer::Stop()
{
    lock_guard<mutex> lg(m_mutex);
    if(m_pConnection)
    {
        m_bRun = false;
    }
}

MongooseServer::MongooseServer(shared_ptr<EventPoster> pPoster, unsigned int nPort) : Server(pPoster, nPort) ,m_pConnection(0)
{

}

MongooseServer::~MongooseServer()
{
    Stop();

}


void MongooseServer::StartUpload(http_message* pMessage)
{

    Log::Get(Log::LOG_INFO) << "Starting upload" << endl;

}

void MongooseServer::EndUpload(mg_connection *pConnection)
{


}

