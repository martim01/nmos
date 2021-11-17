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

void TestPoster::ModeChanged(bool bQueryApi)
{
    if(bQueryApi)
    {
        cout << "++++++QUERY MODE++++++" << endl;
    }
    else
    {
        cout << "++++++P2P MODE++++++" << endl;
    }
}
void TestPoster::NodeChanged(const std::list<std::shared_ptr<pml::nmos::Self> >& lstNodesAdded, const std::list<std::shared_ptr<pml::nmos::Self> >& lstNodesUpdated, const std::list<std::shared_ptr<pml::nmos::Self> >& lstNodesRemoved)
{
    cout << "++++++NODES ADDED++++++" << endl;
    for(auto pResource : lstNodesAdded)
    {
        cout << pResource->GetJson(pml::nmos::ApiVersion(1,2)) << endl;
    }
    cout << "++++++++++++++++++++++++" << endl;

    cout << "++++++NODES UPDATED++++++" << endl;
    for(auto pResource :lstNodesUpdated)
    {
        cout << pResource->GetJson(pml::nmos::ApiVersion(1,2)) << endl;
    }

    cout << "++++++NODES REMOVE++++++" << endl;
    for(auto pResource : lstNodesRemoved)
    {
        cout << pResource->GetJson(pml::nmos::ApiVersion(1,2)) << endl;
    }

    cout << "++++++++++++++++++++++++" << endl;
}

void TestPoster::DeviceChanged(const std::list<std::shared_ptr<pml::nmos::Device> >& lstDevicesAdded, const std::list<std::shared_ptr<pml::nmos::Device> >& lstDevicesUpdated, const std::list<std::shared_ptr<pml::nmos::Device> >& lstDevicesRemoved)
{

    cout << "++++++DEVICES ADDED++++++" << endl;
    for(auto pResource : lstDevicesAdded)
    {
        cout << pResource->GetJson(pml::nmos::ApiVersion(1,2)) << endl;
    }
    cout << "++++++++++++++++++++++++" << endl;

    cout << "++++++DEVICES UPDATED++++++" << endl;
    for(auto pResource : lstDevicesUpdated)
    {
        cout << pResource->GetJson(pml::nmos::ApiVersion(1,2)) << endl;
    }

    cout << "++++++DEVICES REMOVE++++++" << endl;
    for(auto pResource : lstDevicesRemoved)
    {
        cout << pResource->GetJson(pml::nmos::ApiVersion(1,2)) << endl;
    }

    cout << "++++++++++++++++++++++++" << endl;
}

void TestPoster::SourceChanged(const std::list<std::shared_ptr<pml::nmos::Source> >& lstSourcesAdded, const std::list<std::shared_ptr<pml::nmos::Source> >& lstSourcesUpdated, const std::list<std::shared_ptr<pml::nmos::Source> >& lstSourcesRemoved)
{
    cout << "++++++SOURCES ADDED++++++" << endl;
    for(auto pResource : lstSourcesAdded)
    {
        cout << pResource->GetJson(pml::nmos::ApiVersion(1,2)) << endl;
    }
    cout << "++++++++++++++++++++++++" << endl;

    cout << "++++++SOURCES UPDATED++++++" << endl;
    for(auto pResource : lstSourcesUpdated)
    {
        cout << pResource->GetJson(pml::nmos::ApiVersion(1,2)) << endl;
    }

    cout << "++++++SOURCES REMOVE++++++" << endl;
    for(auto pResource : lstSourcesRemoved)
    {
        cout << pResource->GetJson(pml::nmos::ApiVersion(1,2)) << endl;
    }

    cout << "++++++++++++++++++++++++" << endl;
}

void TestPoster::FlowChanged(const std::list<std::shared_ptr<pml::nmos::Flow> >& lstFlowsAdded, const std::list<std::shared_ptr<pml::nmos::Flow> >& lstFlowsUpdated, const std::list<std::shared_ptr<pml::nmos::Flow> >& lstFlowsRemoved)
{
    cout << "++++++FLOWS ADDED++++++" << endl;
    for(auto pResource : lstFlowsAdded)
    {
        cout << pResource->GetJson(pml::nmos::ApiVersion(1,2)) << endl;
    }
    cout << "++++++++++++++++++++++++" << endl;

    cout << "++++++FLOWS UPDATED++++++" << endl;
    for(auto pResource : lstFlowsUpdated)
    {
        cout << pResource->GetJson(pml::nmos::ApiVersion(1,2)) << endl;
    }

    cout << "++++++FLOWS REMOVE++++++" << endl;
    for(auto pResource : lstFlowsRemoved)
    {
        cout << pResource->GetJson(pml::nmos::ApiVersion(1,2)) << endl;
    }

    cout << "++++++++++++++++++++++++" << endl;
}

void TestPoster::SenderChanged(const std::list<std::shared_ptr<pml::nmos::SenderBase> >& lstSendersAdded, const std::list<std::shared_ptr<pml::nmos::SenderBase> >& lstSendersUpdated, const std::list<std::shared_ptr<pml::nmos::SenderBase> >& lstSendersRemoved)
{
    cout << "++++++SENDERS ADDED++++++" << endl;
    for(auto pResource : lstSendersAdded)
    {
        cout << pResource->GetJson(pml::nmos::ApiVersion(1,2)) << endl;
    }
    cout << "++++++++++++++++++++++++" << endl;

    cout << "++++++SENDERS UPDATED++++++" << endl;
    for(auto pResource : lstSendersUpdated)
    {
        cout << pResource->GetJson(pml::nmos::ApiVersion(1,2)) << endl;
    }

    cout << "++++++SENDERS REMOVE++++++" << endl;
    for(auto pResource : lstSendersRemoved)
    {
        cout << pResource->GetJson(pml::nmos::ApiVersion(1,2)) << endl;
    }

    cout << "++++++++++++++++++++++++" << endl;
}

void TestPoster::ReceiverChanged(const std::list<std::shared_ptr<pml::nmos::ReceiverBase> >& lstReceiversAdded, const std::list<std::shared_ptr<pml::nmos::ReceiverBase> >& lstReceiversUpdated, const std::list<std::shared_ptr<pml::nmos::ReceiverBase> >& lstReceiversRemoved)
{
    cout << "++++++RECEIVERS ADDED++++++" << endl;
    for(auto pResource : lstReceiversAdded)
    {
        cout << pResource->GetJson(pml::nmos::ApiVersion(1,2)) << endl;
    }
    cout << "++++++++++++++++++++++++" << endl;

    cout << "++++++RECEIVERS UPDATED++++++" << endl;
    for(auto pResource : lstReceiversUpdated)
    {
        cout << pResource->GetJson(pml::nmos::ApiVersion(1,2)) << endl;
    }

    cout << "++++++RECEIVERS REMOVE++++++" << endl;
    for(auto pResource : lstReceiversRemoved)
    {
        cout << pResource->GetJson(pml::nmos::ApiVersion(1,2)) << endl;
    }

    cout << "++++++++++++++++++++++++" << endl;
}

void TestPoster::QueryServerChanged(const std::string& sUrl)
{
    cout << "++++++QUERY SERVER CHANGED ++++++" << endl;
    cout << sUrl;
    cout << "++++++++++++++++++++++++" << endl;
}

void TestPoster::QueryServerFound(const std::string& sUrl, unsigned short nPriority)
{
    cout << "++++++QUERY SERVER FOUND ++++++" << endl;
    cout << sUrl << ": " << nPriority;
    cout << "++++++++++++++++++++++++" << endl;
}

void TestPoster::QueryServerRemoved(const std::string& sUrl)
{
    cout << "++++++QUERY SERVER REMOVED ++++++" << endl;
    cout << sUrl;
    cout << "++++++++++++++++++++++++" << endl;
}


void TestPoster::QuerySubscription(const std::string& sSubscriptionId, int nResource, const std::string& sQuery)
{

}

void TestPoster::QuerySubscriptionRemoved(const std::string& sSubscriptionId)
{

}

void TestPoster::RequestTargetResult(unsigned long nResult, const std::string& sResponse, const std::string& sResourceId)
{

}

void TestPoster::RequestPatchSenderResult(unsigned long nResult, const std::string& sResponse, const std::string& sResourceId)
{

}

void TestPoster::RequestPatchReceiverResult(unsigned long nResult, const std::string& sResponse, const std::string& sResourceId)
{

}

void TestPoster::RequestConnectResult(const std::string& sSenderId, const std::string& sReceiverId, bool bSuccess, const std::string& sResponse)
{

}

void TestPoster::RequestGetSenderStagedResult(unsigned long nResult, const std::string& sResponse, const std::string& sResourceId)
{

}

void TestPoster::RequestGetSenderActiveResult(unsigned long nResult, const std::string& sResponse, const std::string& sResourceId)
{

}

void TestPoster::RequestGetSenderTransportFileResult(unsigned long nResult, const std::string& sResponse, const std::string& sResourceId)
{

}

void TestPoster::RequestGetReceiverStagedResult(unsigned long nResult, const std::string& sResponse, const std::string& sResourceId)
{

}

void TestPoster::RequestGetReceiverActiveResult(unsigned long nResult, const std::string& sResponse, const std::string& sResourceId)
{

}

