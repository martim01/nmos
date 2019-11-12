#include "curlregister.h"
#include <curl/curl.h>
#include <cstring>
#include "log.h"
#include <thread>
#include "eventposter.h"

const std::string CurlRegister::STR_RESOURCE[7] = {"nodes", "devices", "sources", "flows", "senders", "receivers", "subscriptions"};


static void PostThreaded(const std::string& sUrl, const std::string& sJson, CurlRegister* pRegister, long nUserType)
{
    std::string sResponse;
    long nResponseCode = pRegister->Post(sUrl, sJson, sResponse);
    if(pRegister->GetPoster())
    {
        pRegister->GetPoster()->_CurlDone(nResponseCode, sResponse, nUserType);
    }
}

static void DeleteThreaded(const std::string& sUrl, const std::string& sType, const std::string& sId, CurlRegister* pRegister, long nUserType)
{
    std::string sResponse;
    long nResponseCode = pRegister->Delete(sUrl, sType, sId, sResponse);
    if(pRegister->GetPoster())
    {
        pRegister->GetPoster()->_CurlDone(nResponseCode, sResponse, nUserType);
    }
}

//static void QueryThreaded(const std::string& sBaseUrl, NodeApi::enumResource eResource, const std::string& sQuery, ResourceHolder* pResults, CurlRegister* pRegister, long nUserType)
//{
//    long nResponseCode = pRegister->Query(sBaseUrl, eResource, sQuery, pResults);
//    if(pRegister->GetPoster())
//    {
//        pRegister->GetPoster()->_CurlDone(nResponseCode, "", nUserType);
//    }
//}

static void PutThreaded(const std::string& sUrl, const std::string& sJson, CurlRegister* pRegister, long nUserType, bool bPut, const std::string& sResourceId)
{
    std::string sResponse;
    long nResponseCode = pRegister->PutPatch(sUrl, sJson, sResponse, bPut, sResourceId);
    if(pRegister->GetPoster())
    {
        pRegister->GetPoster()->_CurlDone(nResponseCode, sResponse, nUserType, sResourceId);
    }
}

static void GetThreaded(const std::string& sUrl, CurlRegister* pRegister, long nUserType)
{
    std::string sResponse;
    long nResponseCode = pRegister->Get(sUrl, sResponse);
    if(pRegister->GetPoster())
    {
        pRegister->GetPoster()->_CurlDone(nResponseCode, sResponse, nUserType);
    }
}

CurlRegister::CurlRegister(std::shared_ptr<EventPoster> pPoster) :
    m_pPoster(pPoster)
{

    curl_global_init(CURL_GLOBAL_DEFAULT);

}

CurlRegister::~CurlRegister()
{
    curl_global_cleanup();
}

std::shared_ptr<EventPoster> CurlRegister::GetPoster()
{
    return m_pPoster;
}

void CurlRegister::Post(const std::string& sBaseUrl, const std::string& sJson, long nUserType)
{
    std::thread threadPost(PostThreaded, sBaseUrl, sJson, this, nUserType);
    threadPost.detach();
}

long CurlRegister::Post(const std::string& sUrl, const std::string& sJson, std::string& sResponse)
{
    std::cout << std::endl;
    std::cout << sUrl << std::endl;
    std::cout << std::endl;
    char sError[CURL_ERROR_SIZE];
    CURLcode res;
    long nResponseCode(500);

    CURL* pCurl = curl_easy_init();
    if(pCurl)
    {

        curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, 5);
        curl_easy_setopt(pCurl, CURLOPT_CONNECTTIMEOUT, 5);
        curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 1);
        curl_easy_setopt(pCurl, CURLOPT_HEADER, 0);
        curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(pCurl, CURLOPT_DEBUGFUNCTION, debug_callback);
        curl_easy_setopt(pCurl, CURLOPT_ERRORBUFFER, sError);
        curl_easy_setopt(pCurl, CURLOPT_POST, 1);

        MemoryStruct chunk;

        /* what call to write: */
        curl_easy_setopt(pCurl, CURLOPT_URL, sUrl.c_str());
        curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, &chunk);


        struct curl_slist * pHeaders = NULL;
        if(sJson.length() > 0)
        {
            pHeaders = curl_slist_append(pHeaders, "Accept: application/json");
            pHeaders = curl_slist_append(pHeaders, "Content-Type: application/json");
            curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, pHeaders);
            curl_easy_setopt(pCurl, CURLOPT_POSTFIELDS, sJson.c_str());
        }
        else
        {
            Log::Get(Log::LOG_DEBUG) << "No payload " << std::endl;
            curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, pHeaders);
            pHeaders = curl_slist_append(pHeaders, "Content-Type: ");
            curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, pHeaders);
            curl_easy_setopt(pCurl, CURLOPT_POSTFIELDSIZE, 0);
        }

        res = curl_easy_perform(pCurl);
        /* Check for errors */
        if(res != CURLE_OK)
        {
            curl_easy_getinfo(pCurl, CURLINFO_RESPONSE_CODE, &nResponseCode);
            sResponse = curl_easy_strerror(res);
        }
        else
        {
            sResponse.assign(chunk.pMemory, chunk.nSize);
            curl_easy_getinfo(pCurl, CURLINFO_RESPONSE_CODE, &nResponseCode);
        }
        curl_slist_free_all(pHeaders);
        curl_easy_cleanup(pCurl);
    }
    return nResponseCode;
}




void CurlRegister::Delete(const std::string& sUrl, const std::string& sType, const std::string& sId, long nUserType)
{
    std::thread threadPost(DeleteThreaded, sUrl, sType, sId, this, nUserType);
    threadPost.detach();
}



long CurlRegister::Delete(const std::string& sUrl, const std::string& sType, const std::string& sId, std::string& sResponse)
{
    std::stringstream ssUrl;
    ssUrl << sUrl << "/" << sType << "/" << sId;


    char sError[CURL_ERROR_SIZE];
    CURLcode res;
    long nResponseCode(500);


    CURL* pCurl = curl_easy_init();
    if(pCurl)
    {

        curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, 5);
        curl_easy_setopt(pCurl, CURLOPT_CONNECTTIMEOUT, 5);
        curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 1);
        curl_easy_setopt(pCurl, CURLOPT_HEADER, 0);
        curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(pCurl, CURLOPT_DEBUGFUNCTION, debug_callback);
        curl_easy_setopt(pCurl, CURLOPT_ERRORBUFFER, sError);


        MemoryStruct chunk;

        /* what call to write: */
        curl_easy_setopt(pCurl, CURLOPT_URL, ssUrl.str().c_str());
        curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, &chunk);

        struct curl_slist *pHeaders = NULL;
        pHeaders = curl_slist_append(pHeaders, "Accept: application/json");
        pHeaders = curl_slist_append(pHeaders, "Content-Type: application/json");

        curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, pHeaders);
        curl_easy_setopt(pCurl, CURLOPT_CUSTOMREQUEST, "DELETE");

        res = curl_easy_perform(pCurl);
        /* Check for errors */
        if(res != CURLE_OK)
        {
            curl_easy_getinfo(pCurl, CURLINFO_RESPONSE_CODE, &nResponseCode);
            sResponse = curl_easy_strerror(res);
        }
        else
        {
            sResponse.assign(chunk.pMemory, chunk.nSize);
            curl_easy_getinfo(pCurl, CURLINFO_RESPONSE_CODE, &nResponseCode);
        }
        curl_slist_free_all(pHeaders);
        curl_easy_cleanup(pCurl);
    }
    return nResponseCode;
}


//void CurlRegister::Query(const std::string& sBaseUrl, NodeApi::enumResource eResource, const std::string& sQuery, ResourceHolder* pResults, long nUserType)
//{
//
//    std::thread threadPost(QueryThreaded, sBaseUrl, eResource, sQuery, pResults, this, nUserType);
//    threadPost.detach();
//}
//
//long CurlRegister::Query(const std::string& sBaseUrl, NodeApi::enumResource eResource, const std::string& sQuery, ResourceHolder* pResults)
//{
//    char sError[CURL_ERROR_SIZE];
//    CURLcode res;
//    long nResponseCode(500);
//
//
//    std::stringstream ssUrl;
//    ssUrl << sBaseUrl << "/" << STR_RESOURCE[eResource] << "/" << sQuery;
//
//
//    std::string sResponse;
//
//    CURL* pCurl = curl_easy_init();
//    if(pCurl)
//    {
//
//        curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 1);
//        curl_easy_setopt(pCurl, CURLOPT_HEADER, 0);
//        curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
//        curl_easy_setopt(pCurl, CURLOPT_DEBUGFUNCTION, debug_callback);
//        curl_easy_setopt(pCurl, CURLOPT_ERRORBUFFER, sError);
//
//
//        MemoryStruct chunk;
//
//        /* what call to write: */
//        curl_easy_setopt(pCurl, CURLOPT_URL, ssUrl.str().c_str());
//        curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, &chunk);
//
//        struct curl_slist *pHeaders = NULL;
//        pHeaders = curl_slist_append(pHeaders, "Accept: application/json");
//        pHeaders = curl_slist_append(pHeaders, "Content-Type: application/json");
//
//        curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, pHeaders);
//
//        res = curl_easy_perform(pCurl);
//        /* Check for errors */
//        if(res != CURLE_OK)
//        {
//            curl_easy_getinfo(pCurl, CURLINFO_RESPONSE_CODE, &nResponseCode);
//            sResponse = curl_easy_strerror(res);
//        }
//        else
//        {
//            sResponse.assign(chunk.pMemory, chunk.nSize);
//
//            ParseResults(eResource, sResponse, pResults);
//            curl_easy_getinfo(pCurl, CURLINFO_RESPONSE_CODE, &nResponseCode);
//        }
//        curl_slist_free_all(pHeaders);
//        curl_easy_cleanup(pCurl);
//    }
//    return nResponseCode;
//}


void CurlRegister::Get(const std::string& sUrl, long nUserType)
{
    std::thread threadGet(GetThreaded, sUrl, this, nUserType);
    threadGet.detach();
}

long CurlRegister::Get(const std::string& sUrl, std::string& sResponse, bool bJson)
{
    char sError[CURL_ERROR_SIZE];
    CURLcode res;
    long nResponseCode(500);



    CURL* pCurl = curl_easy_init();
    if(pCurl)
    {

        curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, 5);
        curl_easy_setopt(pCurl, CURLOPT_CONNECTTIMEOUT, 5);
        curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 1);
        curl_easy_setopt(pCurl, CURLOPT_HEADER, 0);
        curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(pCurl, CURLOPT_DEBUGFUNCTION, debug_callback);
        curl_easy_setopt(pCurl, CURLOPT_ERRORBUFFER, sError);


        MemoryStruct chunk;

        /* what call to write: */
        curl_easy_setopt(pCurl, CURLOPT_URL, sUrl.c_str());
        curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, &chunk);

        struct curl_slist *pHeaders = NULL;
        if(bJson)
        {
            pHeaders = curl_slist_append(pHeaders, "Accept: application/json");
            pHeaders = curl_slist_append(pHeaders, "Content-Type: application/json");
            curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, pHeaders);
        }

        res = curl_easy_perform(pCurl);
        /* Check for errors */
        if(res != CURLE_OK)
        {
            curl_easy_getinfo(pCurl, CURLINFO_RESPONSE_CODE, &nResponseCode);
            sResponse = curl_easy_strerror(res);
            Log::Get(Log::LOG_ERROR) << "CURL Error: " << sResponse << std::endl;
        }
        else
        {
            sResponse.assign(chunk.pMemory, chunk.nSize);
            curl_easy_getinfo(pCurl, CURLINFO_RESPONSE_CODE, &nResponseCode);
        }
        if(pHeaders)
        {
            curl_slist_free_all(pHeaders);
        }
        curl_easy_cleanup(pCurl);

    }
    return nResponseCode;
}

//void CurlRegister::ParseResults(NodeApi::enumResource eResource, const std::string& sResponse, ResourceHolder* pResults)
//{
//    Json::Reader jsReader;
//    Json::Value jsResult;
//    if(jsReader.parse(sResponse, jsResult))
//    {
//
//        for(Json::ArrayIndex n = 0; n < jsResult.size(); n++)
//        {
//            //@todo create the correct resources here in the same way as the registryapi does
//
//        }
//        //pResults->Commit();
//    }
//    else
//    {
//        Log::Get(Log::LOG_ERROR) << "Query: Could not parse response" << std::endl;
//    }
//}
//


void CurlRegister::PutPatch(const std::string& sBaseUrl, const std::string& sJson, long nUserType, bool bPut, const std::string& sResourceId)
{
    std::thread threadPut(PutThreaded, sBaseUrl, sJson, this, nUserType, bPut, sResourceId);
    threadPut.detach();
}

long CurlRegister::PutPatch(const std::string& sUrl, const std::string& sJson, std::string& sResponse, bool bPut, const std::string& sResourceId)
{

    char sError[CURL_ERROR_SIZE];
    CURLcode res;
    long nResponseCode(500);

    CURL* pCurl = curl_easy_init();
    if(pCurl)
    {
        curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, 5);
        curl_easy_setopt(pCurl, CURLOPT_CONNECTTIMEOUT, 5);
        curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 1);
        curl_easy_setopt(pCurl, CURLOPT_HEADER, 0);
        curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(pCurl, CURLOPT_DEBUGFUNCTION, debug_callback);
        curl_easy_setopt(pCurl, CURLOPT_ERRORBUFFER, sError);
        if(bPut)
        {
            curl_easy_setopt(pCurl, CURLOPT_CUSTOMREQUEST, "PUT");
        }
        else
        {
            curl_easy_setopt(pCurl, CURLOPT_CUSTOMREQUEST, "PATCH");
        }

        MemoryStruct chunk;

        /* what call to write: */
        curl_easy_setopt(pCurl, CURLOPT_URL, sUrl.c_str());
        curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, &chunk);

        struct curl_slist * pHeaders = NULL;
        pHeaders = curl_slist_append(pHeaders, "Accept: application/json");
        pHeaders = curl_slist_append(pHeaders, "Content-Type: application/json");

        curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, pHeaders);
        curl_easy_setopt(pCurl, CURLOPT_POSTFIELDS, sJson.c_str());

        res = curl_easy_perform(pCurl);
        /* Check for errors */
        if(res != CURLE_OK)
        {
            curl_easy_getinfo(pCurl, CURLINFO_RESPONSE_CODE, &nResponseCode);
            sResponse = curl_easy_strerror(res);
        }
        else
        {
            sResponse.assign(chunk.pMemory, chunk.nSize);
            curl_easy_getinfo(pCurl, CURLINFO_RESPONSE_CODE, &nResponseCode);
        }
        curl_slist_free_all(pHeaders);
        curl_easy_cleanup(pCurl);
    }
    return nResponseCode;
}



size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    MemoryStruct *pChunk = reinterpret_cast<MemoryStruct*>(userp);

    pChunk->pMemory = reinterpret_cast<char*>(realloc(pChunk->pMemory, pChunk->nSize + realsize + 1));
    if(pChunk->pMemory == NULL)
    {
        return 0;
    }

    memcpy(&(pChunk->pMemory[pChunk->nSize]), contents, realsize);
    pChunk->nSize += realsize;
    pChunk->pMemory[pChunk->nSize] = 0;

    return realsize;
}







int debug_callback(CURL *handle, curl_infotype type, char *data, size_t size, void *userptr)
{
//    wmLog::Get()->Log(wxT("Curl"), wxString::FromAscii(data));
    return size;
}





