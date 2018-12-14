#include <iostream>
#include "json/json.h"
#include <iostream>
#include <fstream>

#include "node.h"
#include "sourceaudio.h"
#include "flowaudioraw.h"
#include "device.h"
#include "sender.h"
#include "receiver.h"

using namespace std;



int main()
{
    Node::Get().Init("host1", "http://172.29.80.65:8080/", "host1", "host1");

//    Node::Get().GetSelf().AddVersion("v1.0");
//    Node::Get().GetSelf().AddVersion("v1.1");
    Node::Get().GetSelf().AddApiVersion("v1.2");
    Node::Get().GetSelf().AddEndpoint("172.29.80.65", 8080, false);
    Node::Get().GetSelf().AddEndpoint("172.29.80.65", 443, true);
    Node::Get().GetSelf().AddInternalClock("clk0");
    Node::Get().GetSelf().AddPTPClock("clk1", true, "IEEE1588-2008", "08-00-11-ff-fe-21-e1-b0", true);
    Node::Get().GetSelf().AddInterface("eth0" ,"74-26-96-db-87-31", "74-26-96-db-87-31");
    Node::Get().GetSelf().AddInterface("eth1" ,"74-26-96-db-87-31", "74-26-96-db-87-32");



    Device* pDevice = new Device("TestDevice", "TestDescription", Device::GENERIC,Node::Get().GetSelf().GetId());
    SourceAudio* pSource = new SourceAudio("TestAudio", "TestDescription", pDevice->GetId());
    FlowAudioRaw* pFlow = new FlowAudioRaw("TestFlow", "TestDescription", pSource->GetId(), pDevice->GetId(),48000, FlowAudioRaw::L24);
    Sender* pSender(new Sender("TestSender", "Description", pFlow->GetId(), Sender::RTP_MCAST, pDevice->GetId(), "http://172.29.80.65/by-name/pam.sdp"));
    pSender->AddInterfaceBinding("eth0");


    Receiver* pReceiver = new Receiver("Test Receiver", "TestDescription", Receiver::RTP_MCAST, pDevice->GetId(), Receiver::AUDIO);
    pReceiver->AddCap("audio/L24");
    pReceiver->AddCap("audio/L20");
    pReceiver->AddCap("audio/L16");
    pReceiver->AddInterfaceBinding("eth0");

    Node::Get().GetDevices().AddResource(pDevice);
    Node::Get().GetSources().AddResource(pSource);
    Node::Get().GetFlows().AddResource(pFlow);
    Node::Get().GetReceivers().AddResource(pReceiver);
    Node::Get().GetSenders().AddResource(pSender);
    Node::Get().Commit();

    Node::Get().StartServices(8080);


    getchar ();

    Node::Get().StopServices();
    return 0;

}
