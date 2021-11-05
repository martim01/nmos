#pragma once
#include <string>
#include <condition_variable>
#include "microhttpd.h"
#include <thread>
#include <vector>
#include "json/json.h"
#include "version.h"
#include "nmosserver.h"

class IS04Server : public NmosServer
{
    public:

        /** @brief Constructor - this is called in NodeApi::StartService
        *   @param pPoster a sheared_ptr to an object of a class derived from EventPoster.
        **/
        IS04Server(std::shared_ptr<RestGoose> pServer, const ApiVersion& version, std::shared_ptr<EventPoster> pPoster);
        virtual ~IS04Server();


        void AddSourceEndpoint(const std::string& sId);
        void AddFlowEndpoint(const std::string& sId);
        void AddDeviceEndpoint(const std::string& sId);
        void AddSenderEndpoint(const std::string& sId);
        void AddReceiverEndpoint(const std::string& sId);

        void RemoveSenderEndpoint(const std::string& sId);
        void RemoveReceiverEndpoint(const std::string& sId);


        response GetNmosRoot(const query& theQuery, const postData& theData, const url& theUrl, const userName& theUser);
        response GetNmosVersion(const query& theQuery, const postData& theData, const url& theUrl, const userName& theUser);
        response GetNmosNode(const query& theQuery, const postData& theData, const url& theUrl, const userName& theUser);

        response GetNmosSources(const query& theQuery, const postData& theData, const url& theUrl, const userName& theUser);
        response GetNmosFlows(const query& theQuery, const postData& theData, const url& theUrl, const userName& theUser);
        response GetNmosDevices(const query& theQuery, const postData& theData, const url& theUrl, const userName& theUser);
        response GetNmosSenders(const query& theQuery, const postData& theData, const url& theUrl, const userName& theUser);
        response GetNmosReceivers(const query& theQuery, const postData& theData, const url& theUrl, const userName& theUser);


        response GetNmosSource(const query& theQuery, const postData& theData, const url& theUrl, const userName& theUser);
        response GetNmosFlow(const query& theQuery, const postData& theData, const url& theUrl, const userName& theUser);
        response GetNmosDevice(const query& theQuery, const postData& theData, const url& theUrl, const userName& theUser);
        response GetNmosSender(const query& theQuery, const postData& theData, const url& theUrl, const userName& theUser);
        response GetNmosReceiver(const query& theQuery, const postData& theData, const url& theUrl, const userName& theUser);
        response PutNmosReceiver(const query& theQuery, const postData& theData, const url& theUrl, const userName& theUser);

    private:
        void AddBaseEndpoints();

        Json::Value GetJsonSources(const ApiVersion& version);
        Json::Value GetJsonDevices(const ApiVersion& version);
        Json::Value GetJsonFlows(const ApiVersion& version);
        Json::Value GetJsonReceivers(const ApiVersion& version);
        Json::Value GetJsonSenders(const ApiVersion& version);



        static const std::string ROOT;
        static const std::string SELF;
        static const std::string SOURCES;
        static const std::string FLOWS;
        static const std::string DEVICES;
        static const std::string SENDERS;
        static const std::string RECEIVERS;


        enum {NMOS=0, API_TYPE=1,VERSION=2,ENDPOINT=3, RESOURCE=4, TARGET=5};
        enum {SZ_BASE=0, SZ_NMOS=1, SZ_API_TYPE=2,SZ_VERSION=3,SZ_ENDPOINT=4, SZ_RESOURCE=5, SZ_TARGET=6};

};

