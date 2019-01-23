#include "clientposter.h"
#include "clientapi.h"

void ClientPoster::CurlDone(unsigned long nResult, const std::string& sResponse, long nType)
{

}

void ClientPoster::InstanceResolved(std::shared_ptr<dnsInstance> pInstance)
{
    ClientApi::Get().SetInstanceResolved(pInstance);
}

void ClientPoster::AllForNow(const std::string& sService)
{

}

void ClientPoster::Finished()
{

}

void ClientPoster::RegistrationNodeError()
{

}

void ClientPoster::InstanceRemoved(std::shared_ptr<dnsInstance> pInstance)
{
    ClientApi::Get().SetInstanceRemoved(pInstance);
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
