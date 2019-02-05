#include <iostream>
//#include "json/json.h"
#include <iostream>
#include <fstream>
#include "log.h"
#include "clientapi.h"
#include "self.h"
#include "receiver.h"
#include "testposter.h"

using namespace std;



int main()
{
    ClientApi::Get().SetPoster(make_shared<TestPoster>());

    ClientApi::Get().Start();
    cout << "Press Key To Target" << endl;
    getchar();
    map<string, shared_ptr<Sender> >::const_iterator itSender = ClientApi::Get().GetSenderBegin();
    map<string, shared_ptr<Receiver> >::const_iterator itReceiver = ClientApi::Get().GetReceiverBegin();

    if(itSender != ClientApi::Get().GetSenderEnd() && itReceiver != ClientApi::Get().GetReceiverEnd())
    {
        cout << "TARGET" << endl;
        cout << "-----------------------------------------------" << endl;
        ClientApi::Get().Subscribe(itSender->first, itReceiver->first);
        cout << "Press Key To Park" << endl;
        getchar();
        cout << "PARK" << endl;
        cout << "-----------------------------------------------" << endl;
        ClientApi::Get().Unsubscribe(itReceiver->first);

        cout << "Press Key To Get Sender Staged" << endl;
        getchar();
        cout << "Sender Staged" << endl;
        cout << "-----------------------------------------------" << endl;
        ClientApi::Get().RequestSenderStaged(itSender->first);
        getchar();
        cout << "Sender Active" << endl;
        cout << "-----------------------------------------------" << endl;
        ClientApi::Get().RequestSenderActive(itSender->first);
        getchar();
        cout << "Sender TransportFile" << endl;
        cout << "-----------------------------------------------" << endl;
        ClientApi::Get().RequestSenderTransportFile(itSender->first);
        getchar();

        cout << "Sender Patch Staged" << endl;
        cout << "-----------------------------------------------" << endl;
        connectionSender aConnection
        aConnection.eActivate = connection::ACT_NOW;
        aConnection.bMasterEnable = true;
        aConnection.sReceiverId = itReceiver->first;
        //aConnection.tpSender.sDestinationIp =
        ClientApi::Get().PatchSenderStaged(itSender->first, aConnection);

        cout << "Sender Active" << endl;
        cout << "-----------------------------------------------" << endl;
        ClientApi::Get().RequestSenderActive(itSender->first);
        getchar();


    }
    getchar();

    ClientApi::Get().Stop();
}
