#include "clientposter.h"
#include "clientapi.h"
#include "clientprivate.h"
#include "log.h"

const std::string ClientPoster::STR_TYPE[5] = {"staged", "active", "transportfile", "staged", "active"};

void ClientPoster::CurlDone(unsigned long nResult, const std::string& sResponse, long nType, const std::string& sResourceId)
{
    ClientApi::Get().m_pApi->SetCurlDone(nResult, sResponse, nType, sResourceId);


}

void ClientPoster::InstanceResolved(std::shared_ptr<dnsInstance> pInstance)
{
    ClientApi::Get().m_pApi->SetInstanceResolved(pInstance);
}

void ClientPoster::AllForNow(const std::string& sService)
{
    ClientApi::Get().m_pApi->SetAllForNow(sService);
}

void ClientPoster::Finished()
{

}

void ClientPoster::RegistrationNodeError()
{

}

void ClientPoster::InstanceRemoved(std::shared_ptr<dnsInstance> pInstance)
{
    ClientApi::Get().m_pApi->SetInstanceRemoved(pInstance);
}



void ClientPoster::Target(const std::string& sReceiverId, const std::string& sTransportFile, unsigned short nPort)
{

}

void ClientPoster::PatchSender(const std::string& sSenderId, const connectionSender& conPatch, unsigned short nPort)
{

}

void ClientPoster::PatchReceiver(const std::string& sReceiverId, const connectionReceiver& conPatch, unsigned short nPort)
{

}

void ClientPoster:: ActivateSender(const std::string& sSenderId)
{

}

void ClientPoster::ActivateReceiver(const std::string& sReceiverId)
{

}
