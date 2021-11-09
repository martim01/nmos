#pragma once
#include <curl/curl.h>
#include <string>
#include <memory>

class CurlEvent;
class EventPoster;

struct curlResponse
{
    curlResponse() : nCode(500){}
    long nCode;
    std::string sResponse;
};


/** @brief class that does all the curl requests to PIPs and decodes the returned data. This class is contained in curlthread
**/
class CurlRegister
{

    public:
        ///< @brief constructor
        CurlRegister(std::shared_ptr<EventPoster> pPoster);

        ///< @brief Destructor
        ~CurlRegister();

        std::shared_ptr<EventPoster> GetPoster();

        //simple versions
        static curlResponse Post(const std::string& sBaseUrl, const std::string& sJson);
        static curlResponse Delete(const std::string& sBaseUrl, const std::string& sType, const std::string& sId);
        static curlResponse PutPatch(const std::string& sBaseUrl, const std::string& sJson, bool bPut, const std::string& sResourceId);
        static curlResponse Get(const std::string& sUrl, bool bJson=false);


        //threaded version
        void Post(const std::string& sBaseUrl, const std::string& sJson, long nUserType);
        void Delete(const std::string& sBaseUrl, const std::string& sType, const std::string& sId, long nUserType);
        void PutPatch(const std::string& sBaseUrl, const std::string& sJson, long nUserType, bool bPut, const std::string& sResourceId);
        void Get(const std::string& sUrl, long nUserType);

        static const int TIMEOUT_CONNECT = 3;
        static const int TIMEOUT_MSG = 2;

    private:


        std::shared_ptr<EventPoster> m_pPoster;

        static const std::string STR_RESOURCE[7];



};

