#include "zcposter.h"

using namespace pml::nmos;

void ZCPoster::_InstanceResolved(std::shared_ptr<dnsInstance> pInstance)
{
    std::lock_guard<std::mutex> lg(m_mutex);
    InstanceResolved(pInstance);

}

void ZCPoster::_AllForNow(const std::string& sService)
{
    std::lock_guard<std::mutex> lg(m_mutex);
    AllForNow(sService);
}

void ZCPoster::_Finished()
{
    std::lock_guard<std::mutex> lg(m_mutex);
    Finished();
}

void ZCPoster::_RegistrationNodeError()
{
    std::lock_guard<std::mutex> lg(m_mutex);
    RegistrationNodeError();
}

void ZCPoster::_InstanceRemoved(std::shared_ptr<dnsInstance> pInstance)
{
    std::lock_guard<std::mutex> lg(m_mutex);
    InstanceRemoved(pInstance);
}

