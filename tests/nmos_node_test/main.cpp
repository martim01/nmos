#include <iostream>
//#include "json/json.h"
#include <iostream>
#include <fstream>
#include "threadposter.h"
#include "nodeapi.h"
#include "../../log/include/log.h"
#include "sourceaudio.h"
#include "flowaudioraw.h"
#include "threadpool.h"
using namespace std;



int main()
{

    pml::LogStream::AddOutput(make_unique<pml::LogOutput>());
    pml::LogStream::SetOutputLevel(pml::LOG_DEBUG);



    auto pPoster = std::make_shared<ThreadPoster>();

    pml::nmos::NodeApi::Get().Init(pPoster, 8080, 8080, "MattTest", "host Description");
    pml::nmos::NodeApi::Get().SetHeartbeatTime(5000);
    pml::nmos::NodeApi::Get().GetSelf().AddInternalClock("clk0");
    pml::nmos::NodeApi::Get().GetSelf().AddPTPClock("clk1", false, "IEEE1588-2008", "08-00-11-ff-fe-21-e1-b0", true);
    pml::nmos::NodeApi::Get().GetSelf().AddInterface("eth0");
    pml::nmos::NodeApi::Get().GetSelf().AddTags({{"location", "W1"},{"location", "MCR1"},{"Author", "Matt"}});

    auto pDevice = make_shared<pml::nmos::Device>("MattTest/Device/1", "TestDescription", pml::nmos::Device::GENERIC,pml::nmos::NodeApi::Get().GetSelf().GetId());

    auto pSource = make_shared<pml::nmos::SourceAudio>("MattTest/Source/Audio", "TestDescription", pDevice->GetId());
    pSource->AddChannels({{channelSymbol("L"), channelLabel("Left")},{channelSymbol("R"), channelLabel("Right")}});
    pSource->SetClock("clk1");

    auto pFlow = make_shared<pml::nmos::FlowAudioRaw>("MattTest/Flow/AudioRaw", "TestDescription", pSource->GetId(), pDevice->GetId(),48000, pml::nmos::FlowAudioRaw::L24);
    pFlow->SetPacketTime(pml::nmos::FlowAudioRaw::US_125);
    pFlow->SetMediaClkOffset(0);

    std::vector<std::string> vInterfaces{"eth0", "eth1"};
    std::vector<std::string> vMulticast{"239.100.1.20", "239.100.2.20"};
    auto pSender = make_shared<pml::nmos::Sender>("MattTest/Sender/Audio", "Description", pFlow->GetId(), pml::nmos::Sender::RTP_MCAST, pDevice->GetId(), vInterfaces, pml::nmos::TransportParamsRTP::REDUNDANT, vMulticast);


    pSender->MasterEnable(true);

    auto pReceiver = make_shared<pml::nmos::Receiver>("MattTest/Receiver/Audio", "TestDescription", pml::nmos::Receiver::RTP_MCAST, pDevice->GetId(), pml::nmos::Receiver::AUDIO, pml::nmos::TransportParamsRTP::MULTICAST);
    pReceiver->AddCaps({"audio/L24","audio/L20","audio/L16"});
    pReceiver->AddInterfaceBinding("eth0");

    if(pml::nmos::NodeApi::Get().AddDevice(pDevice) == false)
    {
        pml::log::log(pml::log::Level::kError, "pml::nmos") << "FAILED TO ADD DEVICE";
    }
    if(pml::nmos::NodeApi::Get().AddSource(pSource) == false)
    {
        pml::log::log(pml::log::Level::kError, "pml::nmos") << "FAILED TO ADD SOURCE";
    }
    if(pml::nmos::NodeApi::Get().AddFlow(pFlow) == false)
    {
        pml::log::log(pml::log::Level::kError, "pml::nmos") << "FAILED TO ADD FLOW";
    }
    if(pml::nmos::NodeApi::Get().AddReceiver(pReceiver) == false)
    {
        pml::log::log(pml::log::Level::kError, "pml::nmos") << "FAILED TO ADD RECEIVER";
    }
    if(pml::nmos::NodeApi::Get().AddSender(pSender) == false)
    {
        pml::log::log(pml::log::Level::kError, "pml::nmos") << "FAILED TO ADD SENDER";
    }
    pml::nmos::NodeApi::Get().Commit();



    pml::nmos::NodeApi::Get().StartServices();

    int nCount(0);
    while(true)
    {
        if(pPoster->Wait(chrono::milliseconds(100)))
        {
            switch(pPoster->GetReason())
            {
                case ThreadPoster::CURL_DONE:
                    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "----------------------------------------";
                    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "Curl Done";
                    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "----------------------------------------";
                    break;
                case ThreadPoster::INSTANCE_RESOLVED:
                    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "----------------------------------------";
                    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "Browser: Instance Resolved";
                    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "----------------------------------------";
                    break;
                case ThreadPoster::ALLFORNOW:
                    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "----------------------------------------";
                    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "Browser: All For Now";
                    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "----------------------------------------";
                    break;
                case ThreadPoster::FINISHED:
                    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "----------------------------------------";
                    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "Browser: Finished";
                    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "----------------------------------------";
                    break;
                case ThreadPoster::REGERROR:
                    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "----------------------------------------";
                    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "Publisher: Error";
                    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "----------------------------------------";
                    break;
                case ThreadPoster::INSTANCE_REMOVED:
                    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "----------------------------------------";
                    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "Browser: Instance Removed";
                    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "----------------------------------------";
                    break;
                case ThreadPoster::TARGET:
                    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "----------------------------------------";
                    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "NMOS Target: " << pPoster->GetString() << " [" << pPoster->GetSDP() << "]";
                    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "----------------------------------------";
                    //getchar();
                    pml::nmos::NodeApi::Get().TargetTaken("192.168.1.113", pPoster->GetPort(), true);
                    break;
                case ThreadPoster::PATCH_SENDER:
                    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "----------------------------------------";
                    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "NMOS Patch Sender: " << pPoster->GetString();
                    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "----------------------------------------";
                    pml::nmos::NodeApi::Get().SenderPatchAllowed(pPoster->GetPort(), true, pPoster->GetString(),vMulticast);
                    break;
                case ThreadPoster::PATCH_RECEIVER:
                    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "----------------------------------------";
                    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "NMOS Patch Receiver: " << pPoster->GetString();
                    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "----------------------------------------";
                    pml::nmos::NodeApi::Get().ReceiverPatchAllowed(pPoster->GetPort(), true, pPoster->GetString(), "192.168.1.113");
                    break;
                case ThreadPoster::ACTIVATE_SENDER:
                    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "----------------------------------------";
                    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "NMOS Sender Activated: " << pPoster->GetString();
                    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "----------------------------------------";
                    break;
                case ThreadPoster::ACTIVATE_RECEIVER:
                    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "----------------------------------------";
                    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "NMOS Receiver Activated: " << pPoster->GetString();
                    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "----------------------------------------";
                    break;
            }

        }
//        if(nCount < 100)
//        {
//            cout << "CHANGE PACKET TIME IN " << (100-nCount);
//
//        }
//        nCount++;
//        if(nCount == 100)
//        {
//
//            pFlow->SetPacketTime(FlowAudioRaw::US_250);
//            NodeApi::Get().Commit();
//        }
    }
    pml::nmos::NodeApi::Get().StopServices();
    return 0;
    return 0;
}
