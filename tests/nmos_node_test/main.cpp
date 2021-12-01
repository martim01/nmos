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
        cout << "FAILED TO ADD DEVICE" << endl;
    }
    if(pml::nmos::NodeApi::Get().AddSource(pSource) == false)
    {
        cout << "FAILED TO ADD SOURCE" << endl;
    }
    if(pml::nmos::NodeApi::Get().AddFlow(pFlow) == false)
    {
        cout << "FAILED TO ADD FLOW" << endl;
    }
    if(pml::nmos::NodeApi::Get().AddReceiver(pReceiver) == false)
    {
        cout << "FAILED TO ADD RECEIVER" << endl;
    }
    if(pml::nmos::NodeApi::Get().AddSender(pSender) == false)
    {
        cout << "FAILED TO ADD SENDER" << endl;
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
                    cout << "----------------------------------------" << endl;
                    cout << "Curl Done" << endl;
                    cout << "----------------------------------------" << endl;
                    break;
                case ThreadPoster::INSTANCE_RESOLVED:
                    cout << "----------------------------------------" << endl;
                    cout << "Browser: Instance Resolved" << endl;
                    cout << "----------------------------------------" << endl;
                    break;
                case ThreadPoster::ALLFORNOW:
                    cout << "----------------------------------------" << endl;
                    cout << "Browser: All For Now" << endl;
                    cout << "----------------------------------------" << endl;
                    break;
                case ThreadPoster::FINISHED:
                    cout << "----------------------------------------" << endl;
                    cout << "Browser: Finished" << endl;
                    cout << "----------------------------------------" << endl;
                    break;
                case ThreadPoster::REGERROR:
                    cout << "----------------------------------------" << endl;
                    cout << "Publisher: Error" << endl;
                    cout << "----------------------------------------" << endl;
                    break;
                case ThreadPoster::INSTANCE_REMOVED:
                    cout << "----------------------------------------" << endl;
                    cout << "Browser: Instance Removed" << endl;
                    cout << "----------------------------------------" << endl;
                    break;
                case ThreadPoster::TARGET:
                    cout << "----------------------------------------" << endl;
                    cout << "NMOS Target: " << pPoster->GetString() << " [" << pPoster->GetSDP() << "]" << endl;
                    cout << "----------------------------------------" << endl;
                    //getchar();
                    pml::nmos::NodeApi::Get().TargetTaken("192.168.1.113", pPoster->GetPort(), true);
                    break;
                case ThreadPoster::PATCH_SENDER:
                    cout << "----------------------------------------" << endl;
                    cout << "NMOS Patch Sender: " << pPoster->GetString() << endl;
                    cout << "----------------------------------------" << endl;
                    pml::nmos::NodeApi::Get().SenderPatchAllowed(pPoster->GetPort(), true, pPoster->GetString(),"","239.192.55.101");
                    break;
                case ThreadPoster::PATCH_RECEIVER:
                    cout << "----------------------------------------" << endl;
                    cout << "NMOS Patch Receiver: " << pPoster->GetString() << endl;
                    cout << "----------------------------------------" << endl;
                    pml::nmos::NodeApi::Get().ReceiverPatchAllowed(pPoster->GetPort(), true, pPoster->GetString(), "192.168.1.113");
                    break;
                case ThreadPoster::ACTIVATE_SENDER:
                    cout << "----------------------------------------" << endl;
                    cout << "NMOS Sender Activated: " << pPoster->GetString() << endl;
                    cout << "----------------------------------------" << endl;
                    break;
                case ThreadPoster::ACTIVATE_RECEIVER:
                    cout << "----------------------------------------" << endl;
                    cout << "NMOS Receiver Activated: " << pPoster->GetString() << endl;
                    cout << "----------------------------------------" << endl;
                    break;
            }

        }
//        if(nCount < 100)
//        {
//            cout << "CHANGE PACKET TIME IN " << (100-nCount) << endl;
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
