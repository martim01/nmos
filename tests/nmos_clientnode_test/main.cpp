#include <iostream>
//#include "json/json.h"
#include <iostream>
#include <fstream>
#include "log.h"
#include "clientapi.h"
#include "self.h"
#include "libnmos.h"
#include "receiver.h"
#include "testposter.h"
#include "threadposter.h"
#include "nodeapi.h"
using namespace std;



int main()
{
    NodeApi::Get().Init(8080, 8081, "clientnode", "host Description");
    NodeApi::Get().GetSelf().AddInternalClock("clk0");
    NodeApi::Get().GetSelf().AddPTPClock("clk1", true, "IEEE1588-2008", "08-00-11-ff-fe-21-e1-b0", true);
    NodeApi::Get().GetSelf().AddInterface("eth0");
    NodeApi::Get().GetSelf().AddTag("location", "W1");
    NodeApi::Get().GetSelf().AddTag("location", "MCR1");
    NodeApi::Get().GetSelf().AddTag("Author", "Matt");

    shared_ptr<Device> pDevice = make_shared<Device>("clientnode", "TestDescription", Device::GENERIC,NodeApi::Get().GetSelf().GetId());

    shared_ptr<SourceAudio> pSource = make_shared<SourceAudio>("clientnode", "TestDescription", pDevice->GetId());
    pSource->AddChannel("Left", "L");
    pSource->AddChannel("Right", "R");

    shared_ptr<FlowAudioRaw> pFlow = make_shared<FlowAudioRaw>("clientnode", "TestDescription", pSource->GetId(), pDevice->GetId(),48000, FlowAudioRaw::L24);
    pFlow->SetPacketTime(FlowAudioRaw::US_125);
    pFlow->SetMediaClkOffset(129122110);

    shared_ptr<Sender> pSender = make_shared<Sender>("clientnode", "Description", pFlow->GetId(), Sender::RTP_MCAST, pDevice->GetId(), "eth0");
    shared_ptr<Receiver> pReceiver = make_shared<Receiver>("clientnode", "TestDescription", Receiver::RTP_MCAST, pDevice->GetId(), Receiver::AUDIO);

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
    std::shared_ptr<ThreadPoster> pPoster = std::make_shared<ThreadPoster>();
    NodeApi::Get().StartServices(pPoster);


    ClientApi::Get().SetPoster(make_shared<TestPoster>());

    ClientApi::Get().Start();
    getchar();

    ClientApi::Get().Stop();
    NodeApi::Get().StopServices();
}
