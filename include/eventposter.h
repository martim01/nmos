#pragma once
#include "nmosdlldefine.h"
#include <string>
#include <thread>
#include <mutex>

struct dnsInstance;
class Sender;
class connectionSender;
class connectionReceiver;

class NMOS_EXPOSE EventPoster
{
    public:
        //EventPoster()=0;
        //virtual ~EventPoster()=0;

        void _CurlDone(unsigned long nResult, const std::string& sResponse, long nType, const std::string& sResourceId=std::string());
        void _InstanceResolved(std::shared_ptr<dnsInstance> pInstance);
        void _AllForNow(const std::string& sService);
        void _Finished();
        void _RegistrationNodeError();
        void _InstanceRemoved(std::shared_ptr<dnsInstance> pInstance);
        void _Target(const std::string& sReceiverId, const std::string& sTransportFile, unsigned short nPort);
        void _PatchSender(const std::string& sSenderId, const connectionSender& conPatch, unsigned short nPort);
        void _PatchReceiver(const std::string& sReceiverId, const connectionReceiver& conPatch, unsigned short nPort);
        void _ActivateSender(const std::string& sSenderId);
        void _ActivateReceiver(const std::string& sReceiverId);

    protected:

        /** @brief Called by CurlRegister when a request to a webserver is complete
        *   @param nResult the result of the request. 0 = means no connection, otherwise an HTTP header response code
        *   @param sResonse contains the response from the server
        *   @param nType the type of request made to the server
        *   @param sResourceId is the id of the resource the request was made against if any
        *   @note this is a non-blocking event
        **/
        virtual void CurlDone(unsigned long nResult, const std::string& sResponse, long nType, const std::string& sResourceId)=0;

        /** @brief Called by AvahiBrowser when a dns service instance has been resolved
        *   @param pInstance a pointer to the resolved instance.
        *   @note The pointer must not be deleted as it belongs to AvahiBrowser
        *   @note this is a non-blocking event
        **/
        virtual void InstanceResolved(std::shared_ptr<dnsInstance> pInstance)=0;

        /** @brief Called by AvahiBrowser when no more instances of the given service are likely to be found
        *   @param sService the name of the service
        *   @note this is a non-blocking event
        **/
        virtual void AllForNow(const std::string& sService)=0;

        /** @brief Called by AvahiBrowser when it has definitely finished browsing
        *   @note this is a non-blocking event
        **/
        virtual void Finished()=0;

        /** @brief Called by AvahiBrowser when an error occurs
        *   @note this is a non-blocking event
        **/
        virtual void RegistrationNodeError()=0;

        /** @brief Called by AvahiBrowser when a dns service instance has been removed
        *   @param pInstance a pointer to the removed instance.
        *   @param sService the name of the service.
        *   @note this is a non-blocking event
        **/
        virtual void InstanceRemoved(std::shared_ptr<dnsInstance> pInstance)=0;

        /** @brief Called by MicroServer when a IS-04 Target PUT is performed
        *   @param sReceiverId the uuid of the Receiver to apply the PU to
        *   @param sTransportFile - the SDP served up by the sender
        *   @param nPort - the MicroServer port that the request came from
        *   @note this is a blocking event. The target of this function should call the NodeApi::TargetTaken function once it is ready for the MicroServer to continue
        **/
        virtual void Target(const std::string& sReceiverId, const std::string& sTransportFile, unsigned short nPort)=0;

        /** @brief Called by MicroServer when a IS-05 Sender/Staged PATCH is performed
        *   @param sSenderId the uuid of the Sender to apply the patch to
        *   @param conPatch the connectionSender details that should be applied to the Sender
        *   @param nPort - the MicroServer port that the request came from
        *   @note this is a blocking event. The target of this function should call the NodeApi::SenderPatchAllowed function once it is ready for the MicroServer to continue
        **/
        virtual void PatchSender(const std::string& sSenderId, const connectionSender& conPatch, unsigned short nPort)=0;

        /** @brief Called by MicroServer when a IS-05 Receiver/Staged PATCH is performed
        *   @param sReceiverId the uuid of the Receiver to apply the patch to
        *   @param conPatch the connectionReceiver details that should be applied to the Receiver
        *   @param nPort - the MicroServer port that the request came from
        *   @note this is a blocking event. The target of this function should call the NodeApi::ReceiverPatchAllowed function once it is ready for the MicroServer to continue
        **/
        virtual void PatchReceiver(const std::string& sReceiverId, const connectionReceiver& conPatch, unsigned short nPort)=0;

        /** @brief Called by ActivateThread telling the main thread to activate the staged paramaters - this is called at the point of the act of activation
        *   @param sSenderId the Id of the sender to activate
        *   @note The main thread should call the Sender's Activate function to update the connection API
        **/
        virtual void ActivateSender(const std::string& sSenderId)=0;

        /** @brief Called by ActivateThread telling the main thread to activate the staged paramaters - this is called at the point of the act of activation
        *   @param sReceiverId the Id of the receiver to activate
        *   @note The main thread should call the Receivers's Activate function to update the connection API
        **/
        virtual void ActivateReceiver(const std::string& sReceiverId)=0;


    private:
        std::mutex m_mutex;
};
