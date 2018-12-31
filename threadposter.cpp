#include "threadposter.h"


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

void ThreadPoster::CurlDone(unsigned long nResult, const std::string& sResponse, long nType)
{
    SetReason(CURL_DONE);

    m_cv.notify_one();
}

void ThreadPoster::InstanceResolved(dnsInstance* pInstance)
{
    SetReason(INSTANCE_RESOLVED);
    m_cv.notify_one();
}

void ThreadPoster::AllForNow(const std::string& sService)
{
    SetReason(ALLFORNOW);
    m_cv.notify_one();
}

void ThreadPoster::Finished()
{
    SetReason(FINISHED);
    m_cv.notify_one();
}

void ThreadPoster::RegistrationNodeError()
{
    SetReason(REGERROR);
    m_cv.notify_one();
}

void ThreadPoster::InstanceRemoved(const std::string& sInstance)
{
    SetReason(INSTANCE_REMOVED);
    m_cv.notify_one();
}

void ThreadPoster::Target(const std::string& sReceiverId, const Sender* pSender, unsigned short nPort)
{
    SetReason(TARGET);
    m_sString = sReceiverId;
    m_nShort = nPort;
    m_cv.notify_one();
}

void ThreadPoster::PatchSender(const std::string& sSenderId, const connectionSender& conPatch, unsigned short nPort)
{
    SetReason(PATCH_SENDER);
    m_sString = sSenderId;
    m_nShort = nPort;
    m_cv.notify_one();
}

void ThreadPoster::PatchReceiver(const std::string& sReceiverId, const connectionReceiver& conPatch, unsigned short nPort)
{
    SetReason(PATCH_RECEIVER);
    m_sString = sReceiverId;
    m_nShort = nPort;
    m_cv.notify_one();
}

void ThreadPoster::ActivateSender(const std::string& sSenderId)
{
    SetReason(ACTIVATE_SENDER);
    m_sString = sSenderId;
    m_cv.notify_one();
}

void ThreadPoster::ActivateReceiver(const std::string& sReceiverId)
{
    SetReason(ACTIVATE_RECEIVER);
    m_sString = sReceiverId;
    m_cv.notify_one();
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
