#pragma once
#include <string>

struct MHD_Daemon;
struct MHD_PostProcessor;
struct MHD_Connection;

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
    private:
        unsigned short m_nPort;

        MHD_Daemon* m_pmhd;

        std::string m_sPut;
};

