#include "clientprivate.h"
#include "clientapi.h"
#include "clientposter.h"
#include "log.h"
#include "mdns.h"
#include <thread>
#include <set>
#ifdef __GNU__
#include "avahibrowser.h"
#endif // __GNU__
#include "curlregister.h"

#include "self.h"
#include "device.h"
#include "source.h"
#include "sourceaudio.h"
#include "sourcegeneric.h"
#include "flow.h"
#include "flowaudiocoded.h"
#include "flowvideocoded.h"
#include "flowaudioraw.h"
#include "flowvideoraw.h"
#include "flowdatasdianc.h"
#include "flowmux.h"
#include "senderbase.h"
#include "receiverbase.h"
#include "clientapiposter.h"
#include "utils.h"
#include <algorithm>
#include <numeric>

using namespace pml::nmos;
using namespace std::placeholders;

const std::string ClientApiImpl::STR_RESOURCE[6] = {"node", "device", "source", "flow", "sender", "receiver"};


//Main Client Thread
void ClientThread(ClientApiImpl* pApi)
{
    //start the browser
    pApi->AddBrowseServices();
    pApi->StartBrowsers();

    while(pApi->IsRunning())
    {
        if(pApi->Wait(5000))
        {
            switch(pApi->GetSignal())
            {
                case ClientApiImpl::CLIENT_SIG_INSTANCE_RESOLVED:
                    pApi->HandleInstanceResolved();
                    break;
                case ClientApiImpl::CLIENT_SIG_INSTANCE_REMOVED:
                    pApi->HandleInstanceRemoved();
                    break;
                case ClientApiImpl::CLIENT_SIG_NODE_BROWSED:
                    pApi->NodeDetailsDone();
                    break;
                case ClientApiImpl::CLIENT_SIG_CURL_DONE:
                    pApi->HandleCurlDone();
                    break;
                case ClientApiImpl::CLIENT_SIG_BROWSE_DONE:
                    pApi->HandleBrowseDone();
                default:
                    break;
            }
            pApi->GetNodeDetails();
        }
    }
    pApi->DeleteServiceBrowser();
}

//Called in NodeBrowse Thread
bool VersionChanged(std::shared_ptr<dnsInstance> pInstance, const std::string& sVersion)
{
    if(pInstance->nUpdate != 0)
    {
        auto itVer = pInstance->mTxt.find(sVersion);
        auto itVerLast = pInstance->mTxtLast.find(sVersion);

        if(itVer != pInstance->mTxt.end() && itVerLast != pInstance->mTxtLast.end())
        {
            if(itVer->second != itVerLast->second)
            {
                return true;
            }
        }
        return false;
    }
    return true;
}

//NodeBrowse Thread  - started within main ClientThread
static void NodeBrowser(ClientApiImpl* pApi, std::shared_ptr<dnsInstance> pInstance)
{
    if(pApi->GetMode() == ClientApiImpl::MODE_P2P)
    {
        auto itVersion = pInstance->mTxt.find("api_ver");
        if(itVersion != pInstance->mTxt.end())
        {
            if(itVersion->second.find("v1.2") != std::string::npos)
            {   //@todo allow choosing of version I guess
                std::stringstream ss;
                ss << pInstance->sHostIP << ":" << pInstance->nPort << "/x-nmos/node/v1.2/";

                if(VersionChanged(pInstance, "ver_slf"))
                {
                    pmlLog(pml::LOG_DEBUG) << "NMOS: NodeBrowser ver_slf changed" ;

                    auto curlresp = CurlRegister::Get(std::string(ss.str()+"self"), true);
                    std::list<std::shared_ptr<Self> > lstAdded;
                    std::list<std::shared_ptr<Self> > lstUpdated;
                    std::list<std::shared_ptr<Self> > lstRemoved;
                    pApi->AddNode(lstAdded, lstUpdated, pInstance->sHostIP, curlresp.sResponse);

                    if(pApi->RunQuery(lstAdded, lstUpdated, lstRemoved, ClientApiImpl::NODES))
                    {
                        if(pApi->GetPoster())
                        {
                            pApi->GetPoster()->_NodeChanged(lstAdded, lstUpdated, lstRemoved);
                        }
                    }
                    else
                    {
                        pmlLog(pml::LOG_DEBUG) << "NMOS: NodeBrowser Node does not meet query" ;
                    }
                }
                if(VersionChanged(pInstance, "ver_dvc"))
                {
                    pmlLog(pml::LOG_DEBUG) << "NMOS: NodeBrowser ver_dvc changed" ;

                    auto curlresp = CurlRegister::Get(std::string(ss.str()+"devices"), true);

                    std::list<std::shared_ptr<Device> > lstAdded;
                    std::list<std::shared_ptr<Device> > lstUpdated;
                    std::list<std::shared_ptr<Device> > lstRemoved;
                    //store the devices this ip address currently provides
                    pApi->StoreDevices(pInstance->sHostIP);
                    //add or update all devices on this ip address
                    pApi->AddDevices(lstAdded, lstUpdated, pInstance->sHostIP, curlresp.sResponse);
                    //remove any devices that we previously stored but didn;t update. i.e. ones that no longer exist
                    pApi->RemoveStaleDevices(lstRemoved);

                    if(pApi->RunQuery(lstAdded, lstUpdated, lstRemoved, ClientApiImpl::DEVICES))
                    {
                        if(pApi->GetPoster())
                        {
                            pApi->GetPoster()->_DeviceChanged(lstAdded, lstUpdated, lstRemoved);
                        }
                    }
                    else
                    {
                        pmlLog(pml::LOG_DEBUG) << "NMOS: NodeBrowser Device does not meet query" ;
                    }
                }
                if(VersionChanged(pInstance, "ver_src"))
                {
                    pmlLog(pml::LOG_DEBUG) << "NMOS: NodeBrowser ver_src changed" ;

                    auto curlresp = CurlRegister::Get(std::string(ss.str()+"sources"), true);

                    std::list<std::shared_ptr<Source> > lstAdded;
                    std::list<std::shared_ptr<Source> > lstUpdated;
                    std::list<std::shared_ptr<Source> > lstRemoved;

                    pApi->StoreSources(pInstance->sHostIP);
                    pApi->AddSources(lstAdded, lstUpdated, pInstance->sHostIP, curlresp.sResponse);
                    pApi->RemoveStaleSources(lstRemoved);

                    if(pApi->RunQuery(lstAdded, lstUpdated, lstRemoved, ClientApiImpl::SOURCES))
                    {
                        if(pApi->GetPoster())
                        {
                            pApi->GetPoster()->_SourceChanged(lstAdded, lstUpdated, lstRemoved);
                        }
                    }
                    else
                    {
                        pmlLog(pml::LOG_DEBUG) << "NMOS: NodeBrowser source does not meet query" ;
                    }
                }
                if(VersionChanged(pInstance, "ver_flw"))
                {
                    pmlLog(pml::LOG_DEBUG) << "NMOS: NodeBrowser ver_flw changed" ;

                    auto curlresp = CurlRegister::Get(std::string(ss.str()+"flows"), true);

                    std::list<std::shared_ptr<Flow> > lstAdded;
                    std::list<std::shared_ptr<Flow> > lstUpdated;
                    std::list<std::shared_ptr<Flow> > lstRemoved;

                    pApi->StoreFlows(pInstance->sHostIP);
                    pApi->AddFlows(lstAdded, lstUpdated, pInstance->sHostIP, curlresp.sResponse);
                    pApi->RemoveStaleFlows(lstRemoved);

                    if(pApi->RunQuery(lstAdded, lstUpdated, lstRemoved, ClientApiImpl::FLOWS))
                    {
                        if(pApi->GetPoster())
                        {
                            pApi->GetPoster()->_FlowChanged(lstAdded, lstUpdated, lstRemoved);
                        }
                    }
                    else
                    {
                        pmlLog(pml::LOG_DEBUG) << "NMOS: NodeBrowser Flow does not meet query" ;
                    }
                }
                if(VersionChanged(pInstance, "ver_snd"))
                {
                    pmlLog(pml::LOG_DEBUG) << "NMOS: NodeBrowser ver_snd changed" ;

                    auto curlresp = CurlRegister::Get(std::string(ss.str()+"senders"), true);

                    std::list<std::shared_ptr<SenderBase> > lstAdded;
                    std::list<std::shared_ptr<SenderBase> > lstUpdated;
                    std::list<std::shared_ptr<SenderBase> > lstRemoved;

                    pApi->StoreSenders(pInstance->sHostIP);
                    pApi->AddSenders(lstAdded, lstUpdated, pInstance->sHostIP, curlresp.sResponse);
                    pApi->RemoveStaleSenders(lstRemoved);

                    if(pApi->RunQuery(lstAdded, lstUpdated, lstRemoved, ClientApiImpl::SENDERS))
                    {
                        if(pApi->GetPoster())
                        {
                            pApi->GetPoster()->_SenderChanged(lstAdded, lstUpdated, lstRemoved);
                        }
                    }
                    else
                    {
                        pmlLog(pml::LOG_DEBUG) << "NMOS: NodeBrowser Sender does not meet query" ;
                    }
                }
                if(VersionChanged(pInstance, "ver_rcv"))
                {
                    pmlLog(pml::LOG_DEBUG) << "NMOS: NodeBrowser ver_rcv changed" ;

                    auto curlresp = CurlRegister::Get(std::string(ss.str()+"receivers"), true);

                    std::list<std::shared_ptr<ReceiverBase> > lstAdded;
                    std::list<std::shared_ptr<ReceiverBase> > lstUpdated;
                    std::list<std::shared_ptr<ReceiverBase> > lstRemoved;
                    pApi->StoreReceivers(pInstance->sHostIP);
                    pApi->AddReceivers(lstAdded, lstUpdated, pInstance->sHostIP, curlresp.sResponse);
                    pApi->RemoveStaleReceivers(lstRemoved);

                    if(pApi->RunQuery(lstAdded, lstUpdated, lstRemoved, ClientApiImpl::RECEIVERS))
                    {
                        if(pApi->GetPoster())
                        {
                            pApi->GetPoster()->_ReceiverChanged(lstAdded, lstUpdated, lstRemoved);
                        }
                    }
                    else
                    {
                        pmlLog(pml::LOG_DEBUG) << "NMOS: NodeBrowser Receiver does not meet query" ;
                    }
                }

            }
        }
        pApi->Signal(ClientApiImpl::CLIENT_SIG_NODE_BROWSED);
    }
}

//ConnectThread - called from main program
void ConnectThread(ClientApiImpl* pApi, const std::string& sSenderId, const std::string& sReceiverId, const std::string& sSenderStage, const std::string& sSenderTransport, const std::string& sReceiverStage)
{
    // @todo ConnectThread - if a unicast stream then tell the sender where it should be sending stuff
    // @todo ConnectTrhead = multiple versions of connecting

    connectionSender aCon(connection::FP_ACTIVATION | connection::FP_ENABLE | connection::FP_TRANSPORT_PARAMS);
    aCon.eActivate = connection::ACT_NOW;
    aCon.bMasterEnable = true;
    aCon.tpSender.bRtpEnabled = true;


    auto curlresp = CurlRegister::PutPatch(sSenderStage, ConvertFromJson(aCon.GetJson(ApiVersion(1,0))), false, "");
    if(curlresp.nCode != 200)
    {
        pApi->HandleConnect(sSenderId, sReceiverId, false, curlresp.sResponse);
    }
    else
    {
        connectionReceiver aConR(connection::FP_TRANSPORT_FILE | connection::FP_ACTIVATION | connection::FP_ENABLE | connection::FP_ID | connection::FP_TRANSPORT_PARAMS);
        aConR.eActivate = connection::ACT_NOW;
        aConR.bMasterEnable = true;
        aConR.tpReceiver.bRtpEnabled = true;
        aConR.sTransportFileType = "application/sdp";
        aConR.sSenderId = sSenderId;


        auto curlresp = CurlRegister::Get(sSenderTransport, false);
        if(curlresp.nCode != 200)
        {
            pApi->HandleConnect(sSenderId, sReceiverId, false, curlresp.sResponse);
        }
        else
        {
            aConR.sTransportFileData = curlresp.sResponse;

            std::string sData(ConvertFromJson(aConR.GetJson(ApiVersion(1,0))));
            auto curlresp = CurlRegister::PutPatch(sReceiverStage, sData, false, "");
            if(curlresp.nCode != 200)
            {
                pApi->HandleConnect(sSenderId, sReceiverId, false, curlresp.sResponse);
            }
            else
            {
                pApi->HandleConnect(sSenderId, sReceiverId, true, curlresp.sResponse);
            }
        }
    }

}


//DisconnectThread - called from main program thread
void DisconnectThread(ClientApiImpl* pApi, const std::string& sSenderId, const std::string& sReceiverId, const std::string& sSenderStage, const std::string& sSenderTransport, const std::string& sReceiverStage)
{
    int nResult(200);
    if(false)
    {// @todo ConnectThread - if a unicast stream then tell the sender to stop sending stuff
        connectionSender aCon(connection::FP_ACTIVATION | connection::FP_ENABLE | connection::FP_TRANSPORT_PARAMS);
        aCon.eActivate = connection::ACT_NOW;
        aCon.bMasterEnable = true;
        aCon.tpSender.bRtpEnabled = true;

        auto curlresp = CurlRegister::PutPatch(sSenderStage, ConvertFromJson(aCon.GetJson(ApiVersion(1,0))), false, "");
        if(curlresp.nCode != 200)
        {
            pApi->HandleConnect(sSenderId, sReceiverId, false, curlresp.sResponse);
            return;
        }
    }

    connectionReceiver aConR(connection::FP_TRANSPORT_FILE | connection::FP_ACTIVATION | connection::FP_ENABLE | connection::FP_ID | connection::FP_TRANSPORT_PARAMS);
    aConR.eActivate = connection::ACT_NOW;
    aConR.bMasterEnable = false;
    aConR.tpReceiver.bRtpEnabled = false;
    aConR.sTransportFileType = "application/sdp";
    aConR.sTransportFileData = "";
    aConR.sSenderId = "";

    auto curlresp = CurlRegister::PutPatch(sReceiverStage, ConvertFromJson(aConR.GetJson(ApiVersion(1,0))), false, "");
    if(curlresp.nCode != 200)
    {
        pApi->HandleConnect("", sReceiverId, false, curlresp.sResponse);
    }
    else
    {
        pApi->HandleConnect(sSenderId, sReceiverId, true, curlresp.sResponse);
    }
}


/************************************************
Class Start
************************************************/

void ClientApiImpl::Start()
{
    if(!m_pThread)
    {
        m_pThread = std::make_unique<std::thread>(ClientThread, this);
    }
}

void ClientApiImpl::Stop()
{
    StopRun();
}

ClientApiImpl::ClientApiImpl() :
    m_eMode(MODE_P2P),
    m_bRun(true),
    m_pInstance(0),
    m_nCurlThreadCount(0),
    m_pPoster(0),
    m_pClientPoster(std::make_shared<ClientPoster>()),
    m_pClientZCPoster(std::make_shared<ClientZCPoster>()),
    m_pCurl(new CurlRegister(std::bind(&ClientApiImpl::SetCurlDone, this, _1,_2,_3,_4))),
    m_pThread(nullptr),
    m_bStarted(false),
    m_bDoneQueryBrowse(false)
{
    m_mBrowser.insert(std::make_pair("local", std::make_unique<ServiceBrowser>()));
}

ClientApiImpl::~ClientApiImpl()
{
    Stop();
}

void ClientApiImpl::SetPoster(std::shared_ptr<ClientApiPoster> pPoster)
{
    m_pPoster = pPoster;
}

std::shared_ptr<ClientApiPoster> ClientApiImpl::GetPoster()
{
    return m_pPoster;
}


bool ClientApiImpl::Wait(unsigned long nMilliseconds)
{
    m_mutex.lock();
    m_eSignal = CLIENT_SIG_NONE;
    m_mutex.unlock();

    std::unique_lock<std::mutex> ul(m_mutex);
    return (m_cvBrowse.wait_for(ul, std::chrono::milliseconds(nMilliseconds)) == std::cv_status::no_timeout);
}

bool ClientApiImpl::IsRunning()
{
    return m_bRun;
}

void ClientApiImpl::StopRun()
{
    if(m_pThread)
    {
        m_bRun = false;
        Signal(THREAD_EXIT);
        m_pThread->join();
        m_pThread = nullptr;
    }

}

std::shared_ptr<EventPoster> ClientApiImpl::GetClientPoster()
{
    return m_pClientPoster;
}

std::shared_ptr<ZCPoster> ClientApiImpl::GetClientZCPoster()
{
    return m_pClientZCPoster;
}

ClientApiImpl::enumSignal ClientApiImpl::GetSignal()
{
    std::lock_guard<std::mutex> lg(m_mutex);
    return m_eSignal;
}


void ClientApiImpl::SetCurlDone(unsigned long nResult, const std::string& sResponse, long nType, const std::string& sResourceId)
{
    m_mutex.lock();
    m_nCurlResult = nResult;
    m_sCurlResponse = sResponse;
    m_nCurlType = nType;
    m_eSignal = CLIENT_SIG_CURL_DONE;
    m_sCurlResourceId = sResourceId;
    m_mutex.unlock();
    m_cvBrowse.notify_one();
}

void ClientApiImpl::SetInstanceResolved(std::shared_ptr<dnsInstance> pInstance)
{
    m_mutex.lock();
    m_pInstance = pInstance;
    m_eSignal = CLIENT_SIG_INSTANCE_RESOLVED;
    m_mutex.unlock();
    m_cvBrowse.notify_one();
}

void ClientApiImpl::SetInstanceRemoved(std::shared_ptr<dnsInstance> pInstance)
{
    m_mutex.lock();
    m_pInstance = pInstance;
    m_eSignal = CLIENT_SIG_INSTANCE_REMOVED;
    m_mutex.unlock();
    m_cvBrowse.notify_one();
}

void ClientApiImpl::SetAllForNow(const std::string& sService)
{
    m_mutex.lock();
    m_sService = sService;
    m_eSignal = CLIENT_SIG_BROWSE_DONE;
    m_mutex.unlock();
    m_cvBrowse.notify_one();
}

void ClientApiImpl::Signal(enumSignal eSignal)
{
    m_mutex.lock();
    m_eSignal = eSignal;
    m_mutex.unlock();

    m_cvBrowse.notify_one();
}

void ClientApiImpl::HandleBrowseDone()
{
    std::lock_guard<std::mutex> lg(m_mutex);
    pmlLog(pml::LOG_INFO) << "NMOS: " << "Browsing for '" << m_sService << "' done for now." ;
    if(m_sService == "_nmos-query._tcp")
    {
        m_bDoneQueryBrowse = true;
        //first entry in our multimap of servers will have joint highest priority so lets use it
        SubscribeToQueryServer();
    }
}

void ClientApiImpl::HandleInstanceResolved()
{
    std::lock_guard<std::mutex> lg(m_mutex);
    if(m_pInstance)
    {
        if(m_pInstance->sService == "_nmos-query._tcp")
        {
            pmlLog(pml::LOG_INFO) << "NMOS: " << "Query node found: " << m_pInstance->sName ;
            m_lstResolve.clear();

            //add the node to our priority sorted list of query servers
            auto itTxt = m_pInstance->mTxt.find("pri");
            if(itTxt != m_pInstance->mTxt.end())
            {
                try
                {
                    m_mmQueryNodes.insert(std::make_pair(std::stoi(itTxt->second), m_pInstance));
                    if(m_eMode == MODE_P2P)
                    {   //change the mode away from peer to peer
                        m_eMode = MODE_REGISTRY;
                        if(m_pPoster)
                        {
                            m_pPoster->_ModeChanged(true);
                        }
                    }
                    if(m_bDoneQueryBrowse)
                    {   //could be told all done before we've been given the node due to threading
                        SubscribeToQueryServer();
                    }
                }
                catch(std::invalid_argument& ia)
                {
                    pmlLog(pml::LOG_ERROR) << "NMOS: " << "Priority '" << itTxt->second << "' invalid" ;
                }
            }
        }
        else if(m_pInstance->sService == "_nmos-node._tcp" && m_eMode == MODE_P2P)
        {
            if(m_pInstance->nUpdate == 0)
            {
                pmlLog(pml::LOG_INFO) << "NMOS: " << "Nmos node found" ;
                m_lstResolve.push_back(m_pInstance);
            }
            else
            {
                pmlLog(pml::LOG_INFO) << "NMOS: " << "Nmos node updated" ;
                m_lstResolve.push_back(m_pInstance);
            }
        }
        m_pInstance = 0;
    }
}

void ClientApiImpl::HandleInstanceRemoved()
{
    if(m_pInstance)
    {
        if(m_pInstance->sService == "_nmos_query._tcp")
        {
            pmlLog(pml::LOG_INFO) << "NMOS: " << "Query node: " << m_pInstance->sName << "removed" ;
            if(m_eMode == MODE_REGISTRY)
            {   //@todo should we check here if we have another node??
                m_eMode = MODE_P2P;
                if(m_pPoster)
                {
                    m_pPoster->_ModeChanged(false);
                }
            }
        }
        else if(m_pInstance->sService == "_nmos-node._tcp" && m_eMode == MODE_P2P)
        {
            pmlLog(pml::LOG_INFO) << "NMOS: " << "node Removed: " << m_pInstance->sName ;
            m_lstResolve.remove_if([this](std::shared_ptr<dnsInstance> pInstance) { return pInstance == m_pInstance;});
//            for(auto pInstance : m_lstResolve)
//            {
//                if(pInstance == m_pInstance)
//                {
//                    m_lstResolve.erase(pInstance);
//                    break;
//                }
//            }
            RemoveResources(m_pInstance->sHostIP);
        }
        m_pInstance = 0;
    }
}


void ClientApiImpl::HandleConnect(const std::string& sSenderId, const std::string& sReceiverId, bool bSuccess, const std::string& sResponse)
{
    std::lock_guard<std::mutex> lg(m_mutex);
    if(m_pPoster)
    {
        m_pPoster->_RequestConnectResult(sSenderId, sReceiverId, bSuccess, sResponse);
    }
}


void ClientApiImpl::HandleCurlDone()
{
    std::lock_guard<std::mutex> lg(m_mutex);
    if(m_pPoster)
    {
        switch(m_nCurlType)
        {
            case ClientPoster::CURLTYPE_TARGET:
                HandleCurlDoneTarget();
                break;
            case ClientPoster::CURLTYPE_SENDER_STAGED:
                HandleCurlDoneGetSenderStaged();
                break;
            case ClientPoster::CURLTYPE_SENDER_ACTIVE:
                HandleCurlDoneGetSenderActive();
                break;
            case ClientPoster::CURLTYPE_SENDER_TRANSPORTFILE:
                HandlCurlDoneGetSenderTransportFile();
                break;
            case ClientPoster::CURLTYPE_RECEIVER_STAGED:
                HandleCurlDoneGetReceiverStaged();
                break;
            case ClientPoster::CURLTYPE_RECEIVER_ACTIVE:
                void HandleCurlDoneGetReceiverActive();
                break;
            case ClientPoster::CURLTYPE_SENDER_PATCH:
                HandleCurlDonePatchSender();
                break;
            case ClientPoster::CURLTYPE_RECEIVER_PATCH:
                HandleCurlDonePatchReceiver();
                break;
         }
}
}

void ClientApiImpl::HandleCurlDoneTarget()
{
    //don't need to check for m_pPoster as check in HandleCurlDon
    Json::Value jsData = ConvertToJson(m_sCurlResponse);
    if(202 == m_nCurlResult && jsData["id"].isString())
    {
        pmlLog(pml::LOG_DEBUG) << "NMOS: " << m_nCurlResult << ": " << jsData["id"].asString() ;
        m_pPoster->_RequestTargetResult(202, jsData["id"].asString(), m_sCurlResourceId);
        return;
    }

    if(jsData["error"].isString())
    {
        pmlLog(pml::LOG_DEBUG) << "NMOS: " << m_nCurlResult << ": " << jsData["error"].asString() ;
        m_pPoster->_RequestTargetResult(m_nCurlResult, jsData["error"].asString(), m_sCurlResourceId);
        return;
    }
    pmlLog(pml::LOG_DEBUG) << "NMOS: " << m_nCurlResult << ": " << m_sCurlResponse ;
    m_pPoster->_RequestTargetResult(m_nCurlResult, m_sCurlResponse, m_sCurlResourceId);
}

void ClientApiImpl::HandleCurlDonePatchSender()
{
    Json::Value jsData = ConvertToJson(m_sCurlResponse);

    //don't need to check for m_pPoster as check in HandleCurlDon
    if(m_nCurlResult != 200 && m_nCurlResult != 202 && jsData["error"].isString())
    {
        m_pPoster->_RequestPatchSenderResult(m_nCurlResult, jsData["error"].asString(), m_sCurlResourceId);
    }
    else
    {
        m_pPoster->_RequestPatchSenderResult(m_nCurlResult, m_sCurlResponse, m_sCurlResourceId);
    }
}

void ClientApiImpl::HandleCurlDonePatchReceiver()
{
    //don't need to check for m_pPoster as check in HandleCurlDon
    Json::Value jsData = ConvertToJson(m_sCurlResponse);
    if(m_nCurlResult != 200 && m_nCurlResult != 202 && jsData["error"].isString())
    {
        m_pPoster->_RequestPatchReceiverResult(m_nCurlResult, jsData["error"].asString(), m_sCurlResourceId);
    }
    else
    {
        m_pPoster->_RequestPatchReceiverResult(m_nCurlResult, m_sCurlResponse, m_sCurlResourceId);
    }
}

void ClientApiImpl::HandleCurlDoneGetSenderStaged()
{
    //don't need to check for m_pPoster as check in HandleCurlDon
    Json::Value jsData = ConvertToJson(m_sCurlResponse);

    if(m_nCurlResult != 200 && jsData["error"].isString())
    {
        m_pPoster->_RequestGetSenderStagedResult(m_nCurlResult, jsData["error"].asString(), m_sCurlResourceId);
    }
    else
    {
        m_pPoster->_RequestGetSenderStagedResult(m_nCurlResult, m_sCurlResponse, m_sCurlResourceId);
    }
}

void ClientApiImpl::HandleCurlDoneGetSenderActive()
{
    //don't need to check for m_pPoster as check in HandleCurlDon
    Json::Value jsData = ConvertToJson(m_sCurlResponse);
    if(m_nCurlResult != 200 && jsData["error"].isString())
    {
        m_pPoster->_RequestGetSenderActiveResult(m_nCurlResult, jsData["error"].asString(), m_sCurlResourceId);
    }
    else
    {
        m_pPoster->_RequestGetSenderActiveResult(m_nCurlResult, m_sCurlResponse, m_sCurlResourceId);
    }

}

void ClientApiImpl::HandlCurlDoneGetSenderTransportFile()
{
    //don't need to check for m_pPoster as check in HandleCurlDon
    Json::Value jsData = ConvertToJson(m_sCurlResponse);
    if(m_nCurlResult != 200 && jsData["error"].isString())
    {
        m_pPoster->_RequestGetSenderTransportFileResult(m_nCurlResult, jsData["error"].asString(), m_sCurlResourceId);
    }
    else
    {
        m_pPoster->_RequestGetSenderTransportFileResult(m_nCurlResult, m_sCurlResponse, m_sCurlResourceId);
    }
}

void ClientApiImpl::HandleCurlDoneGetReceiverStaged()
{
    //don't need to check for m_pPoster as check in HandleCurlDon
    Json::Value jsData = ConvertToJson(m_sCurlResponse);
    {
        if(m_nCurlResult != 200 && jsData["error"].isString())
        {
            m_pPoster->_RequestGetReceiverStagedResult(m_nCurlResult, jsData["error"].asString(), m_sCurlResourceId);
        }
        else
        {
            m_pPoster->_RequestGetReceiverStagedResult(m_nCurlResult, m_sCurlResponse, m_sCurlResourceId);
        }
    }
}

void ClientApiImpl::HandleCurlDoneGetReceiverActive()
{
    //don't need to check for m_pPoster as check in HandleCurlDon
    Json::Value jsData = ConvertToJson(m_sCurlResponse);
    {
        if(m_nCurlResult != 200 && jsData["error"].isString())
        {
            m_pPoster->_RequestGetReceiverActiveResult(m_nCurlResult, jsData["error"].asString(), m_sCurlResourceId);
        }
        else
        {
            m_pPoster->_RequestGetReceiverActiveResult(m_nCurlResult, m_sCurlResponse, m_sCurlResourceId);
        }
    }
}

void ClientApiImpl::ConnectToQueryServer()
{
    // @todo ConnectToQueryServer
}

void ClientApiImpl::GetNodeDetails()
{
    if(m_lstResolve.empty() == false && m_nCurlThreadCount < 4)
    {
        pmlLog(pml::LOG_DEBUG) << "NMOS: ClientApiImpl::GetNodeDetails" ;

        m_nCurlThreadCount++;
        //need to get all the node details.
        //lets launch a thread that will ask for self, devices, sources, flows, senders, receivers
        std::thread th(NodeBrowser, this, m_lstResolve.front());
        th.detach();    //@todo better to wait on the thread at close down
        m_lstResolve.erase(m_lstResolve.begin());
    }
}

ClientApiImpl::enumMode ClientApiImpl::GetMode()
{
    std::lock_guard<std::mutex> lg(m_mutex);
    return m_eMode;
}



void ClientApiImpl::AddNode(std::list<std::shared_ptr<Self> >& lstAdded, std::list<std::shared_ptr<Self> >& lstUpdated,const std::string& sIpAddress, const std::string& sData)
{
    std::lock_guard<std::mutex> lg(m_mutex);

    Json::Value jsData = ConvertToJson(sData);

    if(jsData["id"].isString())
    {
        auto pSelf = m_nodes.UpdateResource(jsData);
        if(!pSelf)
        {
            pSelf = std::make_shared<Self>();
            if(pSelf->UpdateFromJson(jsData))
            {
                m_nodes.AddResource(sIpAddress, pSelf);
                lstAdded.push_back(pSelf);

                pmlLog(pml::LOG_INFO) << "NMOS: " << "Node: " << pSelf->GetId() << " found at " << sIpAddress ;
            }
            else
            {
                pmlLog(pml::LOG_INFO) << "NMOS: " << "Found node but json data incorrect: " << pSelf->GetJsonParseError() ;
            }
        }
        else
        {
            lstUpdated.push_back(pSelf);
        }
    }
    else
    {
        pmlLog(pml::LOG_INFO) << "NMOS: " << "Reply from " << sIpAddress << "but not valid JSON - id not correct" ;
    }
}

void ClientApiImpl::AddDevices(std::list<std::shared_ptr<Device> >& lstAdded, std::list<std::shared_ptr<Device> >& lstUpdated, const std::string& sIpAddress,
const std::string& sData)
{
    std::lock_guard<std::mutex> lg(m_mutex);

    Json::Value jsData = ConvertToJson(sData);
    if(jsData.isArray())
    {
        for(Json::ArrayIndex ai = 0; ai < jsData.size(); ++ai)
        {
            if(jsData[ai].isObject() && jsData[ai]["id"].isString())
            {
                auto pResource = m_devices.UpdateResource(jsData[ai]);
                if(!pResource)
                {
                    pResource = std::make_shared<Device>();
                    if(pResource->UpdateFromJson(jsData[ai]))
                    {
                        m_devices.AddResource(sIpAddress, pResource);
                        lstAdded.push_back(pResource);
                        pmlLog(pml::LOG_INFO) << "NMOS: " << "Device: " << pResource->GetId() << " found at " << sIpAddress ;
                    }
                    else
                    {
                        pmlLog(pml::LOG_INFO) << "NMOS: " << "Found device but json data incorrect: " << pResource->GetJsonParseError() ;
                    }
                }
                else
                {
                    lstUpdated.push_back(pResource);
                }
            }
            else
            {
                pmlLog(pml::LOG_INFO) << "NMOS: " << "Reply from " << sIpAddress << "but not JSON is not an array of objects" ;
            }
        }
    }
    else
    {
        pmlLog(pml::LOG_INFO) << "NMOS: " << "Reply from " << sIpAddress << "but not JSON is not array" ;
    }
}

void ClientApiImpl::AddSources(std::list<std::shared_ptr<Source> >& lstAdded, std::list<std::shared_ptr<Source> >& lstUpdated, const std::string& sIpAddress, const std::string& sData)
{
    std::lock_guard<std::mutex> lg(m_mutex);
    Json::Value jsData = ConvertToJson(sData);
    if(jsData.isArray())
    {
        for(Json::ArrayIndex ai = 0; ai < jsData.size(); ++ai)
        {
            if(jsData[ai].isObject() && jsData[ai]["format"].isString() && jsData[ai]["id"].isString())
            {
                auto pSourceCore = m_sources.UpdateResource(jsData[ai]);
                if(!pSourceCore)
                {
                    if(jsData[ai]["format"].asString().find("urn:x-nmos:format:audio") != std::string::npos)
                    {   //Audio
                        auto pResource = std::make_shared<SourceAudio>();
                        if(pResource->UpdateFromJson(jsData[ai]))
                        {
                            m_sources.AddResource(sIpAddress, pResource);
                            lstAdded.push_back(pResource);
                            pmlLog(pml::LOG_INFO) << "NMOS: " << "SourceAudio: " << pResource->GetId() << " found at " << sIpAddress ;
                        }
                        else
                        {
                            pmlLog(pml::LOG_INFO) << "NMOS: " << "Found Source but json data incorrect: " << pResource->GetJsonParseError() ;
                        }
                    }
                    else
                    {   //Generic
                        auto pResource = std::make_shared<SourceGeneric>();
                        if(pResource->UpdateFromJson(jsData[ai]))
                        {
                            m_sources.AddResource(sIpAddress, pResource);

                            lstAdded.push_back(pResource);
                            pmlLog(pml::LOG_INFO) << "NMOS: " << "SourceGeneric: " << pResource->GetId() << " found at " << sIpAddress ;
                        }
                        else
                        {
                            pmlLog(pml::LOG_INFO) << "NMOS: " << "Found Source but json data incorrect: " << pResource->GetJsonParseError() ;
                        }
                    }
                }
                else
                {
                    lstUpdated.push_back(pSourceCore);
                }
            }
            else
            {
                pmlLog(pml::LOG_INFO) << "NMOS: " << "Reply from " << sIpAddress << "but not JSON 'format' is ill defined" ;
            }
        }
    }
    else
    {
        pmlLog(pml::LOG_INFO) << "NMOS: " << "Reply from " << sIpAddress << "but not JSON is not array" ;
    }
}

void ClientApiImpl::AddFlows(std::list<std::shared_ptr<Flow> >& lstAdded, std::list<std::shared_ptr<Flow> >& lstUpdated, const std::string& sIpAddress, const std::string& sData)
{
    std::lock_guard<std::mutex> lg(m_mutex);
    Json::Value jsData = ConvertToJson(sData);

    if(jsData.isArray())
    {
        for(Json::ArrayIndex ai = 0; ai < jsData.size(); ++ai)
        {
            if(jsData[ai].isObject() && jsData[ai]["format"].isString() && jsData[ai]["id"].isString())
            {
                auto pFlowCore = m_flows.UpdateResource(jsData[ai]);
                if(!pFlowCore)
                {
                    CreateFlow(lstAdded, jsData[ai], sIpAddress);
                }
                else
                {
                    lstUpdated.push_back(pFlowCore);
                }
            }
            else
            {
                pmlLog(pml::LOG_INFO) << "NMOS: " << "Reply from " << sIpAddress << "but not JSON 'format' is ill defined" ;
            }
        }
    }
    else
    {
        pmlLog(pml::LOG_INFO) << "NMOS: " << "Reply from " << sIpAddress << "but not JSON is not array" ;
    }
}


void ClientApiImpl::CreateFlow(std::list<std::shared_ptr<Flow>>& lstAdded, const Json::Value& jsData, const std::string& sIpAddress)
{
    if(jsData["format"].asString().find("urn:x-nmos:format:audio") != std::string::npos)
    {
        CreateFlowAudio(lstAdded, jsData, sIpAddress);
    }
    else if(jsData["format"].asString().find("urn:x-nmos:format:video") != std::string::npos)
    {
        CreateFlowVideo(lstAdded, jsData, sIpAddress);
    }
    else if(jsData["format"].asString().find("urn:x-nmos:format:data") != std::string::npos)
    {
        CreateFlowData(lstAdded, jsData, sIpAddress);
    }
    else if(jsData["format"].asString().find("urn:x-nmos:format:mux") != std::string::npos)
    {
        CreateFlowMux(lstAdded, jsData, sIpAddress);
    }
}

void ClientApiImpl::CreateFlowAudio(std::list<std::shared_ptr<Flow>>& lstAdded, const Json::Value& jsData, const std::string& sIpAddress)
{
    if(jsData["media_type"].isString())
    {
        if(jsData["media_type"].asString() == "audio/L24" || jsData["media_type"].asString() == "audio/L20" || jsData["media_type"].asString() == "audio/L16" || jsData["media_type"].asString() == "audio/L8")
        {   //Raw Audio
            CreateFlowAudioRaw(lstAdded, jsData, sIpAddress);
        }
        else
        {   //Code audio
            CreateFlowAudioCoded(lstAdded, jsData, sIpAddress);
        }
    }
}

void ClientApiImpl::CreateFlowAudioCoded(std::list<std::shared_ptr<Flow>>& lstAdded, const Json::Value& jsData, const std::string& sIpAddress)
{
    auto pResource = std::make_shared<FlowAudioCoded>();
    if(pResource->UpdateFromJson(jsData))
    {
        m_flows.AddResource(sIpAddress, pResource);
        lstAdded.push_back(pResource);
        pmlLog(pml::LOG_INFO) << "NMOS: " << "FlowAudioCoded: " << pResource->GetId() << " found at " << sIpAddress ;
    }
    else
    {
        pmlLog(pml::LOG_INFO) << "NMOS: " << "Found Flow but json data incorrect: " << pResource->GetJsonParseError() ;
    }
}
void ClientApiImpl::CreateFlowAudioRaw(std::list<std::shared_ptr<Flow>>& lstAdded, const Json::Value& jsData, const std::string& sIpAddress)
{
    auto pResource = std::make_shared<FlowAudioRaw>();
    if(pResource->UpdateFromJson(jsData))
    {
        m_flows.AddResource(sIpAddress, pResource);
        lstAdded.push_back(pResource);
        pmlLog(pml::LOG_INFO) << "NMOS: " << "FlowAudioRaw: " << pResource->GetId() << " found at " << sIpAddress ;
    }
    else
    {
        pmlLog(pml::LOG_INFO) << "NMOS: " << "Found Flow but json data incorrect: " << pResource->GetJsonParseError() ;
    }

}

void ClientApiImpl::CreateFlowVideo(std::list<std::shared_ptr<Flow>>& lstAdded, const Json::Value& jsData, const std::string& sIpAddress)
{
    if(jsData["media_type"].isString())
    {//Coded vidoe
        if(jsData["media_type"].asString() == "vidoe/raw")
        {
            CreateFlowVideoRaw(lstAdded, jsData, sIpAddress);
        }
        else
        {
            CreateFlowVideoCoded(lstAdded, jsData, sIpAddress);
        }
    }
}

void ClientApiImpl::CreateFlowVideoRaw(std::list<std::shared_ptr<Flow>>& lstAdded, const Json::Value& jsData, const std::string& sIpAddress)
{
    auto pResource = std::make_shared<FlowVideoRaw>();
    if(pResource->UpdateFromJson(jsData))
    {
        m_flows.AddResource(sIpAddress, pResource);
        lstAdded.push_back(pResource);
        pmlLog(pml::LOG_INFO) << "NMOS: " << "FlowVideoRaw: " << pResource->GetId() << " found at " << sIpAddress ;
    }
    else
    {
        pmlLog(pml::LOG_INFO) << "NMOS: " << "Found Flow but json data incorrect: " << pResource->GetJsonParseError() ;
    }
}

void ClientApiImpl::CreateFlowVideoCoded(std::list<std::shared_ptr<Flow>>& lstAdded, const Json::Value& jsData, const std::string& sIpAddress)
{
    auto pResource = std::make_shared<FlowVideoCoded>(jsData["media_type"].asString());
    if(pResource->UpdateFromJson(jsData))
    {
        m_flows.AddResource(sIpAddress, pResource);
        lstAdded.push_back(pResource);
        pmlLog(pml::LOG_INFO) << "NMOS: " << "FlowVideoCoded: " << pResource->GetId() << " found at " << sIpAddress ;
    }
    else
    {
        pmlLog(pml::LOG_INFO) << "NMOS: " << "Found Flow but json data incorrect: " << pResource->GetJsonParseError() ;
    }
}

void ClientApiImpl::CreateFlowData(std::list<std::shared_ptr<Flow>>& lstAdded, const Json::Value& jsData, const std::string& sIpAddress)
{
    if(jsData["media_type"] == "video/smpte291")
    {
        auto pResource = std::make_shared<FlowDataSdiAnc>();
        if(pResource->UpdateFromJson(jsData))
        {
            m_flows.AddResource(sIpAddress, pResource);
            lstAdded.push_back(pResource);
            pmlLog(pml::LOG_INFO) << "NMOS: " << "FlowDataSdiAnc: " << pResource->GetId() << " found at " << sIpAddress ;
        }
        else
        {
            pmlLog(pml::LOG_INFO) << "NMOS: " << "Found Flow but json data incorrect: " << pResource->GetJsonParseError() ;
        }
    }
    //SDIAncData only at the moment
}

void ClientApiImpl::CreateFlowMux(std::list<std::shared_ptr<Flow>>& lstAdded, const Json::Value& jsData, const std::string& sIpAddress)
{
    //Mux only at the momemnt
    auto pResource = std::make_shared<FlowMux>();
    if(pResource->UpdateFromJson(jsData))
    {
        m_flows.AddResource(sIpAddress, pResource);
        lstAdded.push_back(pResource);
        pmlLog(pml::LOG_INFO) << "NMOS: " << "FlowMux: " << pResource->GetId() << " found at " << sIpAddress ;
    }
    else
    {
        pmlLog(pml::LOG_INFO) << "NMOS: " << "Found Flow but json data incorrect: " << pResource->GetJsonParseError() ;
    }
}


void ClientApiImpl::AddSenders(std::list<std::shared_ptr<SenderBase> >& lstAdded, std::list<std::shared_ptr<SenderBase> >& lstUpdated, const std::string& sIpAddress, const std::string& sData)
{
    std::lock_guard<std::mutex> lg(m_mutex);
    Json::Value jsData = ConvertToJson(sData);
    if(jsData.isArray())
    {
        for(Json::ArrayIndex ai = 0; ai < jsData.size(); ++ai)
        {
            if(jsData[ai].isObject() && jsData[ai]["id"].isString())
            {
                auto pResource = m_senders.UpdateResource(jsData[ai]);
                if(!pResource)
                {
                    pResource = std::make_shared<SenderBase>();
                    if(pResource->UpdateFromJson(jsData[ai]))
                    {
                        m_senders.AddResource(sIpAddress, pResource);
                        lstAdded.push_back(pResource);
                        pmlLog(pml::LOG_INFO) << "NMOS: " << "Sender: " << pResource->GetId() << " found at " << sIpAddress ;
                    }
                    else
                    {
                        pmlLog(pml::LOG_INFO) << "NMOS: " << "Found Sender but json data incorrect: " << pResource->GetJsonParseError() ;
                    }
                }
                else
                {
                    lstUpdated.push_back(pResource);
                }
            }
            else
            {
                pmlLog(pml::LOG_INFO) << "NMOS: " << "Reply from " << sIpAddress << "but not JSON is not an array of objects" ;
            }
        }
    }
    else
    {
        pmlLog(pml::LOG_INFO) << "NMOS: " << "Reply from " << sIpAddress << "but not JSON is not array" ;
    }
}

void ClientApiImpl::AddReceivers(std::list<std::shared_ptr<ReceiverBase> >& lstAdded, std::list<std::shared_ptr<ReceiverBase> >& lstUpdated, const std::string& sIpAddress, const std::string& sData)
{
    std::lock_guard<std::mutex> lg(m_mutex);
    Json::Value jsData = ConvertToJson(sData);
    if(jsData.isArray())
    {
        for(Json::ArrayIndex ai = 0; ai < jsData.size(); ++ai)
        {
            if(jsData[ai].isObject() && jsData[ai]["id"].isString())
            {
                auto pResource = m_receivers.UpdateResource(jsData[ai]);
                if(!pResource)
                {
                    pResource = std::make_shared<ReceiverBase>();
                    if(pResource->UpdateFromJson(jsData[ai]))
                    {
                        m_receivers.AddResource(sIpAddress, pResource);
                        lstAdded.push_back(pResource);
                        pmlLog(pml::LOG_INFO) << "NMOS: " << "Receiver: " << pResource->GetId() << " found at " << sIpAddress ;
                    }
                    else
                    {
                        pmlLog(pml::LOG_INFO) << "NMOS: " << "Found Receiver but json data incorrect: " << pResource->GetJsonParseError() ;
                    }
                }
                else
                {
                    lstUpdated.push_back(pResource);
                }
            }
            else
            {
                pmlLog(pml::LOG_INFO) << "NMOS: " << "Reply from " << sIpAddress << "but not JSON is not an array of objects" ;
            }
        }
    }
    else
    {
        pmlLog(pml::LOG_INFO) << "NMOS: " << "Reply from " << sIpAddress << "but not JSON is not array" ;
    }
}


void ClientApiImpl::RemoveResources(const std::string& sIpAddress)
{
    std::list<std::shared_ptr<Self> > lstSelf;
    std::list<std::shared_ptr<Device> > lstDevice;
    std::list<std::shared_ptr<Source> > lstSource;
    std::list<std::shared_ptr<Flow> > lstFlow;
    std::list<std::shared_ptr<SenderBase> > lstSender;
    std::list<std::shared_ptr<ReceiverBase> > lstReceiver;

    m_nodes.RemoveResources(sIpAddress, lstSelf);
    m_devices.RemoveResources(sIpAddress, lstDevice);
    m_sources.RemoveResources(sIpAddress, lstSource);
    m_flows.RemoveResources(sIpAddress, lstFlow);
    m_senders.RemoveResources(sIpAddress, lstSender);
    m_receivers.RemoveResources(sIpAddress, lstReceiver);

    if(RunQuery(lstSelf, ClientApiImpl::NODES))
    {
        m_pPoster->_NodeChanged({}, {}, lstSelf);
    }

    if(RunQuery(lstDevice, ClientApiImpl::DEVICES))
    {
        m_pPoster->_DeviceChanged({}, {}, lstDevice);
    }

    if(RunQuery(lstSource, ClientApiImpl::SOURCES))
    {
        m_pPoster->_SourceChanged({}, {}, lstSource);
    }

    if(RunQuery(lstFlow, ClientApiImpl::FLOWS))
    {
        m_pPoster->_FlowChanged({}, {}, lstFlow);
    }

    if(RunQuery(lstSender, ClientApiImpl::SENDERS))
    {
        m_pPoster->_SenderChanged({}, {}, lstSender);
    }

    if(RunQuery(lstReceiver, ClientApiImpl::RECEIVERS))
    {
        m_pPoster->_ReceiverChanged({}, {}, lstReceiver);
    }
}

void ClientApiImpl::NodeDetailsDone()
{
    m_nCurlThreadCount--;
}


void ClientApiImpl::DeleteServiceBrowser()
{

}




const std::map<std::string, std::shared_ptr<Self> >& ClientApiImpl::GetNodes()
{
    return m_nodes.GetResources();
}


std::map<std::string, std::shared_ptr<Self> >::const_iterator ClientApiImpl::FindNode(const std::string& sUid)
{
    return m_nodes.FindNmosResource(sUid);
}

const std::map<std::string, std::shared_ptr<Device> >& ClientApiImpl::GetDevices()
{
    return m_devices.GetResources();
}


std::map<std::string, std::shared_ptr<Device> >::const_iterator ClientApiImpl::FindDevice(const std::string& sUid)
{
    return m_devices.FindNmosResource(sUid);
}


const std::map<std::string, std::shared_ptr<Source> >& ClientApiImpl::GetSources()
{
    return m_sources.GetResources();
}


std::map<std::string, std::shared_ptr<Source> >::const_iterator ClientApiImpl::FindSource(const std::string& sUid)
{
    return m_sources.FindNmosResource(sUid);
}


const std::map<std::string, std::shared_ptr<Flow> >& ClientApiImpl::GetFlows()
{
    return m_flows.GetResources();
}


std::map<std::string, std::shared_ptr<Flow> >::const_iterator ClientApiImpl::FindFlow(const std::string& sUid)
{
    return m_flows.FindNmosResource(sUid);
}



const std::map<std::string, std::shared_ptr<SenderBase> >& ClientApiImpl::GetSenders()
{
    return m_senders.GetResources();
}


std::map<std::string, std::shared_ptr<SenderBase> >::const_iterator ClientApiImpl::FindSender(const std::string& sUid)
{
    return m_senders.FindNmosResource(sUid);
}



const std::map<std::string, std::shared_ptr<ReceiverBase> >& ClientApiImpl::GetReceivers()
{
    return m_receivers.GetResources();
}

std::map<std::string, std::shared_ptr<ReceiverBase> >::const_iterator ClientApiImpl::FindReceiver(const std::string& sUid)
{
    return m_receivers.FindNmosResource(sUid);
}


bool ClientApiImpl::Subscribe(const std::string& sSenderId, const std::string& sReceiverId)
{
    std::lock_guard<std::mutex> lg(m_mutex);

    auto pSender = GetSender(sSenderId);
    auto pReceiver = GetReceiver(sReceiverId);
    if(!pSender || !pReceiver)
    {
        return false;
    }

    ApiVersion version(0,0);
    std::string sUrl(GetTargetUrl(pReceiver, version));
    if(sUrl.empty() == false)
    {
        //do a PUT to the correct place on the URL

        m_pCurl->PutPatch(sUrl, ConvertFromJson(pSender->GetJson(version)), ClientPoster::CURLTYPE_TARGET, true, pReceiver->GetId());
        pmlLog(pml::LOG_DEBUG) << "TARGET: " << sUrl ;
        return true;
    }
    pmlLog(pml::LOG_ERROR) << "NMOS: " << "Couldn't create target url" ;
    return false;
}

bool ClientApiImpl::Unsubscribe(const std::string& sReceiverId)
{
    std::lock_guard<std::mutex> lg(m_mutex);
    auto pReceiver = GetReceiver(sReceiverId);
    if(!pReceiver)
    {
        return false;
    }

    ApiVersion version(0,0);
    std::string sUrl(GetTargetUrl(pReceiver, version));
    if(sUrl.empty() == false)
    {
        pmlLog(pml::LOG_DEBUG) << "NMOS: " << "TARGET: " << sUrl ;
        //do a PUT to the correct place on the URL
        m_pCurl->PutPatch(sUrl, "{}", ClientPoster::CURLTYPE_TARGET, true, pReceiver->GetId());

        return true;
    }
    return false;
}

std::string ClientApiImpl::GetTargetUrl(std::shared_ptr<ReceiverBase> pReceiver, ApiVersion& version)
{
    //get the device id
    auto itDevice =  m_devices.FindNmosResource(pReceiver->GetParentResourceId());
    if(itDevice == m_devices.GetResources().end())
    {
        pmlLog(pml::LOG_ERROR) << "NMOS: " << "Device: " << pReceiver->GetParentResourceId() << " not found" ;
        return std::string();
    }

    auto itNode =  m_nodes.FindNmosResource(itDevice->second->GetParentResourceId());
    if(itNode == m_nodes.GetResources().end())
    {
        pmlLog(pml::LOG_ERROR) << "NMOS: " << "Node: " << itDevice->second->GetParentResourceId() << " not found" ;
        return std::string();
    }

    //decide on the version to use - for now get highest v1.x @todo deciding on version should possibly work in a user defined way
    version = ApiVersion(0,0);

    for(auto aVersion : itNode->second->GetApiVersions())
    {
        pmlLog(pml::LOG_DEBUG) << "NMOS: " << aVersion.GetVersionAsString() ;
        if(aVersion.GetMajor() == 1)
        {
            version = aVersion;
        }
        else if(aVersion.GetMajor() > 1)
        {
            break;
        }
    }
    if(version.GetMajor() == 0)
    {
        pmlLog(pml::LOG_ERROR) << "NMOS: " << "Version 1.x not found" ;
        return std::string();
    }
    //get the endpoint to use... for now the first non https one @todo deciding on endpoint should probably work in a better way
    auto itEndpoint = std::find_if(itNode->second->GetEndpointsBegin(), itNode->second->GetEndpointsEnd(), [](const ifendpoint& apoint) { return apoint.bSecure==false;});
//    set<ifendpoint>::const_iterator itEndpoint = itNode->second->GetEndpointsBegin();
//    for(; itEndpoint != itNode->second->GetEndpointsEnd(); ++itEndpoint)
//    {
//        if(itEndpoint->bSecure == false)
//        {
//            break;
//        }
//    }
    if(itEndpoint == itNode->second->GetEndpointsEnd())
    {
        pmlLog(pml::LOG_ERROR) << "NMOS: " << "Non-secure endpoint not found" ;
        return std::string();
    }

    //now we can build our url...
    std::stringstream ssurl;
    ssurl << itEndpoint->sHost << ":" << itEndpoint->nPort << "/x-nmos/node/" << version.GetVersionAsString() << "/receivers/" << pReceiver->GetId() << "/target";

    return ssurl.str();

}



std::string ClientApiImpl::GetConnectionUrlSingle(std::shared_ptr<Resource> pResource, const std::string& sDirection, const std::string& sEndpoint, ApiVersion& version)
{
    auto itDevice =  m_devices.FindNmosResource(pResource->GetParentResourceId());
    if(itDevice == m_devices.GetResources().end())
    {
        pmlLog(pml::LOG_ERROR) << "NMOS: " << "Device: " << pResource->GetParentResourceId() << " not found" ;
        return std::string();
    }

    for(auto itControl = itDevice->second->GetControlsBegin(); itControl != itDevice->second->GetControlsEnd(); ++itControl)
    {
        if("urn:x-nmos:control:sr-ctrl/v1.0" == itControl->first)
        {
            version = ApiVersion(1,0);
            std::stringstream ssUrl;
            ssUrl << itControl->second << "/single/" << sDirection << "/" << pResource->GetId() << "/" << sEndpoint;
            return ssUrl.str();
        }
    }
    version = ApiVersion(0,0);
    return std::string();
}

bool ClientApiImpl::RequestSender(const std::string& sSenderId, ClientPoster::enumCurlType eType)
{
    std::lock_guard<std::mutex> lg(m_mutex);
    auto pSender = GetSender(sSenderId);
    if(!pSender)
    {
        return false;
    }

    ApiVersion version(0,0);
    std::string sConnectionUrl(GetConnectionUrlSingle(pSender, "senders", ClientPoster::STR_TYPE[eType], version));
    if(sConnectionUrl.empty())
    {
        return false;
    }

    m_pCurl->Get(sConnectionUrl, (long)eType);
    return true;
}

bool ClientApiImpl::RequestReceiver(const std::string& sReceiverId, ClientPoster::enumCurlType eType)
{
    std::lock_guard<std::mutex> lg(m_mutex);
    auto pReceiver = GetReceiver(sReceiverId);
    if(!pReceiver)
    {
        return false;
    }

    ApiVersion version(0,0);
    std::string sConnectionUrl(GetConnectionUrlSingle(pReceiver, "receivers", ClientPoster::STR_TYPE[eType], version));
    if(sConnectionUrl.empty())
    {
        return false;
    }

    m_pCurl->Get(sConnectionUrl, (long)eType);
    return true;
}

bool ClientApiImpl::RequestSenderStaged(const std::string& sSenderId)
{
    return RequestSender(sSenderId, ClientPoster::CURLTYPE_SENDER_STAGED);
}

bool ClientApiImpl::RequestSenderActive(const std::string& sSenderId)
{
    return RequestSender(sSenderId, ClientPoster::CURLTYPE_SENDER_ACTIVE);
}

bool ClientApiImpl::RequestSenderTransportFile(const std::string& sSenderId)
{
    return RequestSender(sSenderId, ClientPoster::CURLTYPE_SENDER_TRANSPORTFILE);
}

bool ClientApiImpl::RequestReceiverStaged(const std::string& sReceiverId)
{
    return RequestReceiver(sReceiverId, ClientPoster::CURLTYPE_RECEIVER_STAGED);
}

bool ClientApiImpl::RequestReceiverActive(const std::string& sReceiverId)
{
    return RequestReceiver(sReceiverId, ClientPoster::CURLTYPE_RECEIVER_ACTIVE);
}


bool ClientApiImpl::PatchSenderStaged(const std::string& sSenderId, const connectionSender& aConnection)
{
    std::lock_guard<std::mutex> lg(m_mutex);
    auto pSender = GetSender(sSenderId);
    if(!pSender)
    {
        return false;
    }

    ApiVersion version(0,0);
    std::string sConnectionUrl(GetConnectionUrlSingle(pSender, "senders", ClientPoster::STR_TYPE[ClientPoster::CURLTYPE_SENDER_STAGED], version));
    if(sConnectionUrl.empty())
    {
        return false;
    }

    m_pCurl->PutPatch(sConnectionUrl, ConvertFromJson(aConnection.GetJson(version)), ClientPoster::CURLTYPE_SENDER_PATCH, false, sSenderId);

    return true;
}

bool ClientApiImpl::PatchReceiverStaged(const std::string& sReceiverId, const connectionReceiver& aConnection)
{
    std::lock_guard<std::mutex> lg(m_mutex);
    auto pReceiver = GetReceiver(sReceiverId);
    if(!pReceiver)
    {
        return false;
    }

    ApiVersion version(0,0);
    std::string sConnectionUrl(GetConnectionUrlSingle(pReceiver, "receivers", ClientPoster::STR_TYPE[ClientPoster::CURLTYPE_SENDER_STAGED], version));
    if(sConnectionUrl.empty())
    {
        return false;
    }

    m_pCurl->PutPatch(sConnectionUrl, ConvertFromJson(aConnection.GetJson(version)), ClientPoster::CURLTYPE_RECEIVER_PATCH, false, sReceiverId);

    return true;
}


std::shared_ptr<SenderBase> ClientApiImpl::GetSender(const std::string& sSenderId)
{
    auto itSender = m_senders.FindNmosResource(sSenderId);
    if(itSender == m_senders.GetResources().end())
    {
        pmlLog(pml::LOG_ERROR) << "NMOS: " << "Sender: " << sSenderId << " not found." ;
        return 0;
    }
    return itSender->second;
}

std::shared_ptr<ReceiverBase> ClientApiImpl::GetReceiver(const std::string& sReceiverId)
{
    auto itReceiver = m_receivers.FindNmosResource(sReceiverId);
    if(itReceiver == m_receivers.GetResources().end())
    {
        pmlLog(pml::LOG_ERROR) << "NMOS: " << "Receiver: " << sReceiverId << " not found." ;
        return 0;
    }
    return itReceiver->second;
}


void ClientApiImpl::StoreDevices(const std::string& sIpAddress)
{
    m_devices.StoreResources(sIpAddress);
}

void ClientApiImpl::StoreSources(const std::string& sIpAddress)
{
    m_sources.StoreResources(sIpAddress);
}

void ClientApiImpl::StoreFlows(const std::string& sIpAddress)
{
    m_flows.StoreResources(sIpAddress);
}

void ClientApiImpl::StoreSenders(const std::string& sIpAddress)
{
    m_senders.StoreResources(sIpAddress);
}

void ClientApiImpl::StoreReceivers(const std::string& sIpAddress)
{
    m_receivers.StoreResources(sIpAddress);
}

void ClientApiImpl::RemoveStaleDevices(std::list<std::shared_ptr<Device> >& lstRemoved)
{
    m_devices.RemoveStaleResources(lstRemoved);

}

void ClientApiImpl::RemoveStaleSources(std::list<std::shared_ptr<Source> >& lstRemoved)
{
    m_sources.RemoveStaleResources(lstRemoved);
}

void ClientApiImpl::RemoveStaleFlows(std::list<std::shared_ptr<Flow> >& lstRemoved)
{
    m_flows.RemoveStaleResources(lstRemoved);
}

void ClientApiImpl::RemoveStaleSenders(std::list<std::shared_ptr<SenderBase> >& lstRemoved)
{
    m_senders.RemoveStaleResources(lstRemoved);
}

void ClientApiImpl::RemoveStaleReceivers(std::list<std::shared_ptr<ReceiverBase> >& lstRemoved)
{
    m_receivers.RemoveStaleResources(lstRemoved);
}

bool ClientApiImpl::Connect(const std::string& sSenderId, const std::string& sReceiverId)
{
    std::lock_guard<std::mutex> lg(m_mutex);
    auto pSender = GetSender(sSenderId);
    auto pReceiver = GetReceiver(sReceiverId);
    if(!pSender || !pReceiver)
    {
        return false;
    }

    ApiVersion version(0,0);
    std::string sSenderStageUrl(GetConnectionUrlSingle(pSender, "senders", ClientPoster::STR_TYPE[ClientPoster::CURLTYPE_SENDER_STAGED], version));
    std::string sSenderTransportUrl(GetConnectionUrlSingle(pSender, "senders", ClientPoster::STR_TYPE[ClientPoster::CURLTYPE_SENDER_TRANSPORTFILE], version));
    std::string sReceiverStageUrl(GetConnectionUrlSingle(pReceiver, "receivers", ClientPoster::STR_TYPE[ClientPoster::CURLTYPE_RECEIVER_STAGED], version));


    if(sSenderStageUrl.empty() || sSenderTransportUrl.empty() || sReceiverStageUrl.empty())
    {
        return false;
    }

    std::thread th(ConnectThread, this, sSenderId, sReceiverId, sSenderStageUrl, sSenderTransportUrl, sReceiverStageUrl);
    th.detach();
    return true;
}



bool ClientApiImpl::Disconnect( const std::string& sReceiverId)
{
    std::lock_guard<std::mutex> lg(m_mutex);
    pmlLog(pml::LOG_DEBUG) << "NMOS: " << "ClientApiImpl::Disconnect " << sReceiverId ;
    auto pReceiver = GetReceiver(sReceiverId);
    if(!pReceiver)
    {
        pmlLog(pml::LOG_DEBUG) << "NMOS: " << "ClientApiImpl::Disconnect No Receiver" ;
        return false;
    }
    auto pSender = GetSender(pReceiver->GetSender());
    if(!pSender)
    {
        pmlLog(pml::LOG_DEBUG) << "NMOS: " << "ClientApiImpl::Disconnect No Sender" ;
        return false;
    }

    ApiVersion version(0,0);
    std::string sSenderStageUrl(GetConnectionUrlSingle(pSender, "senders", ClientPoster::STR_TYPE[ClientPoster::CURLTYPE_SENDER_STAGED], version));
    std::string sSenderTransportUrl(GetConnectionUrlSingle(pSender, "senders", ClientPoster::STR_TYPE[ClientPoster::CURLTYPE_SENDER_TRANSPORTFILE], version));
    std::string sReceiverStageUrl(GetConnectionUrlSingle(pReceiver, "receivers", ClientPoster::STR_TYPE[ClientPoster::CURLTYPE_RECEIVER_STAGED], version));


    if(sSenderStageUrl.empty() || sSenderTransportUrl.empty() || sReceiverStageUrl.empty())
    {
        pmlLog(pml::LOG_DEBUG) << "NMOS: " << "ClientApiImpl::Disconnect No URLS" ;
        return false;
    }

    std::thread th(DisconnectThread, this, pSender->GetId(), sReceiverId, sSenderStageUrl, sSenderTransportUrl, sReceiverStageUrl);
    th.detach();
    return true;
}


bool ClientApiImpl::AddQuerySubscription(int nResource, const std::string& sQuery, unsigned long nUpdateRate)
{
    bool bSuccess(false);
    switch(m_eMode)
    {
        case MODE_P2P:
            bSuccess = AddQuerySubscriptionP2P(nResource, sQuery);
            break;
        case MODE_REGISTRY:
            bSuccess = AddQuerySubscriptionRegistry(nResource, sQuery, nUpdateRate);
            break;

    }
    return bSuccess;
}

bool ClientApiImpl::RemoveQuerySubscription(const std::string& sSubscriptionId)
{
    bool bSuccess(false);
    switch(m_eMode)
    {
        case MODE_P2P:
            bSuccess = RemoveQuerySubscriptionP2P(sSubscriptionId);
            break;
        case MODE_REGISTRY:
            bSuccess = RemoveQuerySubscriptionRegistry(sSubscriptionId);
            break;

    }
    return bSuccess;
}


bool ClientApiImpl::AddQuerySubscriptionRegistry(int nResource, const std::string& sQuery, unsigned long nUpdateRate)
{
    //@todo
    return false;
}

bool ClientApiImpl::RemoveQuerySubscriptionRegistry(const std::string& sSubscriptionId)
{
    // @todo ClientApiImpl::RemoveQuerySubscriptionRegistry
    return false;
}

bool ClientApiImpl::AddQuerySubscriptionP2P(int nResource, const std::string& sQuery)
{
    //store the query
    query aQuery;
    aQuery.sId = CreateGuid();
    aQuery.sQuery = sQuery;
    aQuery.nResource = nResource;
    m_mmQuery.insert(std::make_pair(nResource, aQuery));

    //call the poster to let the client know the id
    if(m_pPoster)
    {
        m_pPoster->_QuerySubscription(aQuery.sId, nResource, sQuery);
    }

    //run the query on our stored nodes and call the poster for each found query
    switch(nResource)
    {
    case NODES:
        {
            std::list<std::shared_ptr<Self> > lstResources;
            m_nodes.GetResourcesAsList(lstResources);
            if(RunQuery(lstResources, sQuery))
            {
                m_pPoster->_NodeChanged(lstResources, {}, {});
            }
        }
        break;
    case DEVICES:
        {
            std::list<std::shared_ptr<Device> > lstResources;
            m_devices.GetResourcesAsList(lstResources);
            if(RunQuery(lstResources, sQuery))
            {
                m_pPoster->_DeviceChanged(lstResources, {}, {});
            }
        }
        break;
    case SOURCES:
        {
            std::list<std::shared_ptr<Source> > lstResources;
            m_sources.GetResourcesAsList(lstResources);
            if(RunQuery(lstResources, sQuery))
            {
                m_pPoster->_SourceChanged(lstResources, {}, {});
            }
        }
        break;
    case FLOWS:
        {
            std::list<std::shared_ptr<Flow> > lstResources;
            m_flows.GetResourcesAsList(lstResources);
            if(RunQuery(lstResources, sQuery))
            {
                m_pPoster->_FlowChanged(lstResources, {}, {});
            }
        }
        break;
    case SENDERS:
        {
            std::list<std::shared_ptr<SenderBase> > lstResources;
            m_senders.GetResourcesAsList(lstResources);
            if(RunQuery(lstResources, sQuery))
            {
                m_pPoster->_SenderChanged(lstResources, {}, {});
            }
        }
        break;
    case RECEIVERS:
        {
            std::list<std::shared_ptr<ReceiverBase> > lstResources;
            m_receivers.GetResourcesAsList(lstResources);
            if(RunQuery(lstResources, sQuery))
            {
                m_pPoster->_ReceiverChanged(lstResources, {}, {});
            }
        }
        break;
    }
    return true;
}

bool ClientApiImpl::RemoveQuerySubscriptionP2P(const std::string& sSubscriptionId)
{
    for(auto& pairQuery : m_mmQuery)
    {
        if(pairQuery.second.sId == sSubscriptionId)
        {
            if(m_pPoster)
            {
                m_pPoster->_QuerySubscriptionRemoved(sSubscriptionId);
                m_mmQuery.erase(pairQuery.first);
                return true;
            }
        }
    }
    return false;
}

template<class T> bool ClientApiImpl::RunQuery(std::list<std::shared_ptr<T> >& lstCheck, const std::string& sQuery)
{
    if(m_pPoster)
    {
        for(auto itResource = lstCheck.begin(); itResource != lstCheck.end(); )
        {
            if(MeetsQuery(sQuery, (*itResource)))
            {
                ++itResource;
            }
            else
            {
                itResource = lstCheck.erase(itResource);
            }
        }
        return (lstCheck.empty() == false);
    }
    else
    {
        pmlLog(pml::LOG_DEBUG) << "NMOS: " << "ClientApiImpl::RunQuery - No poster" ;
    }
    return false;
}

template<class T> bool ClientApiImpl::RunQuery(std::list<std::shared_ptr<T> >& lstCheck, int nResource)
{
    if(lstCheck.empty())
    {
        pmlLog(pml::LOG_DEBUG) << "NMOS: " << "ClientApiImpl::RunQuery - lstcheck empty" ;
    }

    if(m_mmQuery.empty())
    {
        pmlLog(pml::LOG_DEBUG) << "NMOS: " << "ClientApiImpl::RunQuery - m_mmQuery empty" ;
    }

    for(auto itResource = lstCheck.begin(); itResource != lstCheck.end(); )
    {
        bool bKeep(false);
        for(auto pairQuery : m_mmQuery)
        {
            if((pairQuery.first & nResource))
            {
                if(MeetsQuery(pairQuery.second.sQuery, (*itResource)))
                {
                    bKeep = true;
                    break;
                }
            }
        }

        if(bKeep)
        {
            ++itResource;
        }
        else
        {
            itResource = lstCheck.erase(itResource);
        }
    }
    return (lstCheck.empty() == false);
}

template<class T> bool ClientApiImpl::RunQuery(std::list<std::shared_ptr<T> >& lstAdded, std::list<std::shared_ptr<T> >& lstUpdated, std::list<std::shared_ptr<T> >& lstRemoved, int nResourceType)
{
    if(m_pPoster)
    {
        return (RunQuery(lstAdded, nResourceType) || RunQuery(lstUpdated, nResourceType) || RunQuery(lstRemoved, nResourceType));
    }
    return false;
}

bool ClientApiImpl::MeetsQuery(const std::string& sQuery, std::shared_ptr<Resource> pResource)
{
    return true;

    //if(sQuery.empty())
   // {
    //    return true;
    //}
    // @todo run the query properly
    //return false;9999
}

void ClientApiImpl::SubscribeToQueryServer()
{
    if(m_mmQueryNodes.empty() == false)
    {
        //@todo ClientApiImpl::SubscribeToQueryServer Allow other versions than 1.2
        std::stringstream ssUrl;
        ssUrl <<  m_mmQueryNodes.begin()->second->sHostIP << ":" << m_mmQueryNodes.begin()->second->nPort << "/x-nmos/query/v1.2/subscriptions";

        pmlLog(pml::LOG_DEBUG) << "NMOS: " << "QUERY URL: " << ssUrl.str() ;

        //we run through all our queries and post them to the query server asking for the websocket
        for(auto pairQuery : m_mmQuery)
        {
            //create our json query
            Json::Value jsQuery;
            jsQuery["max_update_rate_ms"] = (int)pairQuery.second.nRefreshRate;
            jsQuery["resource_path"] = "/" + STR_RESOURCE[pairQuery.second.nResource] + "s";
            jsQuery["persist"] = false;
            jsQuery["secure"] = false;
            jsQuery["authorization"] = false;
            //@todo params
            Json::Value jsParams(Json::objectValue);
            jsQuery["params"] = jsParams;

            auto curlresp = CurlRegister::Post(ssUrl.str(), ConvertFromJson(jsQuery));

            HandleQuerySubscriptionResponse(curlResp.nCode, ConvertToJson(curlResponse.sResponse));

        }
    }
}

void ClientApiImpl::HandleQuerySubscriptionResponse(unsigned short nCode, const Json::Value& jsResponse)
{
    if(nCode == 200 || nCode == 201)
    {
        HandleSuccessfulQuerySubscription(jsResponse);
    }
    else if(nCode == 400)
    {
        pmlLog(pml::LOG_ERROR) << "NMOS: Could not subscribe to query server " << jsResponse["error"].asString();
    }
    else if(nCode == 501)
    {
        pmlLog(pml::LOG_ERROR) << "NMOS: Query server does not suppory query " << jsResponse["error"].asString();
    }
    else
    {
        pmlLog(pml::LOG_ERROR) << "NMOS: Unknown error attempting to subscribe to query server";
    }
}

void ClientApiImpl::HandleSuccessfulQuerySubscription(const Json::Value& jsResponse)
{
    pmlLog(pml::LOG_DEBUG) << "NMOS: Successful query subscription  - now create websocket connection";
    /*create a websocket pointing at ws_ref and store it wht the id. Remember all the details
    id
    ws_href
            max_update_rate_ms
            params
            persist
            secure
            resource_path

    */

}

void ClientApiImpl::AddBrowseServices()
{
    for(const auto& pairBrowser : m_mBrowser)
    {
        pairBrowser.second->AddService("_nmos-node._tcp", m_pClientZCPoster);
        pairBrowser.second->AddService("_nmos-query._tcp",m_pClientZCPoster);
    }
}

void ClientApiImpl::StartBrowsers()
{
    for(const auto& pairBrowser : m_mBrowser)
    {
        pairBrowser.second->StartBrowser();
    }
}

bool ClientApiImpl::AddBrowseDomain(const std::string& sDomain)
{
    if(!m_bRun)
    {
        return m_mBrowser.insert(std::make_pair(sDomain, std::make_unique<ServiceBrowser>(sDomain))).second;
    }
    return false;
}
bool ClientApiImpl::RemoveBrowseDomain(const std::string& sDomain)
{
    if(!m_bRun)
    {
        m_mBrowser.erase(sDomain);
        return true;
    }
    return false;
}
