#pragma once
#include "server.h"
#include <string>
#include <condition_variable>
#include "microhttpd.h"
#include <thread>
#include <vector>
#include "json/json.h"
#include "version.h"
#include <sstream>

class EventPoster;
class MicroServer;

struct ConnectionInfo
{
    enum {GET=0, POST = 1, PUT=2, PATCH = 3};
    ConnectionInfo() : nType(GET), pPost(0){}
    int nType;
    std::stringstream ssData;
    MicroServer* pServer;
    MHD_PostProcessor *pPost;
};

class NmosServer;

class MicroServer : public Server
{
    public:

        /** @brief Constructor - this is called in NodeApi::StartService
        *   @param pPoster a sheared_ptr to an object of a class derived from EventPoster.
        *   @param nPort the port to listed for connections on
        **/
        MicroServer(std::shared_ptr<EventPoster> pPoster,unsigned int nPort=80);

        ///< @brief Destructor
        virtual ~MicroServer();

        /** @brief Starts the web server

        **/
        bool Init() override;


        /** @brief Stope the web server
        **/
        void Stop() override;


        static void RequestCompleted (void *cls, MHD_Connection* pConnection, void **ptr, enum MHD_RequestTerminationCode toe);
        static int DoHttpGet(MHD_Connection* pConnection, const std::string& sUrl, ConnectionInfo* pInfo);
        static int DoHttpPut(MHD_Connection* pConnection, const std::string& sUrl, ConnectionInfo* pInfo);
        static int DoHttpPatch(MHD_Connection* pConnection, const std::string& sUrl, ConnectionInfo* pInfo);
        static int DoHttpPost(MHD_Connection* pConnection, const std::string& sUrl, ConnectionInfo* pInfo);
        static int DoReply(MHD_Connection* pConnection, int nCode, const std::string& sResponse, const std::string& sContentType = "application/json");

        static int AnswerToConnection(void *cls, MHD_Connection* pConnection, const char * url, const char * method, const char * sVersion, const char * upload_data, size_t * upload_data_size, void **ptr);



    protected:

        MHD_Daemon* m_pmhd;
};

