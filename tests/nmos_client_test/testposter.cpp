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
void TestPoster::NodeChanged(const std::list<std::shared_ptr<Self> >& lstNodesAdded, const std::list<std::shared_ptr<Self> >& lstNodesUpdated, const std::list<std::shared_ptr<Self> >& lstNodesRemoved)
{
    Json::StyledWriter sw;

    cout << "++++++NODES ADDED++++++" << endl;
    for(std::list<std::shared_ptr<Self> >::const_iterator itResource = lstNodesAdded.begin(); itResource != lstNodesAdded.end(); ++itResource)
    {
        cout << sw.write((*itResource)->GetJson(ApiVersion(1,2))) << endl;
    }
    cout << "++++++++++++++++++++++++" << endl;

    cout << "++++++NODES UPDATED++++++" << endl;
    for(std::list<std::shared_ptr<Self> >::const_iterator itResource = lstNodesUpdated.begin(); itResource != lstNodesUpdated.end(); ++itResource)
    {
        cout << sw.write((*itResource)->GetJson(ApiVersion(1,2))) << endl;
    }

    cout << "++++++NODES REMOVE++++++" << endl;
    for(std::list<std::shared_ptr<Self> >::const_iterator itResource = lstNodesRemoved.begin(); itResource != lstNodesRemoved.end(); ++itResource)
    {
        cout << sw.write((*itResource)->GetJson(ApiVersion(1,2))) << endl;
    }

    cout << "++++++++++++++++++++++++" << endl;
}

void TestPoster::DeviceChanged(const std::list<std::shared_ptr<Device> >& lstDevicesAdded, const std::list<std::shared_ptr<Device> >& lstDevicesUpdated, const std::list<std::shared_ptr<Device> >& lstDevicesRemoved)
{
    Json::StyledWriter sw;

    cout << "++++++DEVICES ADDED++++++" << endl;
    for(std::list<std::shared_ptr<Device> >::const_iterator itResource = lstDevicesAdded.begin(); itResource != lstDevicesAdded.end(); ++itResource)
    {
        cout << sw.write((*itResource)->GetJson(ApiVersion(1,2))) << endl;
    }
    cout << "++++++++++++++++++++++++" << endl;

    cout << "++++++DEVICES UPDATED++++++" << endl;
    for(std::list<std::shared_ptr<Device> >::const_iterator itResource = lstDevicesUpdated.begin(); itResource != lstDevicesUpdated.end(); ++itResource)
    {
        cout << sw.write((*itResource)->GetJson(ApiVersion(1,2))) << endl;
    }

    cout << "++++++DEVICES REMOVE++++++" << endl;
    for(std::list<std::shared_ptr<Device> >::const_iterator itResource = lstDevicesRemoved.begin(); itResource != lstDevicesRemoved.end(); ++itResource)
    {
        cout << sw.write((*itResource)->GetJson(ApiVersion(1,2))) << endl;
    }

    cout << "++++++++++++++++++++++++" << endl;
}

void TestPoster::SourceChanged(const std::list<std::shared_ptr<Source> >& lstSourcesAdded, const std::list<std::shared_ptr<Source> >& lstSourcesUpdated, const std::list<std::shared_ptr<Source> >& lstSourcesRemoved)
{
    Json::StyledWriter sw;

    cout << "++++++SOURCES ADDED++++++" << endl;
    for(std::list<std::shared_ptr<Source> >::const_iterator itResource = lstSourcesAdded.begin(); itResource != lstSourcesAdded.end(); ++itResource)
    {
        cout << sw.write((*itResource)->GetJson(ApiVersion(1,2))) << endl;
    }
    cout << "++++++++++++++++++++++++" << endl;

    cout << "++++++SOURCES UPDATED++++++" << endl;
    for(std::list<std::shared_ptr<Source> >::const_iterator itResource = lstSourcesUpdated.begin(); itResource != lstSourcesUpdated.end(); ++itResource)
    {
        cout << sw.write((*itResource)->GetJson(ApiVersion(1,2))) << endl;
    }

    cout << "++++++SOURCES REMOVE++++++" << endl;
    for(std::list<std::shared_ptr<Source> >::const_iterator itResource = lstSourcesRemoved.begin(); itResource != lstSourcesRemoved.end(); ++itResource)
    {
        cout << sw.write((*itResource)->GetJson(ApiVersion(1,2))) << endl;
    }

    cout << "++++++++++++++++++++++++" << endl;
}

void TestPoster::FlowChanged(const std::list<std::shared_ptr<Flow> >& lstFlowsAdded, const std::list<std::shared_ptr<Flow> >& lstFlowsUpdated, const std::list<std::shared_ptr<Flow> >& lstFlowsRemoved)
{
    Json::StyledWriter sw;

    cout << "++++++FLOWS ADDED++++++" << endl;
    for(std::list<std::shared_ptr<Flow> >::const_iterator itResource = lstFlowsAdded.begin(); itResource != lstFlowsAdded.end(); ++itResource)
    {
        cout << sw.write((*itResource)->GetJson(ApiVersion(1,2))) << endl;
    }
    cout << "++++++++++++++++++++++++" << endl;

    cout << "++++++FLOWS UPDATED++++++" << endl;
    for(std::list<std::shared_ptr<Flow> >::const_iterator itResource = lstFlowsUpdated.begin(); itResource != lstFlowsUpdated.end(); ++itResource)
    {
        cout << sw.write((*itResource)->GetJson(ApiVersion(1,2))) << endl;
    }

    cout << "++++++FLOWS REMOVE++++++" << endl;
    for(std::list<std::shared_ptr<Flow> >::const_iterator itResource = lstFlowsRemoved.begin(); itResource != lstFlowsRemoved.end(); ++itResource)
    {
        cout << sw.write((*itResource)->GetJson(ApiVersion(1,2))) << endl;
    }

    cout << "++++++++++++++++++++++++" << endl;
}

void TestPoster::SenderChanged(const std::list<std::shared_ptr<Sender> >& lstSendersAdded, const std::list<std::shared_ptr<Sender> >& lstSendersUpdated, const std::list<std::shared_ptr<Sender> >& lstSendersRemoved)
{
    Json::StyledWriter sw;

    cout << "++++++SENDERS ADDED++++++" << endl;
    for(std::list<std::shared_ptr<Sender> >::const_iterator itResource = lstSendersAdded.begin(); itResource != lstSendersAdded.end(); ++itResource)
    {
        cout << sw.write((*itResource)->GetJson(ApiVersion(1,2))) << endl;
    }
    cout << "++++++++++++++++++++++++" << endl;

    cout << "++++++SENDERS UPDATED++++++" << endl;
    for(std::list<std::shared_ptr<Sender> >::const_iterator itResource = lstSendersUpdated.begin(); itResource != lstSendersUpdated.end(); ++itResource)
    {
        cout << sw.write((*itResource)->GetJson(ApiVersion(1,2))) << endl;
    }

    cout << "++++++SENDERS REMOVE++++++" << endl;
    for(std::list<std::shared_ptr<Sender> >::const_iterator itResource = lstSendersRemoved.begin(); itResource != lstSendersRemoved.end(); ++itResource)
    {
        cout << sw.write((*itResource)->GetJson(ApiVersion(1,2))) << endl;
    }

    cout << "++++++++++++++++++++++++" << endl;
}

void TestPoster::ReceiverChanged(const std::list<std::shared_ptr<Receiver> >& lstReceiversAdded, const std::list<std::shared_ptr<Receiver> >& lstReceiversUpdated, const std::list<std::shared_ptr<Receiver> >& lstReceiversRemoved)
{
    Json::StyledWriter sw;

    cout << "++++++RECEIVERS ADDED++++++" << endl;
    for(std::list<std::shared_ptr<Receiver> >::const_iterator itResource = lstReceiversAdded.begin(); itResource != lstReceiversAdded.end(); ++itResource)
    {
        cout << sw.write((*itResource)->GetJson(ApiVersion(1,2))) << endl;
    }
    cout << "++++++++++++++++++++++++" << endl;

    cout << "++++++RECEIVERS UPDATED++++++" << endl;
    for(std::list<std::shared_ptr<Receiver> >::const_iterator itResource = lstReceiversUpdated.begin(); itResource != lstReceiversUpdated.end(); ++itResource)
    {
        cout << sw.write((*itResource)->GetJson(ApiVersion(1,2))) << endl;
    }

    cout << "++++++RECEIVERS REMOVE++++++" << endl;
    for(std::list<std::shared_ptr<Receiver> >::const_iterator itResource = lstReceiversRemoved.begin(); itResource != lstReceiversRemoved.end(); ++itResource)
    {
        cout << sw.write((*itResource)->GetJson(ApiVersion(1,2))) << endl;
    }

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

