#pragma once
#include <string>

#include "microhttpd.h"
class MicroServer;

struct ConnectionInfo
{
    enum {GET=0, PUT=1};
    ConnectionInfo() : nType(GET), pPost(0){}
    int nType;
    MicroServer* pServer;
    MHD_PostProcessor *pPost;
};

class MicroServer
{
    public:

        MicroServer();
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

        static int IteratePost (void * ptr, MHD_ValueKind kind, const char *key, const char *filename, const char *content_type, const char *transfer_encoding, const char *data, uint64_t off, size_t size);
        static void RequestCompleted (void *cls, MHD_Connection* pConnection, void **ptr, enum MHD_RequestTerminationCode toe);
        static int DoHttpGet(MHD_Connection* pConnection, std::string sUrl, ConnectionInfo* pInfo);
        static int DoHttpPut(MHD_Connection* pConnection, std::string sUrl, ConnectionInfo* pInfo);
        static int AnswerToConnection(void *cls, MHD_Connection* pConnection, const char * url, const char * method, const char * sVersion, const char * upload_data, size_t * upload_data_size, void **ptr);


    private:
        unsigned short m_nPort;

        MHD_Daemon* m_pmhd;

        std::string m_sPut;
};

