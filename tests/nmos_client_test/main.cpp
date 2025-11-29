#include <iostream>
//#include "json/json.h"
#include <iostream>
#include <fstream>
#include "clientapi.h"
#include "self.h"
#include "receiverbase.h"
#include "testposter.h"
#include "log.h"
#include "threadpool.h"

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

    auto itSender = pml::nmos::ClientApi::Get().GetSenders().begin();
    auto itReceiver = pml::nmos::ClientApi::Get().GetReceivers().begin();

    if(itSender != pml::nmos::ClientApi::Get().GetSenders().end() && itReceiver != pml::nmos::ClientApi::Get().GetReceivers().end())
    {

        pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "Press Key To Get Sender Staged" ;
        getchar();

        pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "Sender Staged" ;
        pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "-----------------------------------------------" ;
        auto resp = pml::nmos::ClientApi::Get().RequestSenderStaged(itSender->first, true);
        pml::log::log(pml::log::Level::kInfo, "pml::nmos") << resp.first.nHttpCode;
        if(resp.second)
        {
            pml::log::log(pml::log::Level::kInfo, "pml::nmos") << (*resp.second).GetJson();
        }

        pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "-----------------------------------------------" ;
        pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "Press Key To Get Sender Active" ;
        getchar();

        pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "Sender Active" ;
        pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "-----------------------------------------------" ;
        auto respSA = pml::nmos::ClientApi::Get().RequestSenderActive(itSender->first, false);
        pml::log::log(pml::log::Level::kInfo, "pml::nmos") << respSA.first.nHttpCode;
        if(respSA.second)
        {
            pml::log::log(pml::log::Level::kInfo, "pml::nmos") << (*respSA.second).GetJson();
        }

        pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "-----------------------------------------------" ;
        pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "Press Key To Get Sender Transportfile" ;
        getchar();

        pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "Sender TransportFile" ;
        pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "-----------------------------------------------" ;
        auto respST = pml::nmos::ClientApi::Get().RequestSenderTransportFile(itSender->first, false);
        pml::log::log(pml::log::Level::kInfo, "pml::nmos") << respST.first.nHttpCode;
        if(respST.second)
        {
            pml::log::log(pml::log::Level::kInfo, "pml::nmos") << (*respST.second);
        }

        pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "-----------------------------------------------" ;
        pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "Press Key To Get Sender Constraints" ;
        getchar();

        pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "Sender Constraints" ;
        pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "-----------------------------------------------" ;
        auto respSC = pml::nmos::ClientApi::Get().RequestSenderConstraints(itSender->first, false);
        pml::log::log(pml::log::Level::kInfo, "pml::nmos") << respSC.first.nHttpCode;
        for(const auto& con : respSC.second)
        {
            pml::log::log(pml::log::Level::kInfo, "pml::nmos") << con.GetJson();
        }
        pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "-----------------------------------------------" ;
//        pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "Patch Sender" ;
//        pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "-----------------------------------------------" ;
//
//        auto resp = pml::nmos::ClientApi::Get().PatchSenderStaged(itSender->first, aConnection);
//        pml::log::log(pml::log::Level::kInfo, "pml::nmos") << resp.first.nHttpCode;
//        if(resp.second)
//        {
//          pml::log::log(pml::log::Level::kInfo, "pml::nmos") << (*resp.second).GetJson(ApiVersion(1,2));
//        }
//
//        pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "Sender Active" ;
//        pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "-----------------------------------------------" ;
//        pml::nmos::ClientApi::Get().RequestSenderActive(itSender->first);
//        getchar();


    }
    pml::log::log(pml::log::Level::kInfo, "pml::nmos")<< "Finished: Press key to exit";
    getchar();

    pml::nmos::ClientApi::Get().Stop();
}
