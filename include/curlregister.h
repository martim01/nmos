#pragma once
#include <curl/curl.h>
#include <string>
#include "nodeapi.h"

class CurlEvent;
/** @brief Struct that keeps the return Curl data in memory until we've finished getting stuff
**/
struct MemoryStruct
{
    ///< @brief Constructor
    MemoryStruct()
    {
        pMemory = reinterpret_cast<char*>(malloc(1));
        nSize = 0;
    }

    ///< @brief Destructor
    ~MemoryStruct()
    {
        free(pMemory);
    }

  char *pMemory;    ///< @brief Pointer to some memory
  size_t nSize;     ///< @brief Size of the allocated memory
};


/** @brief Curl callback function that gets the returned Schedule data from PIPs
*   @param pData
*   @param nSize
*   @param nMemb
*   @param pNetwork
*   @return <i>int<i>
**/
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp);

/** @brief Curl callback function that gets any error/debug messages from libcurl
*   @param handle
*   @param type
*   @param data
*   @param size
*   @param userptr
*   @return <i>int<i>
**/
static int debug_callback(CURL *handle, curl_infotype type, char *data, size_t size, void *userptr);

class EventPoster;
/** @brief class that does all the curl requests to PIPs and decodes the returned data. This class is contained in curlthread
**/
class CurlRegister
{

    public:
        ///< @brief constructor
        CurlRegister(EventPoster* pPoster);

        ///< @brief Destructor
        ~CurlRegister();

        EventPoster* GetPoster();
        //threaded version
        void Post(const std::string& sBaseUrl, const std::string& sJson, long nUserType);
        //simple versions
        long Post(const std::string& sBaseUrl, const std::string& sJson, std::string& sResponse);

        void Delete(const std::string& sBaseUrl, const std::string& sType, const std::string& sId, long nUserType);
        long Delete(const std::string& sBaseUrl, const std::string& sType, const std::string& sId, std::string& sResponse);

        void Query(const std::string& sBaseUrl, NodeApi::enumResource eResource, const std::string& sQuery, ResourceHolder* pResults, long nUserType);
        long Query(const std::string& sBaseUrl, NodeApi::enumResource eResource, const std::string& sQuery, ResourceHolder* pResults);

    private:

        //void PostStatic(const std::string& sUrl, const std::string& sJson, EventPoster* pPoster);
        void ParseResults(NodeApi::enumResource eResource, const std::string& sResponse, ResourceHolder* pResults);
        EventPoster* m_pPoster;

        static const std::string STR_RESOURCE[7];

};
