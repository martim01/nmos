#pragma once
#include <string>
#include <condition_variable>
#include "microhttpd.h"
#include <thread>
#include <vector>
#include "json/json.h"
#include "version.h"
#include "microserver.h"


class IS04Server : public MicroServer
{
    public:

        /** @brief Constructor - this is called in NodeApi::StartService
        *   @param pPoster a sheared_ptr to an object of a class derived from EventPoster.
        **/
        IS04Server(std::shared_ptr<EventPoster> pPoster);



    protected:
        friend class NodeApi;


    private:



        int GetJsonNmos(std::string& sReturn, std::string& sContentType);
        int PutJsonNmos(const std::string& sJson, std::string& sResponse);

        int GetJsonNmosNodeApi(std::string& sReturn);
        Json::Value GetJsonSources(const ApiVersion& version);
        Json::Value GetJsonDevices(const ApiVersion& version);
        Json::Value GetJsonFlows(const ApiVersion& version);
        Json::Value GetJsonReceivers(const ApiVersion& version);
        Json::Value GetJsonSenders(const ApiVersion& version);

        enum {NMOS=0, API_TYPE=1,VERSION=2,ENDPOINT=3, RESOURCE=4, TARGET=5};
        enum {SZ_BASE=0, SZ_NMOS=1, SZ_API_TYPE=2,SZ_VERSION=3,SZ_ENDPOINT=4, SZ_RESOURCE=5, SZ_TARGET=6};

};

