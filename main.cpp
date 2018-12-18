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

using namespace std;



int main()
{
    NodeApi::Get().Init("host1", "http://172.29.80.65:8080/", "host1", "host1");

//    NodeApi::Get().GetSelf().AddVersion("v1.0");
//    NodeApi::Get().GetSelf().AddVersion("v1.1");
    NodeApi::Get().GetSelf().AddApiVersion("v1.2");
    NodeApi::Get().GetSelf().AddEndpoint("172.29.80.65", 8080, false);
    NodeApi::Get().GetSelf().AddEndpoint("172.29.80.65", 443, true);
    NodeApi::Get().GetSelf().AddInternalClock("clk0");
    NodeApi::Get().GetSelf().AddPTPClock("clk1", true, "IEEE1588-2008", "08-00-11-ff-fe-21-e1-b0", true);
    NodeApi::Get().GetSelf().AddInterface("eth0" ,"74-26-96-db-87-31", "74-26-96-db-87-31");
    NodeApi::Get().GetSelf().AddInterface("eth1" ,"74-26-96-db-87-31", "74-26-96-db-87-32");



    Device* pDevice = new Device("TestDevice", "TestDescription", Device::GENERIC,NodeApi::Get().GetSelf().GetId());
    SourceAudio* pSource = new SourceAudio("TestAudio", "TestDescription", pDevice->GetId());
    FlowAudioRaw* pFlow = new FlowAudioRaw("TestFlow", "TestDescription", pSource->GetId(), pDevice->GetId(),48000, FlowAudioRaw::L24);
    Sender* pSender(new Sender("TestSender", "Description", pFlow->GetId(), Sender::RTP_MCAST, pDevice->GetId(), "http://172.29.80.65/by-name/pam.sdp"));
    pSender->AddInterfaceBinding("eth0");


    Receiver* pReceiver = new Receiver("Test Receiver", "TestDescription", Receiver::RTP_MCAST, pDevice->GetId(), Receiver::AUDIO);
    pReceiver->AddCap("audio/L24");
    pReceiver->AddCap("audio/L20");
    pReceiver->AddCap("audio/L16");
    pReceiver->AddInterfaceBinding("eth0");

    NodeApi::Get().GetDevices().AddResource(pDevice);
    NodeApi::Get().GetSources().AddResource(pSource);
    NodeApi::Get().GetFlows().AddResource(pFlow);
    NodeApi::Get().GetReceivers().AddResource(pReceiver);
    NodeApi::Get().GetSenders().AddResource(pSender);
    NodeApi::Get().Commit();

    NodeApi::Get().StartServices(8080, new ServiceBrowser());


    getchar ();


    NodeApi::Get().Register();
    getchar ();

    NodeApi::Get().StopServices();
    return 0;

}
