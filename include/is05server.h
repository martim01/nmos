#pragma once
#include <string>
#include <condition_variable>
#include "microhttpd.h"
#include <thread>
#include <vector>
#include "json/json.h"
#include "version.h"
#include "microserver.h"


class IS05Server : public MicroServer
{
    public:

        /** @brief Constructor - this is called in NodeApi::StartService
        *   @param pPoster a sheared_ptr to an object of a class derived from EventPoster.
        **/
        IS05Server(std::shared_ptr<EventPoster> pPoster);



    protected:
        friend class NodeApi;


    private:



        int GetJsonNmos(std::string& sReturn, std::string& sContentType);

        int PatchJsonNmos(const std::string& sJson, std::string& sResponse);
        int GetJsonNmosConnectionApi(std::string& sReturn, std::string& sContentType);
        int GetJsonNmosConnectionSingleApi(std::string& sReturn, std::string& sContentType, const ApiVersion& version);
        int GetJsonNmosConnectionBulkApi(std::string& sReturn);

        int GetJsonNmosConnectionSingleSenders(std::string& sReturn, std::string& sContentType, const ApiVersion& version);
        int GetJsonNmosConnectionSingleReceivers(std::string& sReturn, const ApiVersion& version);

        int PatchJsonSender(const std::string& sJson, std::string& sResponse, const ApiVersion& version);
        int PatchJsonReceiver(const std::string& sJson, std::string& sResponse, const ApiVersion& version);


        enum {NMOS=0, API_TYPE=1,VERSION=2,ENDPOINT=3, RESOURCE=4, TARGET=5};
        enum {SZ_BASE=0, SZ_NMOS=1, SZ_API_TYPE=2,SZ_VERSION=3,SZ_ENDPOINT=4, SZ_RESOURCE=5, SZ_TARGET=6};
        enum {SZC_TYPE=4, SZC_DIRECTION=5, SZC_ID=6, SZC_LAST=7};
        enum {C_TYPE = 3, C_DIRECTION=4, C_ID=5, C_LAST=6};
};


