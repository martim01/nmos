#pragma once
#include "nmosdlldefine.h"
#include <string>
#include <map>
#include <memory>
#include "connection.h"

class Self;
class Device;
class Source;
class Flow;
class Sender;
class Receiver;
class ClientApiPrivate;

class NMOS_EXPOSE ClientApi
{
    public:

        enum flagResource {NODES=1, DEVICES=2, SOURCES=4, FLOWS=8, SENDERS=16, RECEIVERS=32, ALL=63};

        static ClientApi& Get();

        void Start(flagResource eInterested=ALL);
        void Stop();

        void ChangeInterest(flagResource eInterest);

        bool Subscribe(const std::string& sSenderId, const std::string& sReceiverId);
        bool Unsubscribe(const std::string& sReceiverId);
        bool RequestSenderStaged(const std::string& sSenderId);
        bool RequestSenderActive(const std::string& sSenderId);
        bool RequestSenderTransportFile(const std::string& sSenderId);

        bool RequestReceiverStaged(const std::string& sReceiverId);
        bool RequestReceiverActive(const std::string& sReceiverId);

        bool PatchSenderStaged(const std::string& sSenderId, const connectionSender& aConnection);
        bool PatchReceiverStaged(const std::string& sReceiverId, const connectionReceiver& aConnection);

        std::map<std::string, std::shared_ptr<Self> >::const_iterator GetNodeBegin();
        std::map<std::string, std::shared_ptr<Self> >::const_iterator GetNodeEnd();
        std::map<std::string, std::shared_ptr<Self> >::const_iterator FindNode(const std::string& sUid);
        std::map<std::string, std::shared_ptr<Device> >::const_iterator GetDeviceBegin();
        std::map<std::string, std::shared_ptr<Device> >::const_iterator GetDeviceEnd();
        std::map<std::string, std::shared_ptr<Device> >::const_iterator FindDevice(const std::string& sUid);
        std::map<std::string, std::shared_ptr<Source> >::const_iterator GetSourceBegin();
        std::map<std::string, std::shared_ptr<Source> >::const_iterator GetSourceEnd();
        std::map<std::string, std::shared_ptr<Source> >::const_iterator FindSource(const std::string& sUid);
        std::map<std::string, std::shared_ptr<Flow> >::const_iterator GetFlowBegin();
        std::map<std::string, std::shared_ptr<Flow> >::const_iterator GetFlowEnd();
        std::map<std::string, std::shared_ptr<Flow> >::const_iterator FindFlow(const std::string& sUid);
        std::map<std::string, std::shared_ptr<Sender> >::const_iterator GetSenderBegin();
        std::map<std::string, std::shared_ptr<Sender> >::const_iterator GetSenderEnd();
        std::map<std::string, std::shared_ptr<Sender> >::const_iterator FindSender(const std::string& sUid);
        std::map<std::string, std::shared_ptr<Receiver> >::const_iterator GetReceiverBegin();
        std::map<std::string, std::shared_ptr<Receiver> >::const_iterator GetReceiverEnd();
        std::map<std::string, std::shared_ptr<Receiver> >::const_iterator FindReceiver(const std::string& sUid);


    private:
        friend void NodeBrowser();
        friend void ClientThread();
        friend class ClientPoster;

        ClientApi();
        ~ClientApi();

        ClientApiPrivate* m_pApi;
};
