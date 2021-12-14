#pragma once
#include "nmosdlldefine.h"
#include <string>
#include <thread>
#include <mutex>
#include "nmosapiversion.h"
#include "connection.h"
#include "activation.h"

namespace pml
{
    namespace nmos
    {

        class Sender;

        class NMOS_EXPOSE EventPoster
        {
            public:
                //EventPoster()=0;
                //virtual ~EventPoster()=0;
                enum enumRegState {NODE_PEER, NODE_REGISTERING, NODE_REGISTERED, NODE_REGISTER_FAILED};
                void _RegistrationNodeFound(const std::string& sUrl, unsigned short nPriority, const ApiVersion& version);
                void _RegistrationNodeRemoved(const std::string& sUrl);
                void _RegistrationNodeChanged(const std::string& sUrl, unsigned short nPriority, bool bGood, const ApiVersion& version);
                void _RegistrationChanged(const std::string& sUrl, enumRegState eState);

                void _Target(const std::string& sReceiverId, const std::string& sTransportFile, unsigned short nPort);
                void _PatchSender(const std::string& sSenderId, const connectionSender<activationResponse>& conPatch, unsigned short nPort);
                void _PatchReceiver(const std::string& sReceiverId, const connectionReceiver<activationResponse>& conPatch, unsigned short nPort);
                void _SenderActivated(const std::string& sSenderId);
                void _ReceiverActivated(const std::string& sReceiverId);

            protected:

                /** @brief Called by NodeApi when a new registration node is discovered
                *   @param sUrl the url/ip address of the node
                *   @param nPriority the priority of the register
                *   @param version the highest ApiVersion that the registration node has in common with our node
                **/
                virtual void RegistrationNodeFound(const std::string& sUrl, unsigned short nPriority, const ApiVersion& version)=0;

                /** @brief Called by NodeApi when a new registration node is remoed
                *   @param sUrl the url/ip address of the node
                **/
                virtual void RegistrationNodeRemoved(const std::string& sUrl)=0;

                /** @brief Called by NodeApi whe a property of the registration node changes
                *   @param sUrl the url/ip address of the node
                *   @param nPriority the priority of the register
                *   @param bGood whether the register is healthy or not
                *   @param version the highest ApiVersion that the registration node has in common with our node
                **/
                virtual void RegistrationNodeChanged(const std::string& sUrl, unsigned short nPriority, bool bGood, const ApiVersion& version)=0;


                /** @brief called when the node has either completed registering with a register server of unregistered
                *   @param sUrl the url/ip address of the registration node
                *   @param bRegistered true if the node has registered successfully else false
                **/
                virtual void RegistrationChanged(const std::string& sUrl, enumRegState eState)=0;

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
                virtual void PatchSender(const std::string& sSenderId, const connectionSender<activationResponse>& conPatch, unsigned short nPort)=0;

                /** @brief Called by Server when a IS-05 Receiver/Staged PATCH is performed
                *   @param sReceiverId the uuid of the Receiver to apply the patch to
                *   @param conPatch the connectionReceiver details that should be applied to the Receiver
                *   @param nPort - the Server port that the request came from
                *   @note this is a blocking event. The target of this function should call the NodeApi::ReceiverPatchAllowed function once it is ready for the Server to continue
                **/
                virtual void PatchReceiver(const std::string& sReceiverId, const connectionReceiver<activationResponse>& conPatch, unsigned short nPort)=0;

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
