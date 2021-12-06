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

    auto pSender = make_shared<pml::nmos::Sender>("MattTest/Sender/Audio", "Description", pFlow->GetId(), pml::nmos::Sender::RTP_MCAST, pDevice->GetId(), "eth0", pml::nmos::TransportParamsRTP::CORE, std::string("239.255.255.20"));
    pSender->MasterEnable(true);

    auto pReceiver = make_shared<pml::nmos::Receiver>("MattTest/Receiver/Audio", "TestDescription", pml::nmos::Receiver::RTP_MCAST, pDevice->GetId(), pml::nmos::Receiver::AUDIO,
    (pml::nmos::TransportParamsRTP::flagsTP(pml::nmos::TransportParamsRTP::CORE | pml::nmos::TransportParamsRTP::MULTICAST)));
    pReceiver->AddCaps({"audio/L24","audio/L20","audio/L16"});
    pReceiver->AddInterfaceBinding("eth0");

    if(pml::nmos::NodeApi::Get().AddDevice(pDevice) == false)
    {
        pmlLog(pml::LOG_ERROR) << "FAILED TO ADD DEVICE";
    }
    if(pml::nmos::NodeApi::Get().AddSource(pSource) == false)
    {
        pmlLog(pml::LOG_ERROR) << "FAILED TO ADD SOURCE";
    }
    if(pml::nmos::NodeApi::Get().AddFlow(pFlow) == false)
    {
        pmlLog(pml::LOG_ERROR) << "FAILED TO ADD FLOW";
    }
    if(pml::nmos::NodeApi::Get().AddReceiver(pReceiver) == false)
    {
        pmlLog(pml::LOG_ERROR) << "FAILED TO ADD RECEIVER";
    }
    if(pml::nmos::NodeApi::Get().AddSender(pSender) == false)
    {
        pmlLog(pml::LOG_ERROR) << "FAILED TO ADD SENDER";
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
                    pmlLog() << "----------------------------------------";
                    pmlLog() << "Curl Done";
                    pmlLog() << "----------------------------------------";
                    break;
                case ThreadPoster::INSTANCE_RESOLVED:
                    pmlLog() << "----------------------------------------";
                    pmlLog() << "Browser: Instance Resolved";
                    pmlLog() << "----------------------------------------";
                    break;
                case ThreadPoster::ALLFORNOW:
                    pmlLog() << "----------------------------------------";
                    pmlLog() << "Browser: All For Now";
                    pmlLog() << "----------------------------------------";
                    break;
                case ThreadPoster::FINISHED:
                    pmlLog() << "----------------------------------------";
                    pmlLog() << "Browser: Finished";
                    pmlLog() << "----------------------------------------";
                    break;
                case ThreadPoster::REGERROR:
                    pmlLog() << "----------------------------------------";
                    pmlLog() << "Publisher: Error";
                    pmlLog() << "----------------------------------------";
                    break;
                case ThreadPoster::INSTANCE_REMOVED:
                    pmlLog() << "----------------------------------------";
                    pmlLog() << "Browser: Instance Removed";
                    pmlLog() << "----------------------------------------";
                    break;
                case ThreadPoster::TARGET:
                    pmlLog() << "----------------------------------------";
                    pmlLog() << "NMOS Target: " << pPoster->GetString() << " [" << pPoster->GetSDP() << "]";
                    pmlLog() << "----------------------------------------";
                    //getchar();
                    pml::nmos::NodeApi::Get().TargetTaken("192.168.1.113", pPoster->GetPort(), true);
                    break;
                case ThreadPoster::PATCH_SENDER:
                    pmlLog() << "----------------------------------------";
                    pmlLog() << "NMOS Patch Sender: " << pPoster->GetString();
                    pmlLog() << "----------------------------------------";
                    pml::nmos::NodeApi::Get().SenderPatchAllowed(pPoster->GetPort(), true, pPoster->GetString(),"","239.192.55.101");
                    break;
                case ThreadPoster::PATCH_RECEIVER:
                    pmlLog() << "----------------------------------------";
                    pmlLog() << "NMOS Patch Receiver: " << pPoster->GetString();
                    pmlLog() << "----------------------------------------";
                    pml::nmos::NodeApi::Get().ReceiverPatchAllowed(pPoster->GetPort(), true, pPoster->GetString(), "192.168.1.113");
                    break;
                case ThreadPoster::ACTIVATE_SENDER:
                    pmlLog() << "----------------------------------------";
                    pmlLog() << "NMOS Sender Activated: " << pPoster->GetString();
                    pmlLog() << "----------------------------------------";
                    break;
                case ThreadPoster::ACTIVATE_RECEIVER:
                    pmlLog() << "----------------------------------------";
                    pmlLog() << "NMOS Receiver Activated: " << pPoster->GetString();
                    pmlLog() << "----------------------------------------";
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
