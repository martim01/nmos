#include <iostream>
//#include "json/json.h"
#include <iostream>
#include <fstream>
#include "clientapi.h"
#include "self.h"
#include "receiverbase.h"
#include "testposter.h"
#include "log.h"

using namespace std;



int main()
{
    pml::LogStream::AddOutput(std::make_unique<pml::LogOutput>());
    pml::LogStream::SetOutputLevel(pml::LOG_DEBUG);

    pml::nmos::ClientApi::Get().SetPoster(make_shared<TestPoster>());
    pml::nmos::ClientApi::Get().AddQuerySubscription(pml::nmos::ClientApi::NODES);
    pml::nmos::ClientApi::Get().AddQuerySubscription(pml::nmos::ClientApi::DEVICES);
    pml::nmos::ClientApi::Get().AddQuerySubscription(pml::nmos::ClientApi::FLOWS); //"label=pi-ptp-node/flow/c4")
    pml::nmos::ClientApi::Get().AddQuerySubscription(pml::nmos::ClientApi::SOURCES);
    pml::nmos::ClientApi::Get().AddQuerySubscription(pml::nmos::ClientApi::SENDERS, "transport=urn:x-nmos:transport:rtp.mcast");
    pml::nmos::ClientApi::Get().AddQuerySubscription(pml::nmos::ClientApi::RECEIVERS, "transport=urn:x-nmos:transport:rtp.mcast");
    pml::nmos::ClientApi::Get().Start();
    pmlLog() << "Press Key To Target" ;
    getchar();
    auto itSender = pml::nmos::ClientApi::Get().GetSenders().begin();
    auto itReceiver = pml::nmos::ClientApi::Get().GetReceivers().begin();

    if(itSender != pml::nmos::ClientApi::Get().GetSenders().end() && itReceiver != pml::nmos::ClientApi::Get().GetReceivers().end())
    {
//        pmlLog() << "TARGET" ;
//        pmlLog() << "-----------------------------------------------" ;
//        pml::nmos::ClientApi::Get().Subscribe(itSender->first, itReceiver->first);
//        pmlLog() << "Press Key To Park" ;
//        getchar();
//        pmlLog() << "PARK" ;
//        pmlLog() << "-----------------------------------------------" ;
//        pml::nmos::ClientApi::Get().Unsubscribe(itReceiver->first);

        pmlLog() << "Press Key To Get Sender Staged" ;
        getchar();

        pmlLog() << "Sender Staged" ;
        pmlLog() << "-----------------------------------------------" ;
        auto resp = pml::nmos::ClientApi::Get().RequestSenderStaged(itSender->first, false);
        pmlLog() << resp.first.nCode;
        if(resp.second)
        {
            pmlLog() << (*resp.second).GetJson(pml::nmos::ApiVersion(1,2));
        }

        pmlLog() << "-----------------------------------------------" ;
        pmlLog() << "Press Key To Get Sender Active" ;
        getchar();

        pmlLog() << "Sender Active" ;
        pmlLog() << "-----------------------------------------------" ;
        auto respSA = pml::nmos::ClientApi::Get().RequestSenderActive(itSender->first, false);
        pmlLog() << respSA.first.nCode;
        if(respSA.second)
        {
            pmlLog() << (*respSA.second).GetJson(pml::nmos::ApiVersion(1,2));
        }

        pmlLog() << "-----------------------------------------------" ;
        pmlLog() << "Press Key To Get Sender Transportfile" ;
        getchar();

        pmlLog() << "Sender TransportFile" ;
        pmlLog() << "-----------------------------------------------" ;
        auto respST = pml::nmos::ClientApi::Get().RequestSenderTransportFile(itSender->first, false);
        pmlLog() << respST.first.nCode;
        if(respST.second)
        {
            pmlLog() << (*respST.second);
        }

        pmlLog() << "-----------------------------------------------" ;
        pmlLog() << "Press Key To Get Sender Constraints" ;
        getchar();

        pmlLog() << "Sender Constraints" ;
        pmlLog() << "-----------------------------------------------" ;
        auto respSC = pml::nmos::ClientApi::Get().RequestSenderConstraints(itSender->first, false);
        pmlLog() << respSC.first.nCode;
        if(respSC.second)
        {
            for(const auto con : (*respSC.second))
            {
                pmlLog() << con.GetJson(pml::nmos::ApiVersion(1,2));
            }

        }
        pmlLog() << "-----------------------------------------------" ;
//        pmlLog() << "Patch Sender" ;
//        pmlLog() << "-----------------------------------------------" ;
//
//        auto resp = pml::nmos::ClientApi::Get().PatchSenderStaged(itSender->first, aConnection);
//        pmlLog() << resp.first.nCode;
//        if(resp.second)
//        {
//          pmlLog() << (*resp.second).GetJson(ApiVersion(1,2));
//        }
//
//        pmlLog() << "Sender Active" ;
//        pmlLog() << "-----------------------------------------------" ;
//        pml::nmos::ClientApi::Get().RequestSenderActive(itSender->first);
//        getchar();


    }
    pmlLog()<< "Finished: Press key to exit";
    getchar();

    pml::nmos::ClientApi::Get().Stop();
}
