#pragma once
#include <string>
#include <condition_variable>
#include <thread>
#include <vector>
#include "json/json.h"
#include "nmosapiversion.h"
#include "nmosserver.h"

class RestGoose;

namespace pml
{
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
                IS05Server(std::shared_ptr<RestGoose> pServer, const ApiVersion& version, std::shared_ptr<EventPoster> pPoster, NodeApiPrivate& api);
                virtual ~IS05Server();


                void AddSenderEndpoint(const std::string& sId);
                void AddReceiverEndpoint(const std::string& sId);

                void RemoveSenderEndpoint(const std::string& sId);
                void RemoveReceiverEndpoint(const std::string& sId);

                response GetNmosRoot(const query& theQuery, const postData& theData, const endpoint& theEndpoint, const userName& theUser);
                response GetNmosVersion(const query& theQuery, const postData& theData, const endpoint& theEndpoint, const userName& theUser);
                response GetNmosBulk(const query& theQuery, const postData& theData, const endpoint& theEndpoint, const userName& theUser);
                response GetNmosBulkSenders(const query& theQuery, const postData& theData, const endpoint& theEndpoint, const userName& theUser);
                response PostNmosBulkSenders(const query& theQuery, const postData& theData, const endpoint& theEndpoint, const userName& theUser);
                response GetNmosBulkReceivers(const query& theQuery, const postData& theData, const endpoint& theEndpoint, const userName& theUser);
                response PostNmosBulkReceivers(const query& theQuery, const postData& theData, const endpoint& theEndpoint, const userName& theUser);
                response GetNmosSingle(const query& theQuery, const postData& theData, const endpoint& theEndpoint, const userName& theUser);
                response GetNmosSingleSenders(const query& theQuery, const postData& theData, const endpoint& theEndpoint, const userName& theUser);
                response GetNmosSingleReceivers(const query& theQuery, const postData& theData, const endpoint& theEndpoint, const userName& theUser);
                response GetNmosSingleSender(const query& theQuery, const postData& theData, const endpoint& theEndpoint, const userName& theUser);
                response GetNmosSingleSenderConstraints(const query& theQuery, const postData& theData, const endpoint& theEndpoint, const userName& theUser);
                response GetNmosSingleSenderStaged(const query& theQuery, const postData& theData, const endpoint& theEndpoint, const userName& theUser);
                response GetNmosSingleSenderActive(const query& theQuery, const postData& theData, const endpoint& theEndpoint, const userName& theUser);
                response GetNmosSingleSenderTransportfile(const query& theQuery, const postData& theData, const endpoint& theEndpoint, const userName& theUser);
                response GetNmosSingleSenderTransportType(const query& theQuery, const postData& theData, const endpoint& theEndpoint, const userName& theUser);
                response PatchNmosSingleSenderStaged(const query& theQuery, const postData& theData, const endpoint& theEndpoint, const userName& theUser);
                response GetNmosSingleReceiver(const query& theQuery, const postData& theData, const endpoint& theEndpoint, const userName& theUser);
                response GetNmosSingleReceiverConstraints(const query& theQuery, const postData& theData, const endpoint& theEndpoint, const userName& theUser);
                response GetNmosSingleReceiverStaged(const query& theQuery, const postData& theData, const endpoint& theEndpoint, const userName& theUser);
                response GetNmosSingleReceiverActive(const query& theQuery, const postData& theData, const endpoint& theEndpoint, const userName& theUser);
                response GetNmosSingleReceiverTransportType(const query& theQuery, const postData& theData, const endpoint& theEndpoint, const userName& theUser);
                response PatchNmosSingleReceiverStaged(const query& theQuery, const postData& theData, const endpoint& theEndpoint, const userName& theUser);

            private:
                void AddBaseEndpoints();

                std::shared_ptr<Sender> GetSender(const endpoint& theEndpoint);
                std::shared_ptr<Receiver> GetReceiver(const endpoint& theEndpoint);



                response PatchSender(std::shared_ptr<Sender> pSender, const Json::Value& jsRequest);
                response PatchReceiver(std::shared_ptr<Receiver> pReceiver, const Json::Value& jsRequest);
                response PostJsonSenders(const Json::Value& jsRequest);
                response PostJsonReceivers(const Json::Value& jsRequest);

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

