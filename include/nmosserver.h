#pragma once
#include "json/json.h"
#include <memory>
#include <string>
#include "RestGoose.h"
#include "version.h"
class Server;
class EventPoster;

class NmosServer
{
    public:
        NmosServer(std::shared_ptr<RestGoose> pServer, const ApiVersion& version, std::shared_ptr<EventPoster> pPoster);
        virtual ~NmosServer();


    protected:


        std::vector<std::string> SplitUrl(const url& theUrl);
        response JsonError(int nCode, const std::string& sError, const std::string& sDebug="");



        std::shared_ptr<RestGoose> m_pServer;
        ApiVersion m_version;
        std::shared_ptr<EventPoster> m_pPoster;


};
