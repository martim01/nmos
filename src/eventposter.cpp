#include "eventposter.h"
#include "sender.h"
#include "receiver.h"


void EventPoster::_CurlDone(unsigned long nResult, const std::string& sResponse, long nType, const std::string& sResourceId)
{
    std::lock_guard<std::mutex> lg(m_mutex);
    CurlDone(nResult, sResponse, nType, sResourceId);
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

void EventPoster::_SenderActivated(const std::string& sSenderId)
{
    std::lock_guard<std::mutex> lg(m_mutex);
    SenderActivated(sSenderId);
}

void EventPoster::_ReceiverActivated(const std::string& sReceiverId)
{
    std::lock_guard<std::mutex> lg(m_mutex);
    ReceiverActivated(sReceiverId);
}
