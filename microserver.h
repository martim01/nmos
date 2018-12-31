#pragma once
#include <string>
#include <condition_variable>
#include "microhttpd.h"
#include <thread>
#include <vector>
#include "json/json.h"

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

        MicroServer(std::shared_ptr<EventPoster> pPoster);
        ~MicroServer();

        bool Init(unsigned int nPort=80);
        void Stop();

        void AddPutData(std::string sData);
        std::string GetPutData() const;
        void ResetPutData();

        unsigned short GetPort() const
        {
            return m_nPort;
        }

        static void RequestCompleted (void *cls, MHD_Connection* pConnection, void **ptr, enum MHD_RequestTerminationCode toe);
        static int DoHttpGet(MHD_Connection* pConnection, std::string sUrl, ConnectionInfo* pInfo);
        static int DoHttpPut(MHD_Connection* pConnection, std::string sUrl, ConnectionInfo* pInfo);
        static int DoHttpPatch(MHD_Connection* pConnection, std::string sUrl, ConnectionInfo* pInfo);
        static int AnswerToConnection(void *cls, MHD_Connection* pConnection, const char * url, const char * method, const char * sVersion, const char * upload_data, size_t * upload_data_size, void **ptr);




        unsigned char GetResponseCode();

    protected:
        friend class NodeApi;

        int GetJson(std::string sPath, std::string& sResponse);
        int PutJson(std::string sPath, const std::string& sJson, std::string& sRepsonse);
        int PatchJson(std::string sPath, const std::string& sJson, std::string& sRepsonse);


        void Wait();
        void Signal(unsigned char nCode);

    private:

        void SplitString(std::vector<std::string>& vSplit, std::string str, char cSplit);

        int GetJsonNmos(std::string& sReturn);
        int GetJsonNmosNodeApi(std::string& sReturn);
        Json::Value GetJsonSources();
        Json::Value GetJsonDevices();
        Json::Value GetJsonFlows();
        Json::Value GetJsonReceivers();
        Json::Value GetJsonSenders();
        int GetJsonNmosConnectionApi(std::string& sReturn);
        int GetJsonNmosConnectionSingleApi(std::string& sReturn);
        int GetJsonNmosConnectionBulkApi(std::string& sReturn);

        int GetJsonNmosConnectionSingleSenders(std::string& sReturn);
        int GetJsonNmosConnectionSingleReceivers(std::string& sReturn);

        int PatchJsonSender(const std::string& sJson, std::string& sResponse);
        int PatchJsonReceiver(const std::string& sJson, std::string& sResponse);

        Json::Value GetJsonError(unsigned long nCode = 404, std::string sError="Resource not found");



        unsigned short m_nPort;
        std::shared_ptr<EventPoster> m_pPoster;

        MHD_Daemon* m_pmhd;
        std::string m_sPut;

        std::vector<std::string> m_vPath;

        std::mutex m_mutex;
        std::condition_variable m_cvSync;
        unsigned char m_nCode;

        enum {BASE=0, NMOS=1, API_TYPE=2,VERSION=3,ENDPOINT=4, RESOURCE=5, TARGET=6};
        enum {SZ_BASE=1, SZ_NMOS=2, SZ_API_TYPE=3,SZ_VERSION=4,SZ_ENDPOINT=5, SZ_RESOURCE=6, SZ_TARGET=7};
        enum {SZC_TYPE=5, SZC_DIRECTION=6, SZC_ID=7, SZC_LAST=8};
        enum {C_TYPE = 4, C_DIRECTION=5, C_ID=6, C_LAST=7};
};

