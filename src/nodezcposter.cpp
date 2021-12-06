#include "nodezcposter.h"
#include "nodeapiprivate.h"

using namespace pml::dnssd;

using namespace pml::nmos;

void NodeZCPoster::InstanceResolved(std::shared_ptr<dnsInstance> pInstance)
{
    m_api.HandleInstanceResolved(pInstance);
}

void NodeZCPoster::AllForNow(const std::string& sService)
{
    m_api.Signal(NodeApiPrivate::SIG_BROWSE_DONE);
}

void NodeZCPoster::Finished()
{

}

void NodeZCPoster::RegistrationNodeError()
{

}

void NodeZCPoster::InstanceRemoved(std::shared_ptr<dnsInstance> pInstance)
{
    m_api.HandleInstanceRemoved(pInstance);
}

