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
        pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++QUERY MODE++++++" ;
    }
    else
    {
        pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++P2P MODE++++++" ;
    }
}
void TestPoster::NodeChanged(const pml::nmos::resourcechanges<pml::nmos::Self>& changed)
{
    pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++NODES ADDED++++++" ;
    for(auto pResource : changed.lstAdded)
    {
        pmlLog(pml::LOG_INFO, "pml::nmos") << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }
    pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++++++" << changed.lstAdded.size() << "++++++++++++++" ;

    pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++NODES UPDATED++++++" ;
    for(auto pResource :changed.lstUpdated)
    {
        pmlLog(pml::LOG_INFO, "pml::nmos") << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }
    pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++++++" << changed.lstUpdated.size() << "++++++++++++++" ;
    pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++NODES REMOVE++++++" ;
    for(auto pResource : changed.lstRemoved)
    {
        pmlLog(pml::LOG_INFO, "pml::nmos") << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }

    pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++++++" << changed.lstRemoved.size() << "++++++++++++++" ;
}

void TestPoster::DeviceChanged(const pml::nmos::resourcechanges<pml::nmos::Device>& changed)
{

    pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++DEVICES ADDED++++++" ;
    for(auto pResource : changed.lstAdded)
    {
        pmlLog(pml::LOG_INFO, "pml::nmos") << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }
    pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++++++" << changed.lstAdded.size() << "++++++++++++++" ;

    pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++DEVICES UPDATED++++++" ;
    for(auto pResource : changed.lstUpdated)
    {
        pmlLog(pml::LOG_INFO, "pml::nmos") << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }
    pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++++++" << changed.lstUpdated.size() << "++++++++++++++" ;

    pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++DEVICES REMOVE++++++" ;
    for(auto pResource : changed.lstRemoved)
    {
        pmlLog(pml::LOG_INFO, "pml::nmos") << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }

    pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++++++" << changed.lstRemoved.size() << "++++++++++++++" ;
}

void TestPoster::SourceChanged(const pml::nmos::resourcechanges<pml::nmos::Source>& changed)
{
    pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++SOURCES ADDED++++++" ;
    for(auto pResource : changed.lstAdded)
    {
        pmlLog(pml::LOG_INFO, "pml::nmos") << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }
    pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++++++" << changed.lstAdded.size() << "++++++++++++++" ;

    pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++SOURCES UPDATED++++++" ;
    for(auto pResource : changed.lstUpdated)
    {
        pmlLog(pml::LOG_INFO, "pml::nmos") << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }
    pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++++++" << changed.lstUpdated.size() << "++++++++++++++" ;
    pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++SOURCES REMOVE++++++" ;
    for(auto pResource : changed.lstRemoved)
    {
        pmlLog(pml::LOG_INFO, "pml::nmos") << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }

    pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++++++" << changed.lstRemoved.size() << "++++++++++++++" ;
}

void TestPoster::FlowChanged(const pml::nmos::resourcechanges<pml::nmos::Flow>& changed)
{
    pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++FLOWS ADDED++++++" ;
    for(auto pResource : changed.lstAdded)
    {
        pmlLog(pml::LOG_INFO, "pml::nmos") << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }
    pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++++++" << changed.lstAdded.size() << "++++++++++++++" ;

    pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++FLOWS UPDATED++++++" ;
    for(auto pResource : changed.lstUpdated)
    {
        pmlLog(pml::LOG_INFO, "pml::nmos") << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }
    pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++++++" << changed.lstUpdated.size() << "++++++++++++++" ;

    pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++FLOWS REMOVE++++++" ;
    for(auto pResource : changed.lstRemoved)
    {
        pmlLog(pml::LOG_INFO, "pml::nmos") << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }

    pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++++++" << changed.lstRemoved.size() << "++++++++++++++" ;
}

void TestPoster::SenderChanged(const pml::nmos::resourcechanges<pml::nmos::Sender>& changed)
{
    pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++SENDERS ADDED++++++" ;
    for(auto pResource : changed.lstAdded)
    {
        pmlLog(pml::LOG_INFO, "pml::nmos") << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }
    pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++++++" << changed.lstAdded.size() << "++++++++++++++" ;

    pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++SENDERS UPDATED++++++" ;
    for(auto pResource : changed.lstUpdated)
    {
        pmlLog(pml::LOG_INFO, "pml::nmos") << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }
    pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++++++" << changed.lstUpdated.size() << "++++++++++++++" ;
    pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++SENDERS REMOVE++++++" ;
    for(auto pResource : changed.lstRemoved)
    {
        pmlLog(pml::LOG_INFO, "pml::nmos") << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }

    pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++++++" << changed.lstRemoved.size() << "++++++++++++++" ;
}

void TestPoster::ReceiverChanged(const pml::nmos::resourcechanges<pml::nmos::Receiver>& changed)
{
    pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++RECEIVERS ADDED++++++" ;
    for(auto pResource : changed.lstAdded)
    {
        pmlLog(pml::LOG_INFO, "pml::nmos") << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }
    pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++++++" << changed.lstAdded.size() << "++++++++++++++" ;

    pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++RECEIVERS UPDATED++++++" ;
    for(auto pResource : changed.lstUpdated)
    {
        pmlLog(pml::LOG_INFO, "pml::nmos") << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }
    pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++++++" << changed.lstUpdated.size() << "++++++++++++++" ;
    pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++RECEIVERS REMOVE++++++" ;
    for(auto pResource : changed.lstRemoved)
    {
        pmlLog(pml::LOG_INFO, "pml::nmos") << pResource->GetJson(pml::nmos::ApiVersion(1,2)) ;
    }

    pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++++++" << changed.lstRemoved.size() << "++++++++++++++" ;
}

void TestPoster::QueryServerChanged(const std::string& sUrl)
{
    pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++QUERY SERVER CHANGED ++++++" ;
    pmlLog(pml::LOG_INFO, "pml::nmos") << sUrl;
    pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++++++++++++++++++++" ;
}

void TestPoster::QueryServerFound(const std::string& sUrl, unsigned short nPriority)
{
    pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++QUERY SERVER FOUND ++++++" ;
    pmlLog(pml::LOG_INFO, "pml::nmos") << sUrl << ": " << nPriority;
    pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++++++++++++++++++++" ;
}

void TestPoster::QueryServerRemoved(const std::string& sUrl)
{
    pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++QUERY SERVER REMOVED ++++++" ;
    pmlLog(pml::LOG_INFO, "pml::nmos") << sUrl;
    pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++++++++++++++++++++" ;
}


void TestPoster::QuerySubscription(const std::string& sSubscriptionId, int nResource, const std::string& sQuery)
{
    pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++QUERY SUBSCRIPTION++++++" ;
    pmlLog(pml::LOG_INFO, "pml::nmos") << sSubscriptionId;
    pmlLog(pml::LOG_INFO, "pml::nmos") << nResource;
    pmlLog(pml::LOG_INFO, "pml::nmos") << sQuery;
    pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++++++++++++++++++++" ;
}

void TestPoster::QuerySubscriptionRemoved(const std::string& sSubscriptionId)
{

}

void TestPoster::RequestTargetResult(unsigned long nResult, const std::string& sResponse, const std::string& sResourceId)
{
    pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++RequestTargetResult++++++" ;
    pmlLog(pml::LOG_INFO, "pml::nmos") << "Result = " << nResult;
    pmlLog(pml::LOG_INFO, "pml::nmos") << "Response = " << sResponse;
    pmlLog(pml::LOG_INFO, "pml::nmos") << "Resource = " << sResourceId;
    pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++++++++++++++++++++" ;
}

void TestPoster::RequestPatchSenderResult(const pml::restgoose::clientResponse& resp, const ::optional<connectionSender<activationResponse>>& con, const std::string& sResourceId)
{
    pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++RequestPatchSenderResult++++++" ;
    pmlLog(pml::LOG_INFO, "pml::nmos") << "Result = " << resp.nHttpCode;
    if(con)
    {
        pmlLog(pml::LOG_INFO, "pml::nmos") << "Response = " << (*con).GetJson();
    }
    pmlLog(pml::LOG_INFO, "pml::nmos") << "Resource = " << sResourceId;
    pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++++++++++++++++++++" ;
}

void TestPoster::RequestPatchReceiverResult(const pml::restgoose::clientResponse& resp, const ::optional<connectionReceiver<activationResponse>>& con, const std::string& sResourceId)
{
    pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++RequestPatchReceiverResult++++++" ;
    pmlLog(pml::LOG_INFO, "pml::nmos") << "Result = " << resp.nHttpCode;
    if(con)
    {
        pmlLog(pml::LOG_INFO, "pml::nmos") << "Response = " << (*con).GetJson();
    }
    pmlLog(pml::LOG_INFO, "pml::nmos") << "Resource = " << sResourceId;
    pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++++++++++++++++++++" ;
}

void TestPoster::RequestConnectResult(const std::string& sSenderId, const std::string& sReceiverId, bool bSuccess, const std::string& sResponse)
{
    pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++RequestConnectResult++++++" ;
    pmlLog(pml::LOG_INFO, "pml::nmos") << "Sender = " << sSenderId;
    pmlLog(pml::LOG_INFO, "pml::nmos") << "Receiver = " << sReceiverId;
    pmlLog(pml::LOG_INFO, "pml::nmos") << "Result = " << bSuccess;
    pmlLog(pml::LOG_INFO, "pml::nmos") << "Response = " << sResponse;
    pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++++++++++++++++++++" ;
}

void TestPoster::RequestGetSenderStagedResult(const pml::restgoose::clientResponse& resp, const ::optional<connectionSender<activationResponse>>& con, const std::string& sResourceId)
{
    pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++RequestGetSenderStagedResult++++++" ;
    pmlLog(pml::LOG_INFO, "pml::nmos") << "Result = " << resp.nHttpCode;
    if(con)
    {
        pmlLog(pml::LOG_INFO, "pml::nmos") << "Response = " << (*con).GetJson();
    }
    pmlLog(pml::LOG_INFO, "pml::nmos") << "Resource = " << sResourceId;
    pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++++++++++++++++++++" ;
}

void TestPoster::RequestGetSenderActiveResult(const pml::restgoose::clientResponse& resp, const ::optional<connectionSender<activationResponse>>& con, const std::string& sResourceId)
{
    pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++RequestGetSenderActiveResult++++++" ;
    pmlLog(pml::LOG_INFO, "pml::nmos") << "Result = " << resp.nHttpCode;
    if(con)
    {
        pmlLog(pml::LOG_INFO, "pml::nmos") << "Response = " << (*con).GetJson();
    }
    pmlLog(pml::LOG_INFO, "pml::nmos") << "Resource = " << sResourceId;
    pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++++++++++++++++++++" ;
}

void TestPoster::RequestGetSenderTransportFileResult(const pml::restgoose::clientResponse& resp, const ::optional<std::string>& sTransportFile, const std::string& sResourceId)
{
    pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++RequestGetSenderTransportFileResult++++++" ;
    pmlLog(pml::LOG_INFO, "pml::nmos") << "Result = " << resp.nHttpCode;
    if(sTransportFile)
    {
        pmlLog(pml::LOG_INFO, "pml::nmos") << "Response = " << (*sTransportFile);
    }
    pmlLog(pml::LOG_INFO, "pml::nmos") << "Resource = " << sResourceId;
    pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++++++++++++++++++++" ;
}

void TestPoster::RequestGetReceiverStagedResult(const pml::restgoose::clientResponse& resp, const ::optional<connectionReceiver<activationResponse>>& con, const std::string& sResourceId)
{
    pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++RequestGetReceiverStagedResult++++++" ;
    pmlLog(pml::LOG_INFO, "pml::nmos") << "Result = " << resp.nHttpCode;
    if(con)
    {
        pmlLog(pml::LOG_INFO, "pml::nmos") << "Response = " << (*con).GetJson();
    }
    pmlLog(pml::LOG_INFO, "pml::nmos") << "Resource = " << sResourceId;
    pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++++++++++++++++++++" ;
}

void TestPoster::RequestGetReceiverActiveResult(const pml::restgoose::clientResponse& resp, const ::optional<connectionReceiver<activationResponse>>& con, const std::string& sResourceId)
{
    pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++RequestGetReceiverActiveResult++++++" ;
    pmlLog(pml::LOG_INFO, "pml::nmos") << "Result = " << resp.nHttpCode;
    if(con)
    {
        pmlLog(pml::LOG_INFO, "pml::nmos") << "Response = " << (*con).GetJson();
    }
    pmlLog(pml::LOG_INFO, "pml::nmos") << "Resource = " << sResourceId;
    pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++++++++++++++++++++" ;
}


void TestPoster::RequestGetSenderConstraintsResult(const pml::restgoose::clientResponse& resp, const std::vector<pml::nmos::Constraints>& vConstraints, const std::string& sResourceId)
{
     pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++RequestGetSenderConstraintsResult++++++" ;
    pmlLog(pml::LOG_INFO, "pml::nmos") << "Result = " << resp.nHttpCode;
    
    pmlLog(pml::LOG_INFO, "pml::nmos") << "Resource = " << sResourceId;
    pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++++++++++++++++++++" ;
}

void TestPoster::RequestGetReceiverConstraintsResult(const pml::restgoose::clientResponse& resp, const std::vector<pml::nmos::Constraints>& vConstraints, const std::string& sResourceId)
{
     pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++RequestGetReceiverConstraintsResult++++++" ;
    pmlLog(pml::LOG_INFO, "pml::nmos") << "Result = " << resp.nHttpCode;
    
    pmlLog(pml::LOG_INFO, "pml::nmos") << "Resource = " << sResourceId;
    pmlLog(pml::LOG_INFO, "pml::nmos") << "++++++++++++++++++++++++" ;
}