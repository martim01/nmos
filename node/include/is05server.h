#pragma once
#include <string>
#include <condition_variable>
#include <thread>
#include <vector>
#include "json/json.h"
#include "nmosapiversion.h"
#include "nmosserver.h"

namespace pml
{
    namespace restgoose
    {
        class Server;
        struct response;
    }

    namespace nmos
    {
        class Sender;
        class Receiver;

        class IS05Server : public NmosServer
        {
            public:

                /** @brief Constructor - this is called in NodeApi::StartService
                *   @param pPoster a sheared_ptr to an object of a class derived from EventPoster.
                **/
                IS05Server(std::shared_ptr<pml::restgoose::Server> pServer, const ApiVersion& version, std::shared_ptr<EventPoster> pPoster, NodeApiPrivate& api);
                virtual ~IS05Server();


                void AddSenderEndpoint(const std::string& sId);
                void AddReceiverEndpoint(const std::string& sId);

                void RemoveSenderEndpoint(const std::string& sId);
                void RemoveReceiverEndpoint(const std::string& sId);

                pml::restgoose::response GetNmosRoot(const query& theQuery, const postData& theData, const endpoint& theEndpoint, const userName& theUser);
                pml::restgoose::response GetNmosVersion(const query& theQuery, const postData& theData, const endpoint& theEndpoint, const userName& theUser);
                pml::restgoose::response GetNmosBulk(const query& theQuery, const postData& theData, const endpoint& theEndpoint, const userName& theUser);
                pml::restgoose::response GetNmosBulkSenders(const query& theQuery, const postData& theData, const endpoint& theEndpoint, const userName& theUser);
                pml::restgoose::response PostNmosBulkSenders(const query& theQuery, const postData& theData, const endpoint& theEndpoint, const userName& theUser);
                pml::restgoose::response GetNmosBulkReceivers(const query& theQuery, const postData& theData, const endpoint& theEndpoint, const userName& theUser);
                pml::restgoose::response PostNmosBulkReceivers(const query& theQuery, const postData& theData, const endpoint& theEndpoint, const userName& theUser);
                pml::restgoose::response GetNmosSingle(const query& theQuery, const postData& theData, const endpoint& theEndpoint, const userName& theUser);
                pml::restgoose::response GetNmosSingleSenders(const query& theQuery, const postData& theData, const endpoint& theEndpoint, const userName& theUser);
                pml::restgoose::response GetNmosSingleReceivers(const query& theQuery, const postData& theData, const endpoint& theEndpoint, const userName& theUser);
                pml::restgoose::response GetNmosSingleSender(const query& theQuery, const postData& theData, const endpoint& theEndpoint, const userName& theUser);
                pml::restgoose::response GetNmosSingleSenderConstraints(const query& theQuery, const postData& theData, const endpoint& theEndpoint, const userName& theUser);
                pml::restgoose::response GetNmosSingleSenderStaged(const query& theQuery, const postData& theData, const endpoint& theEndpoint, const userName& theUser);
                pml::restgoose::response GetNmosSingleSenderActive(const query& theQuery, const postData& theData, const endpoint& theEndpoint, const userName& theUser);
                pml::restgoose::response GetNmosSingleSenderTransportfile(const query& theQuery, const postData& theData, const endpoint& theEndpoint, const userName& theUser);
                pml::restgoose::response GetNmosSingleSenderTransportType(const query& theQuery, const postData& theData, const endpoint& theEndpoint, const userName& theUser);
                pml::restgoose::response PatchNmosSingleSenderStaged(const query& theQuery, const postData& theData, const endpoint& theEndpoint, const userName& theUser);
                pml::restgoose::response GetNmosSingleReceiver(const query& theQuery, const postData& theData, const endpoint& theEndpoint, const userName& theUser);
                pml::restgoose::response GetNmosSingleReceiverConstraints(const query& theQuery, const postData& theData, const endpoint& theEndpoint, const userName& theUser);
                pml::restgoose::response GetNmosSingleReceiverStaged(const query& theQuery, const postData& theData, const endpoint& theEndpoint, const userName& theUser);
                pml::restgoose::response GetNmosSingleReceiverActive(const query& theQuery, const postData& theData, const endpoint& theEndpoint, const userName& theUser);
                pml::restgoose::response GetNmosSingleReceiverTransportType(const query& theQuery, const postData& theData, const endpoint& theEndpoint, const userName& theUser);
                pml::restgoose::response PatchNmosSingleReceiverStaged(const query& theQuery, const postData& theData, const endpoint& theEndpoint, const userName& theUser);

            private:
                void AddBaseEndpoints();

                std::shared_ptr<Sender> GetSender(const endpoint& theEndpoint);
                std::shared_ptr<Receiver> GetReceiver(const endpoint& theEndpoint);



                pml::restgoose::response PatchSender(std::shared_ptr<Sender> pSender, const Json::Value& jsRequest);
                pml::restgoose::response PatchReceiver(std::shared_ptr<Receiver> pReceiver, const Json::Value& jsRequest);
                pml::restgoose::response PostJsonSenders(const pml::restgoose::response& request);
                pml::restgoose::response PostJsonReceivers(const pml::restgoose::response& request);

                static const std::string ROOT;
                static const std::string BULK;
                static const std::string SENDERS;
                static const std::string RECEIVERS;
                static const std::string SINGLE;
                static const std::string CONSTRAINTS;
                static const std::string STAGED;
                static const std::string ACTIVE;
                static const std::string TRANSPORTFILE;
                static const std::string TRANSPORTTYPE;


                static const size_t RESOURCE_ID = 5;

        };
    };
};

