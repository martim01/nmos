#pragma once
#include <string>
#include <condition_variable>
#include "microhttpd.h"
#include <thread>
#include <vector>
#include "json/json.h"


class RegistryServer;

struct RegistryInfo
{
    enum {GET=0, POST = 1, PUT=2, PATCH = 3, DELETE=4};
    RegistryInfo() : nType(GET), pPost(0){}
    int nType;
    RegistryServer* pServer;
    MHD_PostProcessor *pPost;
};


class RegistryServer
{
    public:

        /** @brief Constructor - this is called in NodeApi::StartService
        *   @param pPoster a sheared_ptr to an object of a class derived from EventPoster.
        **/
        RegistryServer();

        ///< @brief Destructor
        ~RegistryServer();

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
        static int DoHttpGet(MHD_Connection* pConnection, std::string sUrl, RegistryInfo* pInfo);
        static int DoHttpPost(MHD_Connection* pConnection, std::string sUrl, RegistryInfo* pInfo);
        static int DoHttpDelete(MHD_Connection* pConnection, std::string sUrl, RegistryInfo* pInfo);
        static int AnswerToConnection(void *cls, MHD_Connection* pConnection, const char * url, const char * method, const char * sVersion, const char * upload_data, size_t * upload_data_size, void **ptr);




    protected:

        int GetJson(std::string sPath, std::string& sResponse, std::string& sContentType);
        int DeleteJson(std::string sPath, std::string& sResponse);
        int PostJson(std::string sPath, const std::string& sJson, std::string& sRepsonse, std::string& sLocation);



    private:


        int GetJsonNmos(std::string& sReturn, std::string& sContentType);
        int GetJsonNmosResource(std::string& sReturn);
        int GetJsonNmosHealth(std::string& sReturn);

        int PostJsonNmosResource(const std::string& sJson, std::string& sReturn, std::string& sLocation);
        int PostJsonNmosHealth(std::string& sReturn);



        Json::Value GetJsonError(unsigned long nCode = 404, std::string sError="Resource not found");



        unsigned short m_nPort;

        MHD_Daemon* m_pmhd;
        std::string m_sPut;

        std::vector<std::string> m_vPath;

        std::mutex m_mutex;
        std::condition_variable m_cvSync;
        enum enumSignal{WAIT, FAIL, SUCCESS};

        enumSignal m_eOk;


        /*
        /x-nmos-/registration/{version}/resource/{type}/{id}
        /x-nmos-/registration/{version}/health/nodes/{id}
        */
        enum {BASE=0, NMOS=1, API=2, VERSION=3, RES_HEALTH=4};
        enum {RESOURCE_TYPE=5, RESOURCE_ID=6};
        enum {HEALTH_NODES=5, HEALTH_ID=6};
        enum {SZ_BASE=1, SZ_NMOS=2, SZ_API=3, SZ_VERSION=4, SZ_RES_HEALTH=5,SZ_RES_TYPE=6,SZ_ID=7};
};


