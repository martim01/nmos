#pragma once
#include <string>
#include <condition_variable>
#include "microhttpd.h"
#include <thread>
#include <vector>
#include "json/json.h"
#include "version.h"

class EventPoster;
class MicroServer;

struct ConnectionInfo
{
    enum {GET=0, POST = 1, PUT=2, PATCH = 3};
    ConnectionInfo() : nType(GET), pPost(0){}
    int nType;
    MicroServer* pServer;
    MHD_PostProcessor *pPost;
};


class MicroServer
{
    public:

        /** @brief Constructor - this is called in NodeApi::StartService
        *   @param pPoster a sheared_ptr to an object of a class derived from EventPoster.
        **/
        MicroServer(std::shared_ptr<EventPoster> pPoster);

        ///< @brief Destructor
        ~MicroServer();

        /** @brief Starts the web server
        *   @param nPort the port to listed for connections on
        **/
        bool Init(unsigned int nPort=80);

        /** @brief Stope the web server
        **/
        void Stop();

        /** @brief Adds PUT/POST/PATCH data to a buffer ready for parsing
        *   @param sData the data
        **/
        void AddPutData(std::string sData);

        /** @brief Returns the PUT/POST/PATCH buffer
        *   @return <i>string</i> the data
        **/
        std::string GetPutData() const;

        /** @brief Clears the PUT/POST/PATCH buffer
        **/
        void ResetPutData();

        /** @brief Gets the port number that the web server is using to listen for connections
        *   @return <i>unsigned short</i> the port number
        **/
        unsigned short GetPort() const
        {
            return m_nPort;
        }



        static void RequestCompleted (void *cls, MHD_Connection* pConnection, void **ptr, enum MHD_RequestTerminationCode toe);
        static int DoHttpGet(MHD_Connection* pConnection, std::string sUrl, ConnectionInfo* pInfo);
        static int DoHttpPut(MHD_Connection* pConnection, std::string sUrl, ConnectionInfo* pInfo);
        static int DoHttpPatch(MHD_Connection* pConnection, std::string sUrl, ConnectionInfo* pInfo);
        static int AnswerToConnection(void *cls, MHD_Connection* pConnection, const char * url, const char * method, const char * sVersion, const char * upload_data, size_t * upload_data_size, void **ptr);




    protected:
        friend class NodeApi;

        int GetJson(std::string sPath, std::string& sResponse, std::string& sContentType);
        int PutJson(std::string sPath, const std::string& sJson, std::string& sRepsonse);
        int PatchJson(std::string sPath, const std::string& sJson, std::string& sRepsonse);

        void Wait();
        void PrimeWait();
        void Signal(bool bOk, const std::string& sData);


    private:


        bool IsOk();
        int GetJsonNmos(std::string& sReturn, std::string& sContentType);
        int GetJsonNmosNodeApi(std::string& sReturn);
        Json::Value GetJsonSources(const ApiVersion& version);
        Json::Value GetJsonDevices(const ApiVersion& version);
        Json::Value GetJsonFlows(const ApiVersion& version);
        Json::Value GetJsonReceivers(const ApiVersion& version);
        Json::Value GetJsonSenders(const ApiVersion& version);
        int GetJsonNmosConnectionApi(std::string& sReturn, std::string& sContentType);
        int GetJsonNmosConnectionSingleApi(std::string& sReturn, std::string& sContentType, const ApiVersion& version);
        int GetJsonNmosConnectionBulkApi(std::string& sReturn);

        int GetJsonNmosConnectionSingleSenders(std::string& sReturn, std::string& sContentType, const ApiVersion& version);
        int GetJsonNmosConnectionSingleReceivers(std::string& sReturn, const ApiVersion& version);

        int PatchJsonSender(const std::string& sJson, std::string& sResponse, const ApiVersion& version);
        int PatchJsonReceiver(const std::string& sJson, std::string& sResponse, const ApiVersion& version);

        Json::Value GetJsonError(unsigned long nCode = 404, std::string sError="Resource not found");



        unsigned short m_nPort;
        std::shared_ptr<EventPoster> m_pPoster;

        MHD_Daemon* m_pmhd;
        std::string m_sPut;

        std::vector<std::string> m_vPath;

        std::mutex m_mutex;
        std::condition_variable m_cvSync;
        enum enumSignal{WAIT, FAIL, SUCCESS};

        enumSignal m_eOk;

        std::string m_sSignalData;

        enum {BASE=0, NMOS=1, API_TYPE=2,VERSION=3,ENDPOINT=4, RESOURCE=5, TARGET=6};
        enum {SZ_BASE=1, SZ_NMOS=2, SZ_API_TYPE=3,SZ_VERSION=4,SZ_ENDPOINT=5, SZ_RESOURCE=6, SZ_TARGET=7};
        enum {SZC_TYPE=5, SZC_DIRECTION=6, SZC_ID=7, SZC_LAST=8};
        enum {C_TYPE = 4, C_DIRECTION=5, C_ID=6, C_LAST=7};
};

