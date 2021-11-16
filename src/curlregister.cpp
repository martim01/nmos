#include "curlregister.h"
#include <curl/curl.h>
#include <cstring>
#include "log.h"
#include <thread>
#include "eventposter.h"

using namespace pml::nmos;

const std::string CurlRegister::STR_RESOURCE[7] = {"nodes", "devices", "sources", "flows", "senders", "receivers", "subscriptions"};

static size_t WriteCallback(void* pContents, size_t nSize, size_t nmemb, std::string* pData)
{
    size_t nRealSize = nSize*nmemb;
    pData->append(reinterpret_cast<char*>(pContents), nRealSize);
    return nRealSize;
}

static int DebugCallback(CURL* pCurl, curl_infotype type, char* data, size_t nSize, std::string* pData)
{
    pData->append(data, nSize);
    return 0;
}

static CURL* SetupCurl(const std::string& sUrl)
{
    CURL* pCurl = curl_easy_init();
    if(pCurl)
    {
        curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, CurlRegister::TIMEOUT_MSG);
        curl_easy_setopt(pCurl, CURLOPT_CONNECTTIMEOUT, CurlRegister::TIMEOUT_CONNECT);
        curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 1);
        curl_easy_setopt(pCurl, CURLOPT_HEADER, 0);
        curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(pCurl, CURLOPT_URL, sUrl.c_str());
        curl_easy_setopt(pCurl, CURLOPT_DEBUGFUNCTION, DebugCallback);
        curl_easy_setopt(pCurl, CURLOPT_FOLLOWLOCATION, 1L);
    }
    return pCurl;
}

static curlResponse DoCurl(CURL* pCurl, curl_slist *pHeaders)
{
    curlResponse resp;
    curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, &resp.sResponse);
    curl_easy_setopt(pCurl, CURLOPT_DEBUGDATA, &resp.sDebug);

    auto res = curl_easy_perform(pCurl);
    curl_easy_getinfo(pCurl, CURLINFO_RESPONSE_CODE, &resp.nCode);

    pmlLog(pml::LOG_TRACE) << resp.sDebug;
    /* Check for errors */
    if(res != CURLE_OK)
    {
        resp.sResponse = curl_easy_strerror(res);
        pmlLog(pml::LOG_ERROR) << "NMOS: " << "CURL Error: " << resp.sResponse ;
    }

    if(pHeaders)
    {
        curl_slist_free_all(pHeaders);
    }
    curl_easy_cleanup(pCurl);

    return resp;
}


static void PostThreaded(const std::string& sUrl, const std::string& sJson, CurlRegister* pRegister, long nUserType)
{
    auto resp = pRegister->Post(sUrl, sJson);
    pRegister->Callback(resp.nCode, resp.sResponse, nUserType);

}

static void DeleteThreaded(const std::string& sUrl, const std::string& sType, const std::string& sId, CurlRegister* pRegister, long nUserType)
{
    auto resp = pRegister->Delete(sUrl, sType, sId);
    pRegister->Callback(resp.nCode, resp.sResponse, nUserType);
}


static void PutThreaded(const std::string& sUrl, const std::string& sJson, CurlRegister* pRegister, long nUserType, bool bPut, const std::string& sResourceId)
{
    auto resp = pRegister->PutPatch(sUrl, sJson, bPut, sResourceId);
    pRegister->Callback(resp.nCode, resp.sResponse, nUserType);
}

static void GetThreaded(const std::string& sUrl, CurlRegister* pRegister, long nUserType)
{
    auto resp = pRegister->Get(sUrl);
    pRegister->Callback(resp.nCode, resp.sResponse, nUserType);
}

CurlRegister::CurlRegister(std::function<void(unsigned long, const std::string&, long, const std::string&)> pCallback) :
    m_pCallback(pCallback)
{

    curl_global_init(CURL_GLOBAL_DEFAULT);

}

CurlRegister::~CurlRegister()
{
    curl_global_cleanup();
}


void CurlRegister::Post(const std::string& sBaseUrl, const std::string& sJson, long nUserType)
{
    std::thread threadPost(PostThreaded, sBaseUrl, sJson, this, nUserType);
    threadPost.detach();
}



curlResponse CurlRegister::Post(const std::string& sUrl, const std::string& sJson)
{
    curlResponse resp;

    CURL* pCurl = SetupCurl(sUrl);
    if(pCurl)
    {

        pmlLog(pml::LOG_DEBUG) << "NMOS: " << "POST: " << sUrl << " payload=" << sJson ;

        curl_easy_setopt(pCurl, CURLOPT_POST, 1);

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
            curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, pHeaders);
            pHeaders = curl_slist_append(pHeaders, "Content-Type: ");
            curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, pHeaders);
            curl_easy_setopt(pCurl, CURLOPT_POSTFIELDSIZE, 0);
        }

        resp = DoCurl(pCurl, pHeaders);
    }
    return resp;

}




void CurlRegister::Delete(const std::string& sUrl, const std::string& sType, const std::string& sId, long nUserType)
{
    std::thread threadPost(DeleteThreaded, sUrl, sType, sId, this, nUserType);
    threadPost.detach();
}



curlResponse CurlRegister::Delete(const std::string& sUrl, const std::string& sType, const std::string& sId)
{

    std::stringstream ssUrl;
    ssUrl << sUrl << "/" << sType << "/" << sId;
    pmlLog(pml::LOG_DEBUG) << "NMOS: " << "DELETE: " << ssUrl.str() ;

    curlResponse resp;


    CURL* pCurl = SetupCurl(ssUrl.str());
    if(pCurl)
    {
        struct curl_slist *pHeaders = NULL;
        pHeaders = curl_slist_append(pHeaders, "Accept: application/json");
        pHeaders = curl_slist_append(pHeaders, "Content-Type: application/json");

        curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, pHeaders);
        curl_easy_setopt(pCurl, CURLOPT_CUSTOMREQUEST, "DELETE");

        resp = DoCurl(pCurl, pHeaders);

    }
    return resp;
}


void CurlRegister::Get(const std::string& sUrl, long nUserType)
{
    std::thread threadGet(GetThreaded, sUrl, this, nUserType);
    threadGet.detach();
}

curlResponse CurlRegister::Get(const std::string& sUrl, bool bJson)
{
    curlResponse resp;

    CURL* pCurl = SetupCurl(sUrl);
    if(pCurl)
    {
        struct curl_slist *pHeaders = NULL;
        if(bJson)
        {
            pHeaders = curl_slist_append(pHeaders, "Accept: application/json");
            pHeaders = curl_slist_append(pHeaders, "Content-Type: application/json");
            curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, pHeaders);
        }

        resp = DoCurl(pCurl, pHeaders);

    }
    return resp;
}


void CurlRegister::PutPatch(const std::string& sBaseUrl, const std::string& sJson, long nUserType, bool bPut, const std::string& sResourceId)
{
    std::thread threadPut(PutThreaded, sBaseUrl, sJson, this, nUserType, bPut, sResourceId);
    threadPut.detach();
}

curlResponse CurlRegister::PutPatch(const std::string& sUrl, const std::string& sJson, bool bPut, const std::string& sResourceId)
{

    curlResponse resp;
    CURL* pCurl = SetupCurl(sUrl);
    if(pCurl)
    {
        if(bPut)
        {
            curl_easy_setopt(pCurl, CURLOPT_CUSTOMREQUEST, "PUT");
        }
        else
        {
            curl_easy_setopt(pCurl, CURLOPT_CUSTOMREQUEST, "PATCH");
        }


        struct curl_slist * pHeaders = NULL;
        pHeaders = curl_slist_append(pHeaders, "Accept: application/json");
        pHeaders = curl_slist_append(pHeaders, "Content-Type: application/json");

        curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, pHeaders);
        curl_easy_setopt(pCurl, CURLOPT_POSTFIELDS, sJson.c_str());

        resp = DoCurl(pCurl, pHeaders);
    }
    return resp;
}

void CurlRegister::Callback(unsigned long nResult, const std::string& sResult, long nUser, const std::string sResponse)
{
    if(m_pCallback)
    {
        m_pCallback(nResult, sResult, nUser, sResponse);
    }
}
