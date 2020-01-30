#pragma once
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

class MicroServer
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
        bool Init();

        /** Add ann NMOS server
        *   @param sControl the first part of the url (after the x-nmos/)
        *   @param pServer the server
        **/
        bool AddNmosControl(const std::string& sControl, std::shared_ptr<NmosServer> pServer);

        /** @brief Stope the web server
        **/
        void Stop();


        /** @brief Gets the port number that the web server is using to listen for connections
        *   @return <i>unsigned short</i> the port number
        **/
        unsigned short GetPort() const
        {
            return m_nPort;
        }



        static void RequestCompleted (void *cls, MHD_Connection* pConnection, void **ptr, enum MHD_RequestTerminationCode toe);
        static int DoHttpGet(MHD_Connection* pConnection, const std::string& sUrl, ConnectionInfo* pInfo);
        static int DoHttpPut(MHD_Connection* pConnection, const std::string& sUrl, ConnectionInfo* pInfo);
        static int DoHttpPatch(MHD_Connection* pConnection, const std::string& sUrl, ConnectionInfo* pInfo);
        static int DoHttpPost(MHD_Connection* pConnection, const std::string& sUrl, ConnectionInfo* pInfo);
        static int DoReply(MHD_Connection* pConnection, int nCode, const std::string& sResponse, const std::string& sContentType = "application/json");

        static int AnswerToConnection(void *cls, MHD_Connection* pConnection, const char * url, const char * method, const char * sVersion, const char * upload_data, size_t * upload_data_size, void **ptr);


        void Wait();
        void PrimeWait();
        bool IsOk();
        void Signal(bool bOk, const std::string& sData);
        const std::string& GetSignalData();


    protected:

        int GetJson(std::string sPath, std::string& sResponse, std::string& sContentType);
        int PutJson(std::string sPath, const std::string& sJson, std::string& sRepsonse);
        int PatchJson(std::string sPath, const std::string& sJson, std::string& sRepsonse);
        int PostJson(std::string sPath, const std::string& sJson, std::string& sRepsonse);
        int DeleteJson(std::string sPath, const std::string& sJson, std::string& sRepsonse);

        int GetApis(std::string& sReturn);

        Json::Value GetJsonError(unsigned long nCode = 404, const std::string& sError="Resource not found");




        std::shared_ptr<EventPoster> m_pPoster;

        MHD_Daemon* m_pmhd;
        unsigned short m_nPort;


        std::vector<std::string> m_vPath;

        std::mutex m_mutex;
        std::condition_variable m_cvSync;
        enum enumSignal{WAIT, FAIL, SUCCESS};

        enumSignal m_eOk;

        std::string m_sSignalData;

        std::map<std::string, std::shared_ptr<NmosServer> > m_mServer;

};

