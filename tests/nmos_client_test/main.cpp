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

        pmlLog(pml::LOG_INFO, "pml::nmos") << "Press Key To Get Sender Staged" ;
        getchar();

        pmlLog(pml::LOG_INFO, "pml::nmos") << "Sender Staged" ;
        pmlLog(pml::LOG_INFO, "pml::nmos") << "-----------------------------------------------" ;
        auto resp = pml::nmos::ClientApi::Get().RequestSenderStaged(itSender->first, true);
        pmlLog(pml::LOG_INFO, "pml::nmos") << resp.first.nHttpCode;
        if(resp.second)
        {
            pmlLog(pml::LOG_INFO, "pml::nmos") << (*resp.second).GetJson();
        }

        pmlLog(pml::LOG_INFO, "pml::nmos") << "-----------------------------------------------" ;
        pmlLog(pml::LOG_INFO, "pml::nmos") << "Press Key To Get Sender Active" ;
        getchar();

        pmlLog(pml::LOG_INFO, "pml::nmos") << "Sender Active" ;
        pmlLog(pml::LOG_INFO, "pml::nmos") << "-----------------------------------------------" ;
        auto respSA = pml::nmos::ClientApi::Get().RequestSenderActive(itSender->first, false);
        pmlLog(pml::LOG_INFO, "pml::nmos") << respSA.first.nHttpCode;
        if(respSA.second)
        {
            pmlLog(pml::LOG_INFO, "pml::nmos") << (*respSA.second).GetJson();
        }

        pmlLog(pml::LOG_INFO, "pml::nmos") << "-----------------------------------------------" ;
        pmlLog(pml::LOG_INFO, "pml::nmos") << "Press Key To Get Sender Transportfile" ;
        getchar();

        pmlLog(pml::LOG_INFO, "pml::nmos") << "Sender TransportFile" ;
        pmlLog(pml::LOG_INFO, "pml::nmos") << "-----------------------------------------------" ;
        auto respST = pml::nmos::ClientApi::Get().RequestSenderTransportFile(itSender->first, false);
        pmlLog(pml::LOG_INFO, "pml::nmos") << respST.first.nHttpCode;
        if(respST.second)
        {
            pmlLog(pml::LOG_INFO, "pml::nmos") << (*respST.second);
        }

        pmlLog(pml::LOG_INFO, "pml::nmos") << "-----------------------------------------------" ;
        pmlLog(pml::LOG_INFO, "pml::nmos") << "Press Key To Get Sender Constraints" ;
        getchar();

        pmlLog(pml::LOG_INFO, "pml::nmos") << "Sender Constraints" ;
        pmlLog(pml::LOG_INFO, "pml::nmos") << "-----------------------------------------------" ;
        auto respSC = pml::nmos::ClientApi::Get().RequestSenderConstraints(itSender->first, false);
        pmlLog(pml::LOG_INFO, "pml::nmos") << respSC.first.nHttpCode;
        for(const auto& con : respSC.second)
        {
            pmlLog(pml::LOG_INFO, "pml::nmos") << con.GetJson();
        }
        pmlLog(pml::LOG_INFO, "pml::nmos") << "-----------------------------------------------" ;
//        pmlLog(pml::LOG_INFO, "pml::nmos") << "Patch Sender" ;
//        pmlLog(pml::LOG_INFO, "pml::nmos") << "-----------------------------------------------" ;
//
//        auto resp = pml::nmos::ClientApi::Get().PatchSenderStaged(itSender->first, aConnection);
//        pmlLog(pml::LOG_INFO, "pml::nmos") << resp.first.nHttpCode;
//        if(resp.second)
//        {
//          pmlLog(pml::LOG_INFO, "pml::nmos") << (*resp.second).GetJson(ApiVersion(1,2));
//        }
//
//        pmlLog(pml::LOG_INFO, "pml::nmos") << "Sender Active" ;
//        pmlLog(pml::LOG_INFO, "pml::nmos") << "-----------------------------------------------" ;
//        pml::nmos::ClientApi::Get().RequestSenderActive(itSender->first);
//        getchar();


    }
    pmlLog(pml::LOG_INFO, "pml::nmos")<< "Finished: Press key to exit";
    getchar();

    pml::nmos::ClientApi::Get().Stop();
}
