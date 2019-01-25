#include <iostream>
//#include "json/json.h"
#include <iostream>
#include <fstream>
#include "log.h"
#include "clientapi.h"
#include "self.h"

using namespace std;



int main()
{
    ClientApi::Get().Start();
    getchar();
    map<string, shared_ptr<Sender> >::const_iterator itSender = ClientApi::Get().GetSenderBegin();
    map<string, shared_ptr<Receiver> >::const_iterator itReceiver = ClientApi::Get().GetReceiverBegin();

    if(itSender != ClientApi::Get().GetSenderEnd() && itReceiver != ClientApi::Get().GetReceiverEnd())
    {
        cout << "TARGET" << endl;
        cout << "-----------------------------------------------" << endl;
        ClientApi::Get().Subscribe(itSender->first, itReceiver->first);
        getchar();
        cout << "PARK" << endl;
        cout << "-----------------------------------------------" << endl;
        ClientApi::Get().Unsubscribe(itReceiver->first);
    }
    getchar();

    ClientApi::Get().Stop();
}
