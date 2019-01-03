#include <iostream>
#include "json/json.h"
#include <iostream>
#include <fstream>
#include "threadposter.h"
#include "nodeapi.h"
#include "sourceaudio.h"
#include "flowaudioraw.h"
#include "device.h"
#include "sender.h"
#include "receiver.h"
#include "log.h"
using namespace std;



int main()
{
    //Log::Get().SetOutput(new wxLogOutput(this));

    NodeApi::Get().Init(8080, 8081, "host1", "host1");
    NodeApi::Get().GetSelf().AddApiVersion("v1.2");
    NodeApi::Get().GetSelf().AddInternalClock("clk0");
    NodeApi::Get().GetSelf().AddPTPClock("clk1", true, "IEEE1588-2008", "08-00-11-ff-fe-21-e1-b0", true);
    NodeApi::Get().GetSelf().AddInterface("eth0", "74-26-96-db-87-31", "74-26-96-db-87-32");


    Device* pDevice = new Device("TestDevice", "TestDescription", Device::GENERIC,NodeApi::Get().GetSelf().GetId());

    SourceAudio* pSource = new SourceAudio("TestAudio", "TestDescription", pDevice->GetId());
    pSource->AddChannel("Left", "L");
    pSource->AddChannel("Right", "R");

    FlowAudioRaw* pFlow = new FlowAudioRaw("TestFlow", "TestDescription", pSource->GetId(), pDevice->GetId(),48000, FlowAudioRaw::L24);
    Sender* pSender(new Sender("TestSender", "Description", pFlow->GetId(), Sender::RTP_MCAST, pDevice->GetId(), "http://192.168.1.35/by-name/pam.sdp"));
    pSender->AddInterfaceBinding("eth0");


    Receiver* pReceiver = new Receiver("Test Receiver", "TestDescription", Receiver::RTP_MCAST, pDevice->GetId(), Receiver::AUDIO);
    pReceiver->AddCap("audio/L24");
    pReceiver->AddCap("audio/L20");
    pReceiver->AddCap("audio/L16");
    pReceiver->AddInterfaceBinding("eth0");

    if(NodeApi::Get().AddDevice(pDevice) == false)
    {
        cout << "FAILED TO ADD DEVICE" << endl;
    }
    if(NodeApi::Get().AddSource(pSource) == false)
    {
        cout << "FAILED TO ADD SOURCE" << endl;
    }
    if(NodeApi::Get().AddFlow(pFlow) == false)
    {
        cout << "FAILED TO ADD FLOW" << endl;
    }
    if(NodeApi::Get().AddReceiver(pReceiver) == false)
    {
        cout << "FAILED TO ADD RECEIVER" << endl;
    }
    if(NodeApi::Get().AddSender(pSender) == false)
    {
        cout << "FAILED TO ADD SENDER" << endl;
    }
    NodeApi::Get().Commit();

    std::shared_ptr<ThreadPoster> pPoster = std::make_shared<ThreadPoster>();
    NodeApi::Get().StartServices(pPoster);
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
                    cout << "NMOS Target: " << pPoster->GetString() << " [" << pPoster->GetSender()->GetId() << "]" << endl;
                    cout << "----------------------------------------" << endl;
                    //getchar();
                    NodeApi::Get().TargetTaken(pPoster->GetPort(), true);
                    break;
                case ThreadPoster::PATCH_SENDER:
                    cout << "----------------------------------------" << endl;
                    cout << "NMOS Patch Sender: " << pPoster->GetString() << endl;
                    cout << "----------------------------------------" << endl;
                    NodeApi::Get().SenderPatched(pPoster->GetPort(), true);
                    break;
                case ThreadPoster::PATCH_RECEIVER:
                    cout << "----------------------------------------" << endl;
                    cout << "NMOS Patch Receiver: " << pPoster->GetString() << endl;
                    cout << "----------------------------------------" << endl;
                    NodeApi::Get().ReceiverPatched(pPoster->GetPort(), true);
                    break;
                case ThreadPoster::ACTIVATE_SENDER:
                    cout << "----------------------------------------" << endl;
                    cout << "NMOS Activate Sender: " << pPoster->GetString() << endl;
                    cout << "----------------------------------------" << endl;
                    break;
                case ThreadPoster::ACTIVATE_RECEIVER:
                    cout << "----------------------------------------" << endl;
                    cout << "NMOS Activate Receiver: " << pPoster->GetString() << endl;
                    cout << "----------------------------------------" << endl;
                    break;
            }

        }
    }
    NodeApi::Get().StopServices();
    return 0;

}
