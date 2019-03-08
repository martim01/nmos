#include "clientposter.h"
#include "clientapi.h"
#include "clientprivate.h"
#include "log.h"

const std::string ClientPoster::STR_TYPE[5] = {"staged", "active", "transportfile", "staged", "active"};

void ClientPoster::CurlDone(unsigned long nResult, const std::string& sResponse, long nType, const std::string& sResourceId)
{
    ClientApi::Get().m_pApi->SetCurlDone(nResult, sResponse, nType, sResourceId);


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

void ClientPoster:: SenderActivated(const std::string& sSenderId)
{

}

void ClientPoster::ReceiverActivated(const std::string& sReceiverId)
{

}



void ClientZCPoster::InstanceResolved(std::shared_ptr<dnsInstance> pInstance)
{
    ClientApi::Get().m_pApi->SetInstanceResolved(pInstance);
}

void ClientZCPoster::AllForNow(const std::string& sService)
{
    ClientApi::Get().m_pApi->SetAllForNow(sService);
}

void ClientZCPoster::Finished()
{

}

void ClientZCPoster::RegistrationNodeError()
{

}

void ClientZCPoster::InstanceRemoved(std::shared_ptr<dnsInstance> pInstance)
{
    ClientApi::Get().m_pApi->SetInstanceRemoved(pInstance);
}
