#pragma once
#include "json/json.h"
#include <memory>
#include <string>
#include "RestGoose.h"
#include "nmosapiversion.h"
#include <vector>

namespace pml
{
    namespace nmos
    {
        class EventPoster;
        class NodeApiPrivate;

        using postData = std::vector<pml::restgoose::partData>;

        class NmosServer
        {
            public:
                NmosServer(std::shared_ptr<pml::restgoose::Server> pServer, const ApiVersion& version, std::shared_ptr<EventPoster> pPoster, NodeApiPrivate& api);
                virtual ~NmosServer();


            protected:


                std::vector<std::string> SplitEndpoint(const endpoint& theEndpoint);
                pml::restgoose::response JsonError(int nCode, const std::string& sError, const std::string& sDebug="");

                pml::restgoose::response ConvertPostDataToJson(const std::vector<pml::restgoose::partData>& vData);


                std::shared_ptr<pml::restgoose::Server> m_pServer;
                ApiVersion m_version;
                std::shared_ptr<EventPoster> m_pPoster;
                NodeApiPrivate& m_api;

        };
    };
};
