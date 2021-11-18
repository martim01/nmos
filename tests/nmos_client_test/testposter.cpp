#include "testposter.h"
#include "json/json.h"
#include "self.h"
#include "source.h"
#include "device.h"
#include "flow.h"
#include "sender.h"
#include "receiver.h"
#include <iostream>
#include "log.h"

using namespace std;

void TestPoster::ModeChanged(bool bQueryApi)
{
    if(bQueryApi)
    {
        pmlLog() << "++++++QUERY MODE++++++" ;
    }
    else
    {
        pmlLog() << "++++++P2P MODE++++++" ;
    }
}
void TestPoster::NodeChanged(const std::list<std::shared_ptr<pml::nmos::Self> >& lstNodesAdded, const std::list<std::shared_ptr<pml::nmos::Self> >& lstNodesUpdated, const std::list<std::shared_ptr<pml::nmos::Self> >& lstNodesRemoved)
{
    pmlLog() << "++++++NODES ADDED++++++" ;
    for(auto pResource : lstNodesAdded)
    {
        pmlLog() << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }
    pmlLog() << "++++++++++" << lstNodesAdded.size() << "++++++++++++++" ;

    pmlLog() << "++++++NODES UPDATED++++++" ;
    for(auto pResource :lstNodesUpdated)
    {
        pmlLog() << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }
    pmlLog() << "++++++++++" << lstNodesUpdated.size() << "++++++++++++++" ;
    pmlLog() << "++++++NODES REMOVE++++++" ;
    for(auto pResource : lstNodesRemoved)
    {
        pmlLog() << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }

    pmlLog() << "++++++++++" << lstNodesRemoved.size() << "++++++++++++++" ;
}

void TestPoster::DeviceChanged(const std::list<std::shared_ptr<pml::nmos::Device> >& lstDevicesAdded, const std::list<std::shared_ptr<pml::nmos::Device> >& lstDevicesUpdated, const std::list<std::shared_ptr<pml::nmos::Device> >& lstDevicesRemoved)
{

    pmlLog() << "++++++DEVICES ADDED++++++" ;
    for(auto pResource : lstDevicesAdded)
    {
        pmlLog() << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }
    pmlLog() << "++++++++++" << lstDevicesAdded.size() << "++++++++++++++" ;

    pmlLog() << "++++++DEVICES UPDATED++++++" ;
    for(auto pResource : lstDevicesUpdated)
    {
        pmlLog() << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }
    pmlLog() << "++++++++++" << lstDevicesUpdated.size() << "++++++++++++++" ;

    pmlLog() << "++++++DEVICES REMOVE++++++" ;
    for(auto pResource : lstDevicesRemoved)
    {
        pmlLog() << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }

    pmlLog() << "++++++++++" << lstDevicesRemoved.size() << "++++++++++++++" ;
}

void TestPoster::SourceChanged(const std::list<std::shared_ptr<pml::nmos::Source> >& lstSourcesAdded, const std::list<std::shared_ptr<pml::nmos::Source> >& lstSourcesUpdated, const std::list<std::shared_ptr<pml::nmos::Source> >& lstSourcesRemoved)
{
    pmlLog() << "++++++SOURCES ADDED++++++" ;
    for(auto pResource : lstSourcesAdded)
    {
        pmlLog() << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }
    pmlLog() << "++++++++++" << lstSourcesAdded.size() << "++++++++++++++" ;

    pmlLog() << "++++++SOURCES UPDATED++++++" ;
    for(auto pResource : lstSourcesUpdated)
    {
        pmlLog() << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }
    pmlLog() << "++++++++++" << lstSourcesUpdated.size() << "++++++++++++++" ;
    pmlLog() << "++++++SOURCES REMOVE++++++" ;
    for(auto pResource : lstSourcesRemoved)
    {
        pmlLog() << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }

    pmlLog() << "++++++++++" << lstSourcesRemoved.size() << "++++++++++++++" ;
}

void TestPoster::FlowChanged(const std::list<std::shared_ptr<pml::nmos::Flow> >& lstFlowsAdded, const std::list<std::shared_ptr<pml::nmos::Flow> >& lstFlowsUpdated, const std::list<std::shared_ptr<pml::nmos::Flow> >& lstFlowsRemoved)
{
    pmlLog() << "++++++FLOWS ADDED++++++" ;
    for(auto pResource : lstFlowsAdded)
    {
        pmlLog() << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }
    pmlLog() << "++++++++++" << lstFlowsAdded.size() << "++++++++++++++" ;

    pmlLog() << "++++++FLOWS UPDATED++++++" ;
    for(auto pResource : lstFlowsUpdated)
    {
        pmlLog() << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }
    pmlLog() << "++++++++++" << lstFlowsUpdated.size() << "++++++++++++++" ;

    pmlLog() << "++++++FLOWS REMOVE++++++" ;
    for(auto pResource : lstFlowsRemoved)
    {
        pmlLog() << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }

    pmlLog() << "++++++++++" << lstFlowsRemoved.size() << "++++++++++++++" ;
}

void TestPoster::SenderChanged(const std::list<std::shared_ptr<pml::nmos::SenderBase> >& lstSendersAdded, const std::list<std::shared_ptr<pml::nmos::SenderBase> >& lstSendersUpdated, const std::list<std::shared_ptr<pml::nmos::SenderBase> >& lstSendersRemoved)
{
    pmlLog() << "++++++SENDERS ADDED++++++" ;
    for(auto pResource : lstSendersAdded)
    {
        pmlLog() << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }
    pmlLog() << "++++++++++" << lstSendersAdded.size() << "++++++++++++++" ;

    pmlLog() << "++++++SENDERS UPDATED++++++" ;
    for(auto pResource : lstSendersUpdated)
    {
        pmlLog() << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }
    pmlLog() << "++++++++++" << lstSendersUpdated.size() << "++++++++++++++" ;
    pmlLog() << "++++++SENDERS REMOVE++++++" ;
    for(auto pResource : lstSendersRemoved)
    {
        pmlLog() << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }

    pmlLog() << "++++++++++" << lstSendersRemoved.size() << "++++++++++++++" ;
}

void TestPoster::ReceiverChanged(const std::list<std::shared_ptr<pml::nmos::ReceiverBase> >& lstReceiversAdded, const std::list<std::shared_ptr<pml::nmos::ReceiverBase> >& lstReceiversUpdated, const std::list<std::shared_ptr<pml::nmos::ReceiverBase> >& lstReceiversRemoved)
{
    pmlLog() << "++++++RECEIVERS ADDED++++++" ;
    for(auto pResource : lstReceiversAdded)
    {
        pmlLog() << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }
    pmlLog() << "++++++++++" << lstReceiversAdded.size() << "++++++++++++++" ;

    pmlLog() << "++++++RECEIVERS UPDATED++++++" ;
    for(auto pResource : lstReceiversUpdated)
    {
        pmlLog() << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }
    pmlLog() << "++++++++++" << lstReceiversUpdated.size() << "++++++++++++++" ;
    pmlLog() << "++++++RECEIVERS REMOVE++++++" ;
    for(auto pResource : lstReceiversRemoved)
    {
        pmlLog() << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }

    pmlLog() << "++++++++++" << lstReceiversRemoved.size() << "++++++++++++++" ;
}

void TestPoster::QueryServerChanged(const std::string& sUrl)
{
    pmlLog() << "++++++QUERY SERVER CHANGED ++++++" ;
    pmlLog() << sUrl;
    pmlLog() << "++++++++++++++++++++++++" ;
}

void TestPoster::QueryServerFound(const std::string& sUrl, unsigned short nPriority)
{
    pmlLog() << "++++++QUERY SERVER FOUND ++++++" ;
    pmlLog() << sUrl << ": " << nPriority;
    pmlLog() << "++++++++++++++++++++++++" ;
}

void TestPoster::QueryServerRemoved(const std::string& sUrl)
{
    pmlLog() << "++++++QUERY SERVER REMOVED ++++++" ;
    pmlLog() << sUrl;
    pmlLog() << "++++++++++++++++++++++++" ;
}


void TestPoster::QuerySubscription(const std::string& sSubscriptionId, int nResource, const std::string& sQuery)
{
    pmlLog() << "++++++QUERY SUBSCRIPTION++++++" ;
    pmlLog() << sSubscriptionId;
    pmlLog() << nResource;
    pmlLog() << sQuery;
    pmlLog() << "++++++++++++++++++++++++" ;
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

