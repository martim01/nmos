#pragma once
#include "nmosdlldefine.h"
#include <string>
#include <thread>
#include <mutex>

namespace pml
{
    namespace nmos
    {
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
                void _Target(const std::string& sReceiverId, const std::string& sTransportFile, unsigned short nPort);
                void _PatchSender(const std::string& sSenderId, const connectionSender& conPatch, unsigned short nPort);
                void _PatchReceiver(const std::string& sReceiverId, const connectionReceiver& conPatch, unsigned short nPort);
                void _SenderActivated(const std::string& sSenderId);
                void _ReceiverActivated(const std::string& sReceiverId);

            protected:

                /** @brief Called by CurlRegister when a request to a webserver is complete
                *   @param nResult the result of the request. 0 = means no connection, otherwise an HTTP header response code
                *   @param sResonse contains the response from the server
                *   @param nType the type of request made to the server
                *   @param sResourceId is the id of the resource the request was made against if any
                *   @note this is a non-blocking event
                **/
                virtual void CurlDone(unsigned long nResult, const std::string& sResponse, long nType, const std::string& sResourceId)=0;


                /** @brief Called by Server when a IS-04 Target PUT is performed
                *   @param sReceiverId the uuid of the Receiver to apply the PU to
                *   @param sTransportFile - the SDP served up by the sender
                *   @param nPort - the Server port that the request came from
                *   @note this is a blocking event. The target of this function should call the NodeApi::TargetTaken function once it is ready for the Server to continue
                **/
                virtual void Target(const std::string& sReceiverId, const std::string& sTransportFile, unsigned short nPort)=0;

                /** @brief Called by Server when a IS-05 Sender/Staged PATCH is performed
                *   @param sSenderId the uuid of the Sender to apply the patch to
                *   @param conPatch the connectionSender details that should be applied to the Sender
                *   @param nPort - the Server port that the request came from
                *   @note this is a blocking event. The target of this function should call the NodeApi::SenderPatchAllowed function once it is ready for the Server to continue
                **/
                virtual void PatchSender(const std::string& sSenderId, const connectionSender& conPatch, unsigned short nPort)=0;

                /** @brief Called by Server when a IS-05 Receiver/Staged PATCH is performed
                *   @param sReceiverId the uuid of the Receiver to apply the patch to
                *   @param conPatch the connectionReceiver details that should be applied to the Receiver
                *   @param nPort - the Server port that the request came from
                *   @note this is a blocking event. The target of this function should call the NodeApi::ReceiverPatchAllowed function once it is ready for the Server to continue
                **/
                virtual void PatchReceiver(const std::string& sReceiverId, const connectionReceiver& conPatch, unsigned short nPort)=0;

                /** @brief Called by ActivateThread telling the main thread that the sender's staged parameters have been activated - this is called at the point of the act of activation
                *   @param sSenderId the Id of the sender to activate
                **/
                virtual void SenderActivated(const std::string& sSenderId)=0;

                /** @brief Called by ActivateThread telling the main thread that the receviers's staged parameters have been activated - this is called at the point of the act of activation
                *   @param sReceiverId the Id of the receiver to activate
                *   @note The main thread should call the Receivers's Activate function to update the connection API
                **/
                virtual void ReceiverActivated(const std::string& sReceiverId)=0;


            private:
                std::mutex m_mutex;
        };
    };
};
