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
using namespace pml::nmos;

void TestPoster::ModeChanged(bool bQueryApi)
{
    if(bQueryApi)
    {
        pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++QUERY MODE++++++" ;
    }
    else
    {
        pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++P2P MODE++++++" ;
    }
}
void TestPoster::NodeChanged(const pml::nmos::resourcechanges<pml::nmos::Self>& changed)
{
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++NODES ADDED++++++" ;
    for(auto pResource : changed.lstAdded)
    {
        pml::log::log(pml::log::Level::kInfo, "pml::nmos") << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++++++" << changed.lstAdded.size() << "++++++++++++++" ;

    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++NODES UPDATED++++++" ;
    for(auto pResource :changed.lstUpdated)
    {
        pml::log::log(pml::log::Level::kInfo, "pml::nmos") << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++++++" << changed.lstUpdated.size() << "++++++++++++++" ;
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++NODES REMOVE++++++" ;
    for(auto pResource : changed.lstRemoved)
    {
        pml::log::log(pml::log::Level::kInfo, "pml::nmos") << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }

    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++++++" << changed.lstRemoved.size() << "++++++++++++++" ;
}

void TestPoster::DeviceChanged(const pml::nmos::resourcechanges<pml::nmos::Device>& changed)
{

    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++DEVICES ADDED++++++" ;
    for(auto pResource : changed.lstAdded)
    {
        pml::log::log(pml::log::Level::kInfo, "pml::nmos") << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++++++" << changed.lstAdded.size() << "++++++++++++++" ;

    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++DEVICES UPDATED++++++" ;
    for(auto pResource : changed.lstUpdated)
    {
        pml::log::log(pml::log::Level::kInfo, "pml::nmos") << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++++++" << changed.lstUpdated.size() << "++++++++++++++" ;

    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++DEVICES REMOVE++++++" ;
    for(auto pResource : changed.lstRemoved)
    {
        pml::log::log(pml::log::Level::kInfo, "pml::nmos") << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }

    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++++++" << changed.lstRemoved.size() << "++++++++++++++" ;
}

void TestPoster::SourceChanged(const pml::nmos::resourcechanges<pml::nmos::Source>& changed)
{
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++SOURCES ADDED++++++" ;
    for(auto pResource : changed.lstAdded)
    {
        pml::log::log(pml::log::Level::kInfo, "pml::nmos") << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++++++" << changed.lstAdded.size() << "++++++++++++++" ;

    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++SOURCES UPDATED++++++" ;
    for(auto pResource : changed.lstUpdated)
    {
        pml::log::log(pml::log::Level::kInfo, "pml::nmos") << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++++++" << changed.lstUpdated.size() << "++++++++++++++" ;
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++SOURCES REMOVE++++++" ;
    for(auto pResource : changed.lstRemoved)
    {
        pml::log::log(pml::log::Level::kInfo, "pml::nmos") << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }

    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++++++" << changed.lstRemoved.size() << "++++++++++++++" ;
}

void TestPoster::FlowChanged(const pml::nmos::resourcechanges<pml::nmos::Flow>& changed)
{
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++FLOWS ADDED++++++" ;
    for(auto pResource : changed.lstAdded)
    {
        pml::log::log(pml::log::Level::kInfo, "pml::nmos") << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++++++" << changed.lstAdded.size() << "++++++++++++++" ;

    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++FLOWS UPDATED++++++" ;
    for(auto pResource : changed.lstUpdated)
    {
        pml::log::log(pml::log::Level::kInfo, "pml::nmos") << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++++++" << changed.lstUpdated.size() << "++++++++++++++" ;

    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++FLOWS REMOVE++++++" ;
    for(auto pResource : changed.lstRemoved)
    {
        pml::log::log(pml::log::Level::kInfo, "pml::nmos") << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }

    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++++++" << changed.lstRemoved.size() << "++++++++++++++" ;
}

void TestPoster::SenderChanged(const pml::nmos::resourcechanges<pml::nmos::Sender>& changed)
{
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++SENDERS ADDED++++++" ;
    for(auto pResource : changed.lstAdded)
    {
        pml::log::log(pml::log::Level::kInfo, "pml::nmos") << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++++++" << changed.lstAdded.size() << "++++++++++++++" ;

    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++SENDERS UPDATED++++++" ;
    for(auto pResource : changed.lstUpdated)
    {
        pml::log::log(pml::log::Level::kInfo, "pml::nmos") << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++++++" << changed.lstUpdated.size() << "++++++++++++++" ;
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++SENDERS REMOVE++++++" ;
    for(auto pResource : changed.lstRemoved)
    {
        pml::log::log(pml::log::Level::kInfo, "pml::nmos") << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }

    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++++++" << changed.lstRemoved.size() << "++++++++++++++" ;
}

void TestPoster::ReceiverChanged(const pml::nmos::resourcechanges<pml::nmos::Receiver>& changed)
{
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++RECEIVERS ADDED++++++" ;
    for(auto pResource : changed.lstAdded)
    {
        pml::log::log(pml::log::Level::kInfo, "pml::nmos") << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++++++" << changed.lstAdded.size() << "++++++++++++++" ;

    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++RECEIVERS UPDATED++++++" ;
    for(auto pResource : changed.lstUpdated)
    {
        pml::log::log(pml::log::Level::kInfo, "pml::nmos") << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++++++" << changed.lstUpdated.size() << "++++++++++++++" ;
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++RECEIVERS REMOVE++++++" ;
    for(auto pResource : changed.lstRemoved)
    {
        pml::log::log(pml::log::Level::kInfo, "pml::nmos") << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }

    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++++++" << changed.lstRemoved.size() << "++++++++++++++" ;
}

void TestPoster::QueryServerChanged(const std::string& sUrl)
{
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++QUERY SERVER CHANGED ++++++" ;
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << sUrl;
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++++++++++++++++++++" ;
}

void TestPoster::QueryServerFound(const std::string& sUrl, unsigned short nPriority)
{
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++QUERY SERVER FOUND ++++++" ;
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << sUrl << ": " << nPriority;
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++++++++++++++++++++" ;
}

void TestPoster::QueryServerRemoved(const std::string& sUrl)
{
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++QUERY SERVER REMOVED ++++++" ;
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << sUrl;
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++++++++++++++++++++" ;
}


void TestPoster::QuerySubscription(const std::string& sSubscriptionId, int nResource, const std::string& sQuery)
{
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++QUERY SUBSCRIPTION++++++" ;
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << sSubscriptionId;
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << nResource;
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << sQuery;
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++++++++++++++++++++" ;
}

void TestPoster::QuerySubscriptionRemoved(const std::string& sSubscriptionId)
{

}

void TestPoster::RequestTargetResult(unsigned long nResult, const std::string& sResponse, const std::string& sResourceId)
{
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++RequestTargetResult++++++" ;
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "Result = " << nResult;
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "Response = " << sResponse;
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "Resource = " << sResourceId;
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++++++++++++++++++++" ;
}

void TestPoster::RequestPatchSenderResult(const pml::restgoose::clientResponse& resp, const ::optional<connectionSender<activationResponse>>& con, const std::string& sResourceId)
{
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++RequestPatchSenderResult++++++" ;
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "Result = " << resp.nHttpCode;
    if(con)
    {
        pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "Response = " << (*con).GetJson();
    }
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "Resource = " << sResourceId;
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++++++++++++++++++++" ;
}

void TestPoster::RequestPatchReceiverResult(const pml::restgoose::clientResponse& resp, const ::optional<connectionReceiver<activationResponse>>& con, const std::string& sResourceId)
{
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++RequestPatchReceiverResult++++++" ;
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "Result = " << resp.nHttpCode;
    if(con)
    {
        pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "Response = " << (*con).GetJson();
    }
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "Resource = " << sResourceId;
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++++++++++++++++++++" ;
}

void TestPoster::RequestConnectResult(const std::string& sSenderId, const std::string& sReceiverId, bool bSuccess, const std::string& sResponse)
{
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++RequestConnectResult++++++" ;
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "Sender = " << sSenderId;
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "Receiver = " << sReceiverId;
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "Result = " << bSuccess;
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "Response = " << sResponse;
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++++++++++++++++++++" ;
}

void TestPoster::RequestGetSenderStagedResult(const pml::restgoose::clientResponse& resp, const ::optional<connectionSender<activationResponse>>& con, const std::string& sResourceId)
{
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++RequestGetSenderStagedResult++++++" ;
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "Result = " << resp.nHttpCode;
    if(con)
    {
        pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "Response = " << (*con).GetJson();
    }
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "Resource = " << sResourceId;
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++++++++++++++++++++" ;
}

void TestPoster::RequestGetSenderActiveResult(const pml::restgoose::clientResponse& resp, const ::optional<connectionSender<activationResponse>>& con, const std::string& sResourceId)
{
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++RequestGetSenderActiveResult++++++" ;
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "Result = " << resp.nHttpCode;
    if(con)
    {
        pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "Response = " << (*con).GetJson();
    }
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "Resource = " << sResourceId;
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++++++++++++++++++++" ;
}

void TestPoster::RequestGetSenderTransportFileResult(const pml::restgoose::clientResponse& resp, const ::optional<std::string>& sTransportFile, const std::string& sResourceId)
{
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++RequestGetSenderTransportFileResult++++++" ;
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "Result = " << resp.nHttpCode;
    if(sTransportFile)
    {
        pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "Response = " << (*sTransportFile);
    }
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "Resource = " << sResourceId;
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++++++++++++++++++++" ;
}

void TestPoster::RequestGetReceiverStagedResult(const pml::restgoose::clientResponse& resp, const ::optional<connectionReceiver<activationResponse>>& con, const std::string& sResourceId)
{
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++RequestGetReceiverStagedResult++++++" ;
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "Result = " << resp.nHttpCode;
    if(con)
    {
        pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "Response = " << (*con).GetJson();
    }
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "Resource = " << sResourceId;
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++++++++++++++++++++" ;
}

void TestPoster::RequestGetReceiverActiveResult(const pml::restgoose::clientResponse& resp, const ::optional<connectionReceiver<activationResponse>>& con, const std::string& sResourceId)
{
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++RequestGetReceiverActiveResult++++++" ;
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "Result = " << resp.nHttpCode;
    if(con)
    {
        pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "Response = " << (*con).GetJson();
    }
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "Resource = " << sResourceId;
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++++++++++++++++++++" ;
}


void TestPoster::RequestGetSenderConstraintsResult(const pml::restgoose::clientResponse& resp, const std::vector<pml::nmos::Constraints>& vConstraints, const std::string& sResourceId)
{
     pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++RequestGetSenderConstraintsResult++++++" ;
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "Result = " << resp.nHttpCode;
    
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "Resource = " << sResourceId;
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++++++++++++++++++++" ;
}

void TestPoster::RequestGetReceiverConstraintsResult(const pml::restgoose::clientResponse& resp, const std::vector<pml::nmos::Constraints>& vConstraints, const std::string& sResourceId)
{
     pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++RequestGetReceiverConstraintsResult++++++" ;
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "Result = " << resp.nHttpCode;
    
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "Resource = " << sResourceId;
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "++++++++++++++++++++++++" ;
}