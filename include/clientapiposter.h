#pragma once
#include "nmosdlldefine.h"
#include <string>
#include <memory>
#include <set>


class Self;
class Device;
class Source;
class Flow;
class Sender;
class Receiver;

class NMOS_EXPOSE ClientApiPoster
{
    public:
        ClientApiPoster(){};
        ~ClientApiPoster(){};

        enum enumChange{RESOURCE_ADDED=1, RESOURCE_UPDATED};

        void _NodeChanged(std::shared_ptr<Self> pNode, enumChange eChange);
        void _DeviceChanged(std::shared_ptr<Device> pDevice, enumChange eChange);
        void _SourceChanged(std::shared_ptr<Source> pSource, enumChange eChange);
        void _FlowChanged(std::shared_ptr<Flow> pFlow, enumChange eChange);
        void _SenderChanged(std::shared_ptr<Sender> pSender, enumChange eChange);
        void _ReceiverChanged(std::shared_ptr<Receiver> pReceiver, enumChange eChange);

        void _NodesRemoved(const std::set<std::string>& setRemoved);
        void _DevicesRemoved(const std::set<std::string>& setRemoved);
        void _SourcesRemoved(const std::set<std::string>& setRemoved);
        void _FlowsRemoved(const std::set<std::string>& setRemoved);
        void _SendersRemoved(const std::set<std::string>& setRemoved);
        void _ReceiversRemoved(const std::set<std::string>& setRemoved);

        void _RequestTargetResult(unsigned long nResult, const std::string& sResponse, const std::string& sResourceId);
        void _RequestPatchSenderResult(unsigned long nResult, const std::string& sResponse, const std::string& sResourceId);
        void _RequestPatchReceiverResult(unsigned long nResult, const std::string& sResponse, const std::string& sResourceId);

        void _RequestConnectResult(const std::string& sSenderId, const std::string& sReceiverId, bool bSuccess, const std::string& sResponse);

    protected:

        /** @brief Called when a Self is either added or changed
        *   @param pNode a shared_ptr to the added or updated f
        *   @param eChange an enumChanged reflecting whether the Self was added or updated
        **/
        virtual void NodeChanged(std::shared_ptr<Self> pNode, enumChange eChange)=0;

        /** @brief Called when a Device is either added or changed
        *   @param pDevice a shared_ptr to the added or updated Device
        *   @param eChange an enumChanged reflecting whether the Device was added or updated
        **/
        virtual void DeviceChanged(std::shared_ptr<Device> pDevice, enumChange eChange)=0;

        /** @brief Called when a Source is either added or changed
        *   @param pSource a shared_ptr to the added or updated Source
        *   @param eChange an enumChanged reflecting whether the Source was added or updated
        **/
        virtual void SourceChanged(std::shared_ptr<Source> pSource, enumChange eChange)=0;

        /** @brief Called when a Flow is either added or changed
        *   @param pFlow a shared_ptr to the added or updated Flow
        *   @param eChange an enumChanged reflecting whether the Flow was added or updated
        **/
        virtual void FlowChanged(std::shared_ptr<Flow> pFlow, enumChange eChange)=0;

        /** @brief Called when a Sender is either added or changed
        *   @param pSender a shared_ptr to the added or updated Sender
        *   @param eChange an enumChanged reflecting whether the Sender was added or updated
        **/
        virtual void SenderChanged(std::shared_ptr<Sender> pSender, enumChange eChange)=0;

        /** @brief Called when a Receiver is either added or changed
        *   @param pReceiver a shared_ptr to the added or updated Receiver
        *   @param eChange an enumChanged reflecting whether the Receiver was added or updated
        **/
        virtual void ReceiverChanged(std::shared_ptr<Receiver> pReceiver, enumChange eChange)=0;

        /** @brief Called when one or more Self are removed
        *   @param setRemoved set of Self uuIds
        **/
        virtual void NodesRemoved(const std::set<std::string>& setRemoved)=0;

        /** @brief Called when one or more Devices are removed
        *   @param setRemoved set of Device uuIds
        **/
        virtual void DevicesRemoved(const std::set<std::string>& setRemoved)=0;

        /** @brief Called when one or more Sources are removed
        *   @param setRemoved set of Source uuIds
        **/
        virtual void SourcesRemoved(const std::set<std::string>& setRemoved)=0;

        /** @brief Called when one or more Flows are removed
        *   @param setRemoved set of Flows uuIds
        **/
        virtual void FlowsRemoved(const std::set<std::string>& setRemoved)=0;

        /** @brief Called when one or more Senders are removed
        *   @param setRemoved set of Sender uuIds
        **/
        virtual void SendersRemoved(const std::set<std::string>& setRemoved)=0;

        /** @brief Called when one or more Receivers are removed
        *   @param setRemoved set of Receiver uuIds
        **/
        virtual void ReceiversRemoved(const std::set<std::string>& setRemoved)=0;

        /** @brief Called when a ClientApi::Subscribe or ClientApi::Unsubscribe gets an answer from the target webserver
        *   @param nResult the http result code
        *   @param sResonse on success this will contain the uuId of the Sender, of failure this will contain a message with the reason for failure
        *   @param sResourceId contains the uuId of the Receiver
        **/
        virtual void RequestTargetResult(unsigned long nResult, const std::string& sResponse, const std::string& sResourceId)=0;


        virtual void RequestPatchSenderResult(unsigned long nResult, const std::string& sResponse, const std::string& sResourceId)=0;
        virtual void RequestPatchReceiverResult(unsigned long nResult, const std::string& sResponse, const std::string& sResourceId)=0;

        virtual void RequestConnectResult(const std::string& sSenderId, const std::string& sReceiverId, bool bSuccess, const std::string& sResponse)=0;

    private:

};
