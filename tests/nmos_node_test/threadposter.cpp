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

void ThreadPoster::CurlDone(unsigned long nResult, const std::string& sResponse, long nType, const std::string& sResourceId)
{
    SetReason(CURL_DONE);

    LaunchThread();
}

void ThreadPoster::InstanceResolved(std::shared_ptr<pml::nmos::dnsInstance> pInstance)
{
    SetReason(INSTANCE_RESOLVED);
    LaunchThread();
}

void ThreadPoster::AllForNow(const std::string& sService)
{
    SetReason(ALLFORNOW);
    LaunchThread();

}

void ThreadPoster::Finished()
{
    SetReason(FINISHED);
    LaunchThread();
}

void ThreadPoster::RegistrationNodeError()
{
    SetReason(REGERROR);
    LaunchThread();
}

void ThreadPoster::InstanceRemoved(std::shared_ptr<pml::nmos::dnsInstance> pInstance)
{
    SetReason(INSTANCE_REMOVED);
    LaunchThread();
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

