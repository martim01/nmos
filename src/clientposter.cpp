#include "clientposter.h"
#include "clientapi.h"
#include "clientprivate.h"
#include "log.h"

using namespace pml::nmos;



void ClientZCPoster::InstanceResolved(std::shared_ptr<dnsInstance> pInstance)
{
    m_pImpl->SetInstanceResolved(pInstance);
}

void ClientZCPoster::AllForNow(const std::string& sService)
{
    m_pImpl->SetAllForNow(sService);
}

void ClientZCPoster::Finished()
{

}

void ClientZCPoster::RegistrationNodeError()
{

}

void ClientZCPoster::InstanceRemoved(std::shared_ptr<dnsInstance> pInstance)
{
    m_pImpl->SetInstanceRemoved(pInstance);
}
