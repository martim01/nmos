#include <iostream>
//#include "json/json.h"
#include <iostream>
#include <fstream>
#include "clientapi.h"
#include "self.h"
#include "receiver.h"
#include "testposter.h"

using namespace std;



int main()
{
    pml::nmos::ClientApi::Get().SetPoster(make_shared<TestPoster>());
    pml::nmos::ClientApi::Get().AddQuerySubscription(pml::nmos::ClientApi::NODES);
    pml::nmos::ClientApi::Get().AddQuerySubscription(pml::nmos::ClientApi::DEVICES);
    pml::nmos::ClientApi::Get().AddQuerySubscription(pml::nmos::ClientApi::FLOWS);
    pml::nmos::ClientApi::Get().AddQuerySubscription(pml::nmos::ClientApi::SOURCES);
    pml::nmos::ClientApi::Get().AddQuerySubscription(pml::nmos::ClientApi::SENDERS);
    pml::nmos::ClientApi::Get().AddQuerySubscription(pml::nmos::ClientApi::RECEIVERS);
    pml::nmos::ClientApi::Get().Start();
    cout << "Press Key To Target" << endl;
    getchar();
    auto itSender = pml::nmos::ClientApi::Get().GetSenders().begin();
    auto itReceiver = pml::nmos::ClientApi::Get().GetReceivers().begin();

    if(itSender != pml::nmos::ClientApi::Get().GetSenders().end() && itReceiver != pml::nmos::ClientApi::Get().GetReceivers().end())
    {
        cout << "TARGET" << endl;
        cout << "-----------------------------------------------" << endl;
        pml::nmos::ClientApi::Get().Subscribe(itSender->first, itReceiver->first);
        cout << "Press Key To Park" << endl;
        getchar();
        cout << "PARK" << endl;
        cout << "-----------------------------------------------" << endl;
        pml::nmos::ClientApi::Get().Unsubscribe(itReceiver->first);

        cout << "Press Key To Get Sender Staged" << endl;
        getchar();
        cout << "Sender Staged" << endl;
        cout << "-----------------------------------------------" << endl;
        pml::nmos::ClientApi::Get().RequestSenderStaged(itSender->first);
        getchar();
        cout << "Sender Active" << endl;
        cout << "-----------------------------------------------" << endl;
        pml::nmos::ClientApi::Get().RequestSenderActive(itSender->first);
        getchar();
        cout << "Sender TransportFile" << endl;
        cout << "-----------------------------------------------" << endl;
        pml::nmos::ClientApi::Get().RequestSenderTransportFile(itSender->first);
        getchar();

        cout << "Sender Patch Staged" << endl;
        cout << "-----------------------------------------------" << endl;
        pml::nmos::connectionSender aConnection;
        aConnection.eActivate = pml::nmos::connection::ACT_NOW;
        aConnection.bMasterEnable = true;
        aConnection.sReceiverId = itReceiver->first;
        //aConnection.tpSender.sDestinationIp =
        pml::nmos::ClientApi::Get().PatchSenderStaged(itSender->first, aConnection);

        cout << "Sender Active" << endl;
        cout << "-----------------------------------------------" << endl;
        pml::nmos::ClientApi::Get().RequestSenderActive(itSender->first);
        getchar();


    }
    getchar();

    pml::nmos::ClientApi::Get().Stop();
}
