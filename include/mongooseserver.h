#pragma once
#include "server.h"
#include <string>
#include <condition_variable>
#include "mongoose.h"
#include <thread>
#include <vector>
#include "json/json.h"
#include "version.h"
#include <sstream>

class EventPoster;
class NmosServer;

class MongooseServer : public Server
{
    public:

        /** @brief Constructor - this is called in NodeApi::StartService
        *   @param pPoster a sheared_ptr to an object of a class derived from EventPoster.
        *   @param nPort the port to listed for connections on
        **/
        MongooseServer(std::shared_ptr<EventPoster> pPoster,unsigned int nPort=80);

        ///< @brief Destructor
        virtual ~MongooseServer();

        /** @brief Starts the web server

        **/
        bool Init() override;

        /** @brief Stope the web server
        **/
        void Stop() override;


        void HandleEvent(mg_connection *pConnection, int nEvent, void* pData);

    protected:

        void Loop();

        void EventHttp(mg_connection *pConnection, int nEvent, void* pData);
        void StartUpload(http_message* pMessage);
        void EndUpload(mg_connection *pConnection);

        int DoHttpGet(mg_connection* pConnection, const std::string& sUrl);
        int DoHttpPut(mg_connection* pConnection, const std::string& sUrl, http_message* pMessage);
        int DoHttpPatch(mg_connection* pConnection, const std::string& sUrl, http_message* pMessage);
        int DoHttpPost(mg_connection* pConnection, const std::string& sUrl, http_message* pMessage);
        int DoReply(mg_connection* pConnection, int nCode, const std::string& sResponse, const std::string& sContentType = "application/json");

        mg_connection* m_pConnection;
        mg_mgr m_mgr;
        mg_serve_http_opts m_ServerOpts;

        bool m_bRun;


};


