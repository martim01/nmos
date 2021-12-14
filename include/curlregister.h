#pragma once
#include <curl/curl.h>
#include <string>
#include <memory>
#include <list>
#include <functional>

namespace pml
{
    namespace nmos
    {
        class CurlEvent;
        class EventPoster;

        struct curlResponse
        {
            curlResponse() : nCode(500){}
            long nCode;
            std::string sResponse;
            std::string sDebug;
        };


        /** @brief class that does all the curl requests to PIPs and decodes the returned data. This class is contained in curlthread
        **/
        class CurlRegister
        {

            public:
                ///< @brief constructor
                CurlRegister(std::function<void(const curlResponse&, unsigned long, const std::string&)> pCallback);

                ///< @brief Destructor
                ~CurlRegister();


                //simple versions
                static curlResponse Post(const std::string& sBaseUrl, const std::string& sJson);
                static curlResponse Delete(const std::string& sBaseUrl, const std::string& sType, const std::string& sId);
                static curlResponse PutPatch(const std::string& sBaseUrl, const std::string& sJson, bool bPut, const std::string& sResourceId);
                static curlResponse Get(const std::string& sUrl, bool bJson=false);


                //threaded version
                void PostAsync(const std::string& sBaseUrl, const std::string& sJson, long nUserType);
                void DeleteAsync(const std::string& sBaseUrl, const std::string& sType, const std::string& sId, long nUserType);
                void PutPatchAsync(const std::string& sBaseUrl, const std::string& sJson, long nUserType, bool bPut, const std::string& sResourceId);
                void GetAsync(const std::string& sUrl, long nUserType, bool bJson=false);

                void Callback(const curlResponse& resp, long nUser, const std::string sResponse="");

                static const int TIMEOUT_CONNECT = 3;
                static const int TIMEOUT_MSG = 2;

            private:


                std::function<void(const curlResponse&, unsigned long, const std::string&)> m_pCallback;

                static const std::string STR_RESOURCE[7];

                //std::list<std::shared_ptr<std::thread>> m_lstThread;


        };
    };
};
