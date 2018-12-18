#include "curlregister.h"
#include <curl/curl.h>
#include <cstring>
#include "log.h"
#include <thread>
#include "curlevent.h"

static void PostStatic(const std::string& sUrl, const std::string& sJson, CurlEvent* pPoster)
{
    char sError[CURL_ERROR_SIZE];
    CURLcode res;
    long nResponseCode(500);
    std::string sResponse;

    CURL* pCurl = curl_easy_init();
    if(pCurl)
    {

        curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 1);
        curl_easy_setopt(pCurl, CURLOPT_HEADER, 0);
        curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(pCurl, CURLOPT_DEBUGFUNCTION, debug_callback);
        curl_easy_setopt(pCurl, CURLOPT_ERRORBUFFER, sError);


        MemoryStruct chunk;

        /* what call to write: */
        curl_easy_setopt(pCurl, CURLOPT_URL, sUrl.c_str());
        curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, &chunk);

        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Accept: application/json");
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, headers);
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
        curl_easy_cleanup(pCurl);
    }
    pPoster->CurlDone(nResponseCode, sResponse);
}


CurlRegister::CurlRegister(CurlEvent* pPoster) :
    m_pPoster(pPoster)
{

    curl_global_init(CURL_GLOBAL_DEFAULT);

}

CurlRegister::~CurlRegister()
{
    if(m_pPoster)
    {
        delete m_pPoster;
    }
    curl_global_cleanup();
}


void CurlRegister::Post(const std::string& sUrl, const std::string& sJson)
{
    Log::Get(Log::INFO) << "CurlRegster: Post '" << sUrl << "'" << std::endl;
    Log::Get(Log::INFO) << "CurlRegster: Json '" << sJson << "'" << std::endl;
    std::thread threadPost(PostStatic, sUrl, sJson, m_pPoster);
    threadPost.detach();
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





