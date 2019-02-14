#include "eventposter.h"
#include "nodeapi.h"
#include "sender.h"
#include "receiver.h"
#
void EventPoster::_CurlDone(unsigned long nResult, const std::string& sResponse, long nType, const std::string& sResourceId)
{
    std::lock_guard<std::mutex> lg(m_mutex);
    CurlDone(nResult, sResponse, nType, sResourceId);
}


void EventPoster::_InstanceResolved(std::shared_ptr<dnsInstance> pInstance)
{
    std::lock_guard<std::mutex> lg(m_mutex);
    InstanceResolved(pInstance);

}

void EventPoster::_AllForNow(const std::string& sService)
{
    std::lock_guard<std::mutex> lg(m_mutex);
    AllForNow(sService);
}

void EventPoster::_Finished()
{
    std::lock_guard<std::mutex> lg(m_mutex);
    Finished();
}

void EventPoster::_RegistrationNodeError()
{
    std::lock_guard<std::mutex> lg(m_mutex);
    RegistrationNodeError();
}

void EventPoster::_InstanceRemoved(std::shared_ptr<dnsInstance> pInstance)
{
    std::lock_guard<std::mutex> lg(m_mutex);
    InstanceRemoved(pInstance);
}

void EventPoster::_Target(const std::string& sReceiverId, const std::string& sTransportFile, unsigned short nPort)
{
    std::lock_guard<std::mutex> lg(m_mutex);
    Target(sReceiverId, sTransportFile, nPort);
}

void EventPoster::_PatchSender(const std::string& sSenderId, const connectionSender& conPatch, unsigned short nPort)
{
    std::lock_guard<std::mutex> lg(m_mutex);
    PatchSender(sSenderId, conPatch, nPort);
}

void EventPoster::_PatchReceiver(const std::string& sReceiverId, const connectionReceiver& conPatch, unsigned short nPort)
{
    std::lock_guard<std::mutex> lg(m_mutex);
    PatchReceiver(sReceiverId, conPatch, nPort);
}

void EventPoster::_ActivateSender(const std::string& sSenderId)
{
    std::lock_guard<std::mutex> lg(m_mutex);
    std::shared_ptr<Sender> pSender(NodeApi::Get().GetSender(sSenderId));
    if(pSender && pSender->IsActivateAllowed())
    {
        ActivateSender(sSenderId);
    }
}

void EventPoster::_ActivateReceiver(const std::string& sReceiverId)
{
    std::lock_guard<std::mutex> lg(m_mutex);
    std::shared_ptr<Receiver> pReceiver(NodeApi::Get().GetReceiver(sReceiverId));
    if(pReceiver && pReceiver->IsActivateAllowed())
    {
        ActivateReceiver(sReceiverId);
    }
}
