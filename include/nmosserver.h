#pragma once
#include "json/json.h"
#include <memory>
#include <string>
#include "RestGoose.h"
#include "nmosapiversion.h"

class RestGoose;

namespace pml
{
    namespace nmos
    {
        class EventPoster;
        class NodeApiPrivate;

        class NmosServer
        {
            public:
                NmosServer(std::shared_ptr<RestGoose> pServer, const ApiVersion& version, std::shared_ptr<EventPoster> pPoster, NodeApiPrivate& api);
                virtual ~NmosServer();


            protected:


                std::vector<std::string> SplitUrl(const url& theUrl);
                response JsonError(int nCode, const std::string& sError, const std::string& sDebug="");



                std::shared_ptr<RestGoose> m_pServer;
                ApiVersion m_version;
                std::shared_ptr<EventPoster> m_pPoster;
                NodeApiPrivate& m_api;

        };
    };
};
