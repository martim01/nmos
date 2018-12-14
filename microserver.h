#pragma once
#include <string>

struct MHD_Daemon;
struct MHD_PostProcessor;
struct MHD_Connection;


struct ConnectionInfo
{
    enum {GET=0, PUT=1};
    ConnectionInfo() : nType(GET), pPost(0){}
    int nType;
    MHD_PostProcessor *pPost;
};

class MicroServer
{
    public:

        static MicroServer& Get();
        bool Init(unsigned int nPort=80);
        void Stop();

        void AddPutData(std::string sData);
        std::string GetPutData() const;
        void ResetPutData();

    private:
        MicroServer();
        ~MicroServer();

        MHD_Daemon* m_pmhd;

        std::string m_sPut;
};

