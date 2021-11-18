#include <iostream>
//#include "json/json.h"
#include <iostream>
#include <fstream>
#include "clientapi.h"
#include "self.h"
#include "receiver.h"
#include "testposter.h"
#include "log.h"

using namespace std;



int main()
{
    pml::LogStream::AddOutput(std::make_unique<pml::LogOutput>());
    pml::LogStream::SetOutputLevel(pml::LOG_DEBUG);

    pml::nmos::ClientApi::Get().SetPoster(make_shared<TestPoster>());
    //pml::nmos::ClientApi::Get().AddQuerySubscription(pml::nmos::ClientApi::NODES);
    //pml::nmos::ClientApi::Get().AddQuerySubscription(pml::nmos::ClientApi::DEVICES);
    pml::nmos::ClientApi::Get().AddQuerySubscription(pml::nmos::ClientApi::FLOWS, "media_type=audio/L24"); //"label=pi-ptp-node/flow/c4")
    //pml::nmos::ClientApi::Get().AddQuerySubscription(pml::nmos::ClientApi::SOURCES);
    //pml::nmos::ClientApi::Get().AddQuerySubscription(pml::nmos::ClientApi::SENDERS);
    //pml::nmos::ClientApi::Get().AddQuerySubscription(pml::nmos::ClientApi::RECEIVERS);
    pml::nmos::ClientApi::Get().Start();
    pmlLog() << "Press Key To Target" ;
    getchar();
    auto itSender = pml::nmos::ClientApi::Get().GetSenders().begin();
    auto itReceiver = pml::nmos::ClientApi::Get().GetReceivers().begin();

    if(itSender != pml::nmos::ClientApi::Get().GetSenders().end() && itReceiver != pml::nmos::ClientApi::Get().GetReceivers().end())
    {
        pmlLog() << "TARGET" ;
        pmlLog() << "-----------------------------------------------" ;
        pml::nmos::ClientApi::Get().Subscribe(itSender->first, itReceiver->first);
        pmlLog() << "Press Key To Park" ;
        getchar();
        pmlLog() << "PARK" ;
        pmlLog() << "-----------------------------------------------" ;
        pml::nmos::ClientApi::Get().Unsubscribe(itReceiver->first);

        pmlLog() << "Press Key To Get Sender Staged" ;
        getchar();
        pmlLog() << "Sender Staged" ;
        pmlLog() << "-----------------------------------------------" ;
        pml::nmos::ClientApi::Get().RequestSenderStaged(itSender->first);
        getchar();
        pmlLog() << "Sender Active" ;
        pmlLog() << "-----------------------------------------------" ;
        pml::nmos::ClientApi::Get().RequestSenderActive(itSender->first);
        getchar();
        pmlLog() << "Sender TransportFile" ;
        pmlLog() << "-----------------------------------------------" ;
        pml::nmos::ClientApi::Get().RequestSenderTransportFile(itSender->first);
        getchar();

        pmlLog() << "Sender Patch Staged" ;
        pmlLog() << "-----------------------------------------------" ;
        pml::nmos::connectionSender aConnection;
        aConnection.eActivate = pml::nmos::connection::ACT_NOW;
        aConnection.bMasterEnable = true;
        aConnection.sReceiverId = itReceiver->first;
        //aConnection.tpSender.sDestinationIp =
        pml::nmos::ClientApi::Get().PatchSenderStaged(itSender->first, aConnection);

        pmlLog() << "Sender Active" ;
        pmlLog() << "-----------------------------------------------" ;
        pml::nmos::ClientApi::Get().RequestSenderActive(itSender->first);
        getchar();


    }
    getchar();

    pml::nmos::ClientApi::Get().Stop();
}
