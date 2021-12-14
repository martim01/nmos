#include "eventposter.h"
using namespace pml::nmos;


void EventPoster::_RegistrationNodeFound(const std::string& sUrl, unsigned short nPriority, const ApiVersion& version)
{
    std::lock_guard<std::mutex> lg(m_mutex);
    RegistrationNodeFound(sUrl, nPriority,version);
}

void EventPoster::_RegistrationNodeRemoved(const std::string& sUrl)
{
    std::lock_guard<std::mutex> lg(m_mutex);
    RegistrationNodeRemoved(sUrl);
}

void EventPoster::_RegistrationNodeChanged(const std::string& sUrl, unsigned short nPriority, bool bGood, const ApiVersion& version)
{
    std::lock_guard<std::mutex> lg(m_mutex);
    RegistrationNodeChanged(sUrl, nPriority, bGood, version);
}


void EventPoster::_RegistrationChanged(const std::string& sUrl, enumRegState eState)
{
    std::lock_guard<std::mutex> lg(m_mutex);
    RegistrationChanged(sUrl, eState);
}

void EventPoster::_Target(const std::string& sReceiverId, const std::string& sTransportFile, unsigned short nPort)
{
    std::lock_guard<std::mutex> lg(m_mutex);
    Target(sReceiverId, sTransportFile, nPort);
}

void EventPoster::_PatchSender(const std::string& sSenderId, const connectionSender<activationResponse>& conPatch, unsigned short nPort)
{
    std::lock_guard<std::mutex> lg(m_mutex);
    PatchSender(sSenderId, conPatch, nPort);
}

void EventPoster::_PatchReceiver(const std::string& sReceiverId, const connectionReceiver<activationResponse>& conPatch, unsigned short nPort)
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
