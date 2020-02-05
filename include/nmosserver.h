#pragma once
#include "json/json.h"
#include <memory>
#include <string>

class Server;
class EventPoster;

class NmosServer
{
    public:
        NmosServer(): m_pPoster(0){}
        virtual ~NmosServer(){}

        void SetPoster(std::shared_ptr<EventPoster> pPoster,unsigned short nPort);

        virtual int GetJsonNmos(Server* pServer, std::string& sReturn, std::string& sContentType);
        virtual int PutJsonNmos(Server* pServer, const std::string& sJson, std::string& sResponse);
        virtual int PatchJsonNmos(Server* pServer, const std::string& sJson, std::string& sResponse);
        virtual int PostJsonNmos(Server* pServer, const std::string& sJson, std::string& sResponse);
        virtual int DeleteJsonNmos(Server* pServer, const std::string& sJson, std::string& sResponse);

        void SetPath(const std::vector<std::string>& vPath);

    protected:
        Json::Value GetJsonError(unsigned long nCode = 404, const std::string& sError="Resource not found");

        std::shared_ptr<EventPoster> m_pPoster;
        unsigned short m_nPort;
        std::vector<std::string> m_vPath;

};
