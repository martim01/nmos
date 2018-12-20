#include <iostream>
#include "json/json.h"
#include <iostream>
#include <fstream>

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

    NodeApi::Get().StartServices(NULL);
    getchar();
    NodeApi::Get().StopServices();
    return 0;

}
