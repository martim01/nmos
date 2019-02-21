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
        virtual ~MicroServer();

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



        void Signal(bool bOk, const std::string& sData);


    protected:

        int GetJson(std::string sPath, std::string& sResponse, std::string& sContentType);
        int PutJson(std::string sPath, const std::string& sJson, std::string& sRepsonse);
        int PatchJson(std::string sPath, const std::string& sJson, std::string& sRepsonse);

        void Wait();
        void PrimeWait();



        bool IsOk();

        virtual int GetJsonNmos(std::string& sReturn, std::string& sContentType);
        virtual int PutJsonNmos(const std::string& sJson, std::string& sResponse);
        virtual int PatchJsonNmos(const std::string& sJson, std::string& sResponse);


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


};

