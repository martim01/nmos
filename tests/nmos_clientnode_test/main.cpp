#include <iostream>
//#include "json/json.h"
#include <iostream>
#include <fstream>
#include "clientapi.h"
#include "self.h"
#include "libnmos.h"
#include "receivernode.h"
#include "sendernode.h"
#include "testposter.h"
#include "threadposter.h"
#include "nodeapi.h"
using namespace std;



int main()
{
    NodeApi::Get().Init(std::make_shared<ThreadPoster>(), 8080, 8081, "clientnode", "host Description");
    NodeApi::Get().GetSelf().AddInternalClock("clk0");
    NodeApi::Get().GetSelf().AddPTPClock("clk1", true, "IEEE1588-2008", "08-00-11-ff-fe-21-e1-b0", true);
    NodeApi::Get().GetSelf().AddInterface("eth0");
    NodeApi::Get().GetSelf().AddTag("location", "W1");
    NodeApi::Get().GetSelf().AddTag("location", "MCR1");
    NodeApi::Get().GetSelf().AddTag("Author", "Matt");

    auto pDevice = make_shared<Device>("clientnode", "TestDescription", Device::GENERIC,NodeApi::Get().GetSelf().GetId());

    auto pSource = make_shared<SourceAudio>("clientnode", "TestDescription", pDevice->GetId());
    pSource->AddChannel("Left", "L");
    pSource->AddChannel("Right", "R");

    auto pFlow = make_shared<FlowAudioRaw>("clientnode", "TestDescription", pSource->GetId(), pDevice->GetId(),48000, FlowAudioRaw::L24);
    pFlow->SetPacketTime(FlowAudioRaw::US_125);
    pFlow->SetMediaClkOffset(129122110);

    auto pSender = make_shared<SenderNode>("clientnode", "Description", pFlow->GetId(), Sender::RTP_MCAST, pDevice->GetId(), "eth0");
    auto pReceiver = make_shared<ReceiverNode>("clientnode", "TestDescription", Receiver::RTP_MCAST, pDevice->GetId(), Receiver::AUDIO);

    pReceiver->AddCap("audio/L24");
    pReceiver->AddCap("audio/L20");
    pReceiver->AddCap("audio/L16");
    pReceiver->AddInterfaceBinding("eth0");

    NodeApi::Get().AddDevice(pDevice);
    NodeApi::Get().AddSource(pSource);
    NodeApi::Get().AddFlow(pFlow);
    NodeApi::Get().AddReceiver(pReceiver);
    NodeApi::Get().AddSender(pSender);
    NodeApi::Get().Commit();

    NodeApi::Get().StartServices();


    ClientApi::Get().SetPoster(make_shared<TestPoster>());

    ClientApi::Get().Start();
    getchar();

    ClientApi::Get().Stop();
    NodeApi::Get().StopServices();
}
