#include "threadposter.h"
#include <thread>
#include <chrono>

static void Notify(ThreadPoster* pPoster)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    pPoster->Signal();
}

bool ThreadPoster::Wait(std::chrono::milliseconds ms)
{
    std::unique_lock<std::mutex> ul(m_mutexMain);

    return (m_cv.wait_for(ul, ms) == std::cv_status::no_timeout);
}

unsigned int ThreadPoster::GetReason()
{
    std::lock_guard<std::mutex> lg(m_mutexMain);
    return m_nReason;
}

void ThreadPoster::SetReason(unsigned int nReason)
{
    std::lock_guard<std::mutex> lg(m_mutexMain);
    m_nReason = nReason;
}




void ThreadPoster::Target(const std::string& sReceiverId, const std::string& sTransportFile, unsigned short nPort)
{
    SetReason(TARGET);
    m_sString = sReceiverId;
    m_nShort = nPort;
    m_sSDP = sTransportFile;
    LaunchThread();
}

void ThreadPoster::PatchSender(const std::string& sSenderId, const pml::nmos::connectionSender& conPatch, unsigned short nPort)
{
    SetReason(PATCH_SENDER);
    m_sString = sSenderId;
    m_nShort = nPort;

    LaunchThread();
}

void ThreadPoster::PatchReceiver(const std::string& sReceiverId, const pml::nmos::connectionReceiver& conPatch, unsigned short nPort)
{
    SetReason(PATCH_RECEIVER);
    m_sString = sReceiverId;
    m_nShort = nPort;
    LaunchThread();
}

void ThreadPoster::SenderActivated(const std::string& sSenderId)
{
    SetReason(ACTIVATE_SENDER);
    m_sString = sSenderId;
    LaunchThread();
}

void ThreadPoster::ReceiverActivated(const std::string& sReceiverId)
{
    SetReason(ACTIVATE_RECEIVER);
    m_sString = sReceiverId;
    LaunchThread();
}

void ThreadPoster::RegistrationNodeFound(const std::string& sUrl, unsigned short nPriority, const pml::nmos::ApiVersion& version)
{
    SetReason(REGISTRATION_NODE_FOUND);
    m_sString = sUrl;
    m_nShort = nPriority;
    LaunchThread();
}

void ThreadPoster::RegistrationNodeRemoved(const std::string& sUrl)
{
    SetReason(REGISTRATION_NODE_REMOVED);
    m_sString = sUrl;
    LaunchThread();
}

void ThreadPoster::RegistrationNodeChanged(const std::string& sUrl, unsigned short nPriority, bool bGood, const pml::nmos::ApiVersion& version)
{
    SetReason(REGISTRATION_NODE_CHANGED);
    m_sString = sUrl;
    m_nShort = nPriority;
    m_nLong = bGood;
    LaunchThread();
}

void ThreadPoster::RegistrationNodeChosen(const std::string& sUrl, unsigned short nPriority, const pml::nmos::ApiVersion& version)
{
    SetReason(REGISTRATION_NODE_CHOSEN);
    m_sString = sUrl;
    m_nShort = nPriority;
    LaunchThread();
}

void ThreadPoster::RegistrationChanged(const std::string& sUrl, bool bRegistered)
{
    SetReason(REGISTRATION_CHANGED);
    m_sString = sUrl;
    m_nShort = bRegistered;
    LaunchThread();
}


const std::string& ThreadPoster::GetString()
{
    std::lock_guard<std::mutex> lg(m_mutexMain);
    return m_sString;
}

unsigned short ThreadPoster::GetPort()
{
    std::lock_guard<std::mutex> lg(m_mutexMain);
    return m_nShort;
}

void ThreadPoster::Signal()
{
    m_cv.notify_one();
}


void ThreadPoster::LaunchThread()
{
    std::thread thNotify(Notify, this);
    thNotify.detach();
}

