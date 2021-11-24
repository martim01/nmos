#include "testposter.h"
#include "json/json.h"
#include "self.h"
#include "source.h"
#include "device.h"
#include "flow.h"
#include "senderbase.h"
#include "receiverbase.h"
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
void TestPoster::NodeChanged(const pml::nmos::resourcechanges<pml::nmos::Self>& changed)
{
    pmlLog() << "++++++NODES ADDED++++++" ;
    for(auto pResource : changed.lstAdded)
    {
        pmlLog() << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }
    pmlLog() << "++++++++++" << changed.lstAdded.size() << "++++++++++++++" ;

    pmlLog() << "++++++NODES UPDATED++++++" ;
    for(auto pResource :changed.lstUpdated)
    {
        pmlLog() << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }
    pmlLog() << "++++++++++" << changed.lstUpdated.size() << "++++++++++++++" ;
    pmlLog() << "++++++NODES REMOVE++++++" ;
    for(auto pResource : changed.lstRemoved)
    {
        pmlLog() << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }

    pmlLog() << "++++++++++" << changed.lstRemoved.size() << "++++++++++++++" ;
}

void TestPoster::DeviceChanged(const pml::nmos::resourcechanges<pml::nmos::Device>& changed)
{

    pmlLog() << "++++++DEVICES ADDED++++++" ;
    for(auto pResource : changed.lstAdded)
    {
        pmlLog() << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }
    pmlLog() << "++++++++++" << changed.lstAdded.size() << "++++++++++++++" ;

    pmlLog() << "++++++DEVICES UPDATED++++++" ;
    for(auto pResource : changed.lstUpdated)
    {
        pmlLog() << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }
    pmlLog() << "++++++++++" << changed.lstUpdated.size() << "++++++++++++++" ;

    pmlLog() << "++++++DEVICES REMOVE++++++" ;
    for(auto pResource : changed.lstRemoved)
    {
        pmlLog() << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }

    pmlLog() << "++++++++++" << changed.lstRemoved.size() << "++++++++++++++" ;
}

void TestPoster::SourceChanged(const pml::nmos::resourcechanges<pml::nmos::Source>& changed)
{
    pmlLog() << "++++++SOURCES ADDED++++++" ;
    for(auto pResource : changed.lstAdded)
    {
        pmlLog() << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }
    pmlLog() << "++++++++++" << changed.lstAdded.size() << "++++++++++++++" ;

    pmlLog() << "++++++SOURCES UPDATED++++++" ;
    for(auto pResource : changed.lstUpdated)
    {
        pmlLog() << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }
    pmlLog() << "++++++++++" << changed.lstUpdated.size() << "++++++++++++++" ;
    pmlLog() << "++++++SOURCES REMOVE++++++" ;
    for(auto pResource : changed.lstRemoved)
    {
        pmlLog() << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }

    pmlLog() << "++++++++++" << changed.lstRemoved.size() << "++++++++++++++" ;
}

void TestPoster::FlowChanged(const pml::nmos::resourcechanges<pml::nmos::Flow>& changed)
{
    pmlLog() << "++++++FLOWS ADDED++++++" ;
    for(auto pResource : changed.lstAdded)
    {
        pmlLog() << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }
    pmlLog() << "++++++++++" << changed.lstAdded.size() << "++++++++++++++" ;

    pmlLog() << "++++++FLOWS UPDATED++++++" ;
    for(auto pResource : changed.lstUpdated)
    {
        pmlLog() << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }
    pmlLog() << "++++++++++" << changed.lstUpdated.size() << "++++++++++++++" ;

    pmlLog() << "++++++FLOWS REMOVE++++++" ;
    for(auto pResource : changed.lstRemoved)
    {
        pmlLog() << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }

    pmlLog() << "++++++++++" << changed.lstRemoved.size() << "++++++++++++++" ;
}

void TestPoster::SenderChanged(const pml::nmos::resourcechanges<pml::nmos::Sender>& changed)
{
    pmlLog() << "++++++SENDERS ADDED++++++" ;
    for(auto pResource : changed.lstAdded)
    {
        pmlLog() << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }
    pmlLog() << "++++++++++" << changed.lstAdded.size() << "++++++++++++++" ;

    pmlLog() << "++++++SENDERS UPDATED++++++" ;
    for(auto pResource : changed.lstUpdated)
    {
        pmlLog() << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }
    pmlLog() << "++++++++++" << changed.lstUpdated.size() << "++++++++++++++" ;
    pmlLog() << "++++++SENDERS REMOVE++++++" ;
    for(auto pResource : changed.lstRemoved)
    {
        pmlLog() << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }

    pmlLog() << "++++++++++" << changed.lstRemoved.size() << "++++++++++++++" ;
}

void TestPoster::ReceiverChanged(const pml::nmos::resourcechanges<pml::nmos::Receiver>& changed)
{
    pmlLog() << "++++++RECEIVERS ADDED++++++" ;
    for(auto pResource : changed.lstAdded)
    {
        pmlLog() << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }
    pmlLog() << "++++++++++" << changed.lstAdded.size() << "++++++++++++++" ;

    pmlLog() << "++++++RECEIVERS UPDATED++++++" ;
    for(auto pResource : changed.lstUpdated)
    {
        pmlLog() << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }
    pmlLog() << "++++++++++" << changed.lstUpdated.size() << "++++++++++++++" ;
    pmlLog() << "++++++RECEIVERS REMOVE++++++" ;
    for(auto pResource : changed.lstRemoved)
    {
        pmlLog() << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }

    pmlLog() << "++++++++++" << changed.lstRemoved.size() << "++++++++++++++" ;
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
    pmlLog() << "++++++RequestTargetResult++++++" ;
    pmlLog() << "Result = " << nResult;
    pmlLog() << "Response = " << sResponse;
    pmlLog() << "Resource = " << sResourceId;
    pmlLog() << "++++++++++++++++++++++++" ;
}

void TestPoster::RequestPatchSenderResult(unsigned long nResult, const std::string& sResponse, const std::string& sResourceId)
{
    pmlLog() << "++++++RequestPatchSenderResult++++++" ;
    pmlLog() << "Result = " << nResult;
    pmlLog() << "Response = " << sResponse;
    pmlLog() << "Resource = " << sResourceId;
    pmlLog() << "++++++++++++++++++++++++" ;
}

void TestPoster::RequestPatchReceiverResult(unsigned long nResult, const std::string& sResponse, const std::string& sResourceId)
{
    pmlLog() << "++++++RequestPatchReceiverResult++++++" ;
    pmlLog() << "Result = " << nResult;
    pmlLog() << "Response = " << sResponse;
    pmlLog() << "Resource = " << sResourceId;
    pmlLog() << "++++++++++++++++++++++++" ;
}

void TestPoster::RequestConnectResult(const std::string& sSenderId, const std::string& sReceiverId, bool bSuccess, const std::string& sResponse)
{
    pmlLog() << "++++++RequestConnectResult++++++" ;
    pmlLog() << "Sender = " << sSenderId;
    pmlLog() << "Receiver = " << sReceiverId;
    pmlLog() << "Result = " << bSuccess;
    pmlLog() << "Response = " << sResponse;
    pmlLog() << "++++++++++++++++++++++++" ;
}

void TestPoster::RequestGetSenderStagedResult(unsigned long nResult, const std::string& sResponse, const std::string& sResourceId)
{
    pmlLog() << "++++++RequestGetSenderStagedResult++++++" ;
    pmlLog() << "Result = " << nResult;
    pmlLog() << "Response = " << sResponse;
    pmlLog() << "Resource = " << sResourceId;
    pmlLog() << "++++++++++++++++++++++++" ;
}

void TestPoster::RequestGetSenderActiveResult(unsigned long nResult, const std::string& sResponse, const std::string& sResourceId)
{
    pmlLog() << "++++++RequestGetSenderActiveResult++++++" ;
    pmlLog() << "Result = " << nResult;
    pmlLog() << "Response = " << sResponse;
    pmlLog() << "Resource = " << sResourceId;
    pmlLog() << "++++++++++++++++++++++++" ;
}

void TestPoster::RequestGetSenderTransportFileResult(unsigned long nResult, const std::string& sResponse, const std::string& sResourceId)
{
    pmlLog() << "++++++RequestGetSenderTransportFileResult++++++" ;
    pmlLog() << "Result = " << nResult;
    pmlLog() << "Response = " << sResponse;
    pmlLog() << "Resource = " << sResourceId;
    pmlLog() << "++++++++++++++++++++++++" ;
}

void TestPoster::RequestGetReceiverStagedResult(unsigned long nResult, const std::string& sResponse, const std::string& sResourceId)
{
    pmlLog() << "++++++RequestGetReceiverStagedResult++++++" ;
    pmlLog() << "Result = " << nResult;
    pmlLog() << "Response = " << sResponse;
    pmlLog() << "Resource = " << sResourceId;
    pmlLog() << "++++++++++++++++++++++++" ;
}

void TestPoster::RequestGetReceiverActiveResult(unsigned long nResult, const std::string& sResponse, const std::string& sResourceId)
{
    pmlLog() << "++++++RequestGetReceiverActiveResult++++++" ;
    pmlLog() << "Result = " << nResult;
    pmlLog() << "Response = " << sResponse;
    pmlLog() << "Resource = " << sResourceId;
    pmlLog() << "++++++++++++++++++++++++" ;
}

