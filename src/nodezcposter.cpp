#include "nodezcposter.h"
#include "nodeapi.h"


using namespace pml::nmos;

void NodeZCPoster::InstanceResolved(std::shared_ptr<dnsInstance> pInstance)
{
    NodeApi::Get().HandleInstanceResolved(pInstance);
}

void NodeZCPoster::AllForNow(const std::string& sService)
{
    NodeApi::Get().Signal(NodeApi::SIG_BROWSE_DONE);
}

void NodeZCPoster::Finished()
{

}

void NodeZCPoster::RegistrationNodeError()
{

}

void NodeZCPoster::InstanceRemoved(std::shared_ptr<dnsInstance> pInstance)
{
    NodeApi::Get().HandleInstanceRemoved(pInstance);
}

