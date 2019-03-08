#include "testposter.h"
#include "json/json.h"
#include "self.h"
#include "source.h"
#include "device.h"
#include "flow.h"
#include "sender.h"
#include "receiver.h"
#include <iostream>
using namespace std;

void TestPoster::NodeChanged(std::shared_ptr<Self> pNode, enumChange eChange)
{
    Json::FastWriter sw;

    cout << "++++++NODE CHANGED++++++" << endl;
    cout << sw.write(pNode->GetJson(ApiVersion(1,2))) << endl;
    cout << "++++++++++++++++++++++++" << endl;
}

void TestPoster::DeviceChanged(std::shared_ptr<Device> pDevice, enumChange eChange)
{
    Json::FastWriter sw;

    cout << "++++DEVICE CHANGED++++++" << endl;
    cout << sw.write(pDevice->GetJson(ApiVersion(1,2))) << endl;
    cout << "++++++++++++++++++++++++" << endl;
}

void TestPoster::SourceChanged(std::shared_ptr<Source> pSource, enumChange eChange)
{
    Json::FastWriter sw;

    cout << "++++SOURCE CHANGED++++++" << endl;
    cout << sw.write(pSource->GetJson(ApiVersion(1,2))) << endl;
    cout << "++++++++++++++++++++++++" << endl;
}

void TestPoster::FlowChanged(std::shared_ptr<Flow> pFlow, enumChange eChange)
{
    Json::FastWriter sw;

    cout << "++++++FLOW CHANGED++++++" << endl;
    cout << sw.write(pFlow->GetJson(ApiVersion(1,2))) << endl;
    cout << "++++++++++++++++++++++++" << endl;
}

void TestPoster::SenderChanged(std::shared_ptr<Sender> pSender, enumChange eChange)
{
    Json::FastWriter sw;

    cout << "++++SENDER CHANGED++++++" << endl;
    cout << sw.write(pSender->GetJson(ApiVersion(1,2))) << endl;
    cout << "++++++++++++++++++++++++" << endl;
}

void TestPoster::ReceiverChanged(std::shared_ptr<Receiver> pReceiver, enumChange eChange)
{
    Json::FastWriter sw;

    cout << "++RECEIVER CHANGED++++++" << endl;
    cout << sw.write(pReceiver->GetJson(ApiVersion(1,2))) << endl;
    cout << "++++++++++++++++++++++++" << endl;
}

void TestPoster::NodesRemoved(const std::set<std::string>& setRemoved)
{

}

void TestPoster::DevicesRemoved(const std::set<std::string>& setRemoved)
{

}

void TestPoster::SourcesRemoved(const std::set<std::string>& setRemoved)
{

}

void TestPoster::FlowsRemoved(const std::set<std::string>& setRemoved)
{

}

void TestPoster::SendersRemoved(const std::set<std::string>& setRemoved)
{

}

void TestPoster::ReceiversRemoved(const std::set<std::string>& setRemoved)
{

}
