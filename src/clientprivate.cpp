#include "clientprivate.h"
#include "clientapi.h"
#include "log.h"
#include "mdns.h"
#include <thread>
#include <set>
#ifdef __GNU__
#include <unistd.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#endif // __GNU__
#include "curlregister.h"
#include "dnssd.h"
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
#include "flowdatajson.h"
#include "flowmux.h"
#include "senderbase.h"
#include "receiverbase.h"
#include "clientapiposter.h"
#include "utils.h"
#include <algorithm>
#include <numeric>
#include "websocketclient.h"
#include "clientposter.h"
#include "activation.h"
#include "threadpool.h"

using namespace pml::nmos;
using namespace std::placeholders;


const std::array<std::string, 6> ClientApiImpl::STR_RESOURCE = {"node", "device", "source", "flow", "sender", "receiver"};
const std::array<std::string, 7> ClientApiImpl::STR_CONNECTION = {"staged", "active", "transportfile","constraints", "staged", "active", "constraint"};

std::vector<Constraints> CreateConstraints(const Json::Value& jsData)
{
    std::vector<Constraints> vConstraints;
    if(jsData.isArray() != false)
    {
        for(Json::ArrayIndex i = 0; i < jsData.size(); i++)
        {
            if(jsData[i].isObject())
            {
                Constraints con;
                if(con.UpdateFromJson(jsData[i]))
                {
                    vConstraints.push_back(con);
                }
            }
        }
    }
    return vConstraints;
}


bool CheckSubnet(const url& address1, const url& address2, unsigned long  nSubnet)
{
    in_addr addr1, addr2;
    if(inet_aton(address1.Get().c_str(), &addr1) != 0 && inet_aton(address2.Get().c_str(), &addr2) != 0)
    {
        return ((addr1.s_addr & nSubnet) == (addr2.s_addr & nSubnet));
    }
    return false;
}

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
bool VersionChanged(std::shared_ptr<pml::dnssd::dnsInstance> pInstance, const std::string& sVersion)
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
static void NodeBrowser(ClientApiImpl* pApi, std::shared_ptr<pml::dnssd::dnsInstance> pInstance)
{
    pmlLog() << "NODEBROWSER";
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
                    auto changed = pApi->AddNode(pInstance->sHostIP, ConvertToJson(curlresp.sResponse));

                    if(pApi->RunQuery(changed, ClientApiImpl::NODES))
                    {
                        if(pApi->GetPoster())
                        {
                            pApi->GetPoster()->_NodeChanged(changed);
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

                    //store the devices this ip address currently provides
                    pApi->StoreDevices(pInstance->sHostIP);
                    //add or update all devices on this ip address
                    auto changed = pApi->AddDevices(pInstance->sHostIP, ConvertToJson(curlresp.sResponse));
                    //remove any devices that we previously stored but didn;t update. i.e. ones that no longer exist
                    changed += pApi->RemoveStaleDevices();

                    if(pApi->RunQuery(changed, ClientApiImpl::DEVICES))
                    {
                        if(pApi->GetPoster())
                        {
                            pApi->GetPoster()->_DeviceChanged(changed);
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

                    pApi->StoreSources(pInstance->sHostIP);
                    auto changed = pApi->AddSources(pInstance->sHostIP, ConvertToJson(curlresp.sResponse));
                    changed += pApi->RemoveStaleSources();

                    if(pApi->RunQuery(changed, ClientApiImpl::SOURCES))
                    {
                        pmlLog() << "AddSources: " << changed.lstAdded.size() << ", " << changed.lstUpdated.size();
                        if(pApi->GetPoster())
                        {
                            pApi->GetPoster()->_SourceChanged(changed);
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


                    pApi->StoreFlows(pInstance->sHostIP);
                    auto changed = pApi->AddFlows(pInstance->sHostIP, ConvertToJson(curlresp.sResponse));
                    changed += pApi->RemoveStaleFlows();
                    pmlLog() << "Flows: " << changed.lstAdded.size() << "," << changed.lstUpdated.size() << "," << changed.lstRemoved.size();
                    if(pApi->RunQuery(changed, ClientApiImpl::FLOWS))
                    {
                        pmlLog() << "Flows: " << changed.lstAdded.size() << "," << changed.lstUpdated.size() << "," << changed.lstRemoved.size();
                        if(pApi->GetPoster())
                        {
                            pApi->GetPoster()->_FlowChanged(changed);
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


                    pApi->StoreSenders(pInstance->sHostIP);
                    auto changed = pApi->AddSenders( pInstance->sHostIP, ConvertToJson(curlresp.sResponse));
                    changed += pApi->RemoveStaleSenders();

                    if(pApi->RunQuery(changed, ClientApiImpl::SENDERS))
                    {
                        if(pApi->GetPoster())
                        {
                            pApi->GetPoster()->_SenderChanged(changed);
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

                    pApi->StoreReceivers(pInstance->sHostIP);
                    auto changed = pApi->AddReceivers(pInstance->sHostIP, ConvertToJson(curlresp.sResponse));
                    changed += pApi->RemoveStaleReceivers();

                    if(pApi->RunQuery(changed, ClientApiImpl::RECEIVERS))
                    {
                        if(pApi->GetPoster())
                        {
                            pApi->GetPoster()->_ReceiverChanged(changed);
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
void ConnectThread(ClientApiImpl* pApi, const std::string& sSenderId, const std::string& sReceiverId, const std::string& sSenderUrl, const std::string& sReceiverUrl)
{
    // @todo ConnectThread - if a unicast stream then tell the sender where it should be sending stuff
    // @todo ConnectTrhead = multiple versions of connecting

    //get the constraints

    auto curlResp = CurlRegister::Get(sSenderUrl + "/single/senders/"+sSenderId+"/"+ClientApiImpl::STR_CONNECTION[enumConnection::SENDER_CONSTRAINTS], true);
    if(curlResp.nCode != 200)
    {
        pmlLog(pml::LOG_WARN) << "NMOS: Connect - failed to get sender " << sSenderId << " constraints";
        pApi->HandleConnect(sSenderId, sReceiverId, false, curlResp.sResponse);
    }
    auto vSenderConstraints = CreateConstraints(ConvertToJson(curlResp.sResponse));
    if(vSenderConstraints.empty())
    {
        pmlLog(pml::LOG_WARN) << "NMOS: Connect - failed to parse sender " << sSenderId << " constraints";
        pApi->HandleConnect(sSenderId, sReceiverId, false, curlResp.sResponse);
    }

    curlResp = CurlRegister::Get(sReceiverUrl + "/single/receivers/"+sReceiverId+"/"+ClientApiImpl::STR_CONNECTION[enumConnection::RECEIVER_CONSTRAINTS], true);
    if(curlResp.nCode != 200)
    {
        pmlLog(pml::LOG_WARN) << "NMOS: Connect - failed to get receiver " << sReceiverId << " constraints";
        pApi->HandleConnect(sSenderId, sReceiverId, false, curlResp.sResponse);
    }
    auto vReceiverConstraints = CreateConstraints(ConvertToJson(curlResp.sResponse));
    if(vReceiverConstraints.empty())
    {
        pmlLog(pml::LOG_WARN) << "NMOS: Connect - failed to parse receiver " << sReceiverId << " constraints";
        pApi->HandleConnect(sSenderId, sReceiverId, false, curlResp.sResponse);
    }

    //@todo check that what we want to do will meet the constraints

    connectionSender<activationRequest> aCon(true, (TransportParamsRTP::flagsTP)(TransportParamsRTP::CORE | (vSenderConstraints.size()>1 ? TransportParamsRTP::REDUNDANT : 0)));
    aCon.GetActivation().SetMode(activation::ACT_NOW);

    for(size_t i = 1; i < vSenderConstraints.size(); i++)
    {
        aCon.EnableTransport(i, (i < vReceiverConstraints.size()));
    }


    auto sSenderStageUrl = sSenderUrl + "/single/senders/"+sSenderId+"/"+ClientApiImpl::STR_CONNECTION[enumConnection::SENDER_STAGED];
    auto sSenderTransportUrl = sSenderUrl + "/single/senders/"+sSenderId+"/"+ClientApiImpl::STR_CONNECTION[enumConnection::SENDER_TRANSPORTFILE];
    auto sReceiverStageUrl = sReceiverUrl + "/single/receivers/"+sReceiverId+"/"+ClientApiImpl::STR_CONNECTION[enumConnection::RECEIVER_STAGED];

    curlResp = CurlRegister::PutPatch(sSenderStageUrl, ConvertFromJson(aCon.GetJson()), false, "");
    if(curlResp.nCode != 200)
    {
        pmlLog(pml::LOG_WARN) << "NMOS: ConnectThread Patch Sender failed: " << curlResp.nCode << " " << curlResp.sResponse;
        pApi->HandleConnect(sSenderId, sReceiverId, false, curlResp.sResponse);
    }
    else
    {
        curlResp = CurlRegister::Get(sSenderTransportUrl, false);
        if(curlResp.nCode != 200)
        {
            pmlLog(pml::LOG_WARN) << "NMOS: ConnectThread Get SDP failed: " << curlResp.nCode << " " << curlResp.sResponse;
            pApi->HandleConnect(sSenderId, sReceiverId, false, curlResp.sResponse);
        }
        else
        {
            connectionReceiver<activationResponse> aConR(true, (TransportParamsRTP::flagsTP)(TransportParamsRTP::CORE | (vReceiverConstraints.size()>1 ? TransportParamsRTP::REDUNDANT : 0)));
            aConR.GetActivation().SetMode(activation::ACT_NOW);
            aConR.SetSenderId(sSenderId);
            for(size_t i = 1; i < vSenderConstraints.size(); i++)
            {
                aConR.EnableTransport(i, (i < vReceiverConstraints.size()));
            }

            aConR.SetTransportFile(std::string("application/sdp"), curlResp.sResponse);

            std::string sData(ConvertFromJson(aConR.GetJson()));
            curlResp = CurlRegister::PutPatch(sReceiverStageUrl, sData, false, "");
            if(curlResp.nCode != 200)
            {
                pmlLog(pml::LOG_WARN) << "NMOS: ConnectThread Patch Receiver failed: " << curlResp.nCode << " " << curlResp.sResponse;
                pApi->HandleConnect(sSenderId, sReceiverId, false, curlResp.sResponse);
            }
            else
            {
                pApi->HandleConnect(sSenderId, sReceiverId, true, curlResp.sResponse);
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
        connectionSender<activationRequest> aCon(true, TransportParamsRTP::CORE);
        aCon.GetActivation().SetMode(activation::enumActivate::ACT_NOW);

        auto curlresp = CurlRegister::PutPatch(sSenderStage, ConvertFromJson(aCon.GetJson()), false, "");
        if(curlresp.nCode != 200)
        {
            pApi->HandleConnect(sSenderId, sReceiverId, false, curlresp.sResponse);
            return;
        }
    }

    connectionReceiver<activationResponse> aConR(true, TransportParamsRTP::CORE);
    aConR.GetActivation().SetMode(activation::enumActivate::ACT_NOW);
    aConR.SetSenderId({});

//    aConR.bMasterEnable = false;
//    aConR.tpReceivers[0].bRtpEnabled = false;   //@todo need the other stream as well I guess
//    aConR.sTransportFileType = "application/sdp";
//    aConR.sTransportFileData = "";

    auto curlresp = CurlRegister::PutPatch(sReceiverStage, ConvertFromJson(aConR.GetJson()), false, "");
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
        m_pWebSocket->Run();
    }
}

void ClientApiImpl::Stop()
{
    StopRun();
}

ClientApiImpl::ClientApiImpl() :
    m_version(1,1),
    m_eMode(MODE_P2P),
    m_bRun(true),
    m_pInstance(0),
    m_nCurlThreadCount(0),
    m_pPoster(0),
    m_pClientZCPoster(std::make_shared<ClientZCPoster>(this)),
    m_pCurl(new CurlRegister(std::bind(&ClientApiImpl::SetCurlDone, this, _1,_2,_3))),
    m_pThread(nullptr),
    m_bStarted(false),
    m_bDoneQueryBrowse(false),
    m_pWebSocket(std::make_unique<WebSocketClient>(std::bind(&ClientApiImpl::WebsocketConnected, this, _1), std::bind(&ClientApiImpl::WebsocketMessage, this, _1, _2)))
{
    m_mBrowser.insert(std::make_pair("local", std::make_unique<pml::dnssd::Browser>()));

    m_mGrainUpdate = { {"/nodes/", std::bind(&ClientApiImpl::GrainUpdateNode, this, _1, _2)},
                        {"/devices/", std::bind(&ClientApiImpl::GrainUpdateDevice, this, _1, _2)},
                        {"/sources/", std::bind(&ClientApiImpl::GrainUpdateSource, this, _1, _2)},
                        {"/flows/", std::bind(&ClientApiImpl::GrainUpdateFlow, this, _1, _2)},
                        {"/senders/", std::bind(&ClientApiImpl::GrainUpdateSender, this, _1, _2)},
                        {"/receivers/", std::bind(&ClientApiImpl::GrainUpdateReceiver, this, _1, _2)} };

    GetSubnetMasks();
    for(auto pairMask : m_mSubnetMasks)
    {
        pmlLog() << pairMask.first << "=" << pairMask.second;
    }
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

        m_pWebSocket->Stop();
    }

}


ClientApiImpl::enumSignal ClientApiImpl::GetSignal()
{
    std::lock_guard<std::mutex> lg(m_mutex);
    return m_eSignal;
}


void ClientApiImpl::SetCurlDone(const curlResponse& resp, unsigned long nType, const std::string& sResourceId)
{
    m_mutex.lock();
    m_asyncResponse = resp;
    m_nCurlType = nType;
    m_eSignal = CLIENT_SIG_CURL_DONE;
    m_sCurlResourceId = sResourceId;
    m_mutex.unlock();
    m_cvBrowse.notify_one();
}

void ClientApiImpl::SetInstanceResolved(std::shared_ptr<pml::dnssd::dnsInstance> pInstance)
{
    m_mutex.lock();
    m_pInstance = pInstance;
    m_eSignal = CLIENT_SIG_INSTANCE_RESOLVED;
    m_mutex.unlock();
    m_cvBrowse.notify_one();
}

void ClientApiImpl::SetInstanceRemoved(std::shared_ptr<pml::dnssd::dnsInstance> pInstance)
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
                    int nPriority = std::stoi(itTxt->second);
                    m_mmQueryNodes.insert(std::make_pair(nPriority, m_pInstance));
                    if(m_pPoster)
                    {
                        m_pPoster->_QueryServerFound(m_pInstance->sHostIP, nPriority);
                    }

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
        m_pInstance = nullptr;
    }
}

void ClientApiImpl::HandleInstanceRemoved()
{
    if(m_pInstance)
    {
        if(m_pInstance->sService == "_nmos_query._tcp")
        {
            pmlLog(pml::LOG_INFO) << "NMOS: " << "Query node: " << m_pInstance->sName << "removed" ;

            bool bSubscribeAgain(true);

            for(auto itRemove = m_mmQueryNodes.begin(); itRemove != m_mmQueryNodes.end(); ++itRemove)
            {
                if(itRemove->second == m_pInstance)
                {
                    m_mmQueryNodes.erase(itRemove);
                    break;
                }
                else
                {   //not the first one so don't need to resubscribe
                    bSubscribeAgain = false;
                }
            }

            if(m_pPoster)
            {
                m_pPoster->_QueryServerRemoved(m_pInstance->sHostIP);
            }

            if(m_eMode == MODE_REGISTRY)
            {
                if(m_mmQueryNodes.empty())
                {
                    m_eMode = MODE_P2P;
                    if(m_pPoster)
                    {
                        m_pPoster->_ModeChanged(false);
                    }
                }
                else if(bSubscribeAgain)
                {
                    SubscribeToQueryServer();
                }
            }
        }
        else if(m_pInstance->sService == "_nmos-node._tcp" && m_eMode == MODE_P2P)
        {
            pmlLog(pml::LOG_INFO) << "NMOS: " << "node Removed: " << m_pInstance->sName ;
            m_lstResolve.remove_if([this](std::shared_ptr<pml::dnssd::dnsInstance> pInstance) { return pInstance == m_pInstance;});
            RemoveResources(m_pInstance->sHostIP);
        }
        m_pInstance = nullptr;
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
        switch(static_cast<enumConnection>(m_nCurlType))
        {
            case enumConnection::TARGET:
                HandleCurlDoneTarget();
                break;
            case enumConnection::SENDER_STAGED:
                HandleCurlDoneGetSenderStaged();
                break;
            case enumConnection::SENDER_ACTIVE:
                HandleCurlDoneGetSenderActive();
                break;
            case enumConnection::SENDER_TRANSPORTFILE:
                HandlCurlDoneGetSenderTransportFile();
                break;
            case enumConnection::SENDER_CONSTRAINTS:
                HandleCurlDoneGetSenderConstraints();
                break;
            case enumConnection::RECEIVER_STAGED:
                HandleCurlDoneGetReceiverStaged();
                break;
            case enumConnection::RECEIVER_ACTIVE:
                void HandleCurlDoneGetReceiverActive();
                break;
            case enumConnection::RECEIVER_CONSTRAINTS:
                HandleCurlDoneGetReceiverConstraints();
                break;
            case enumConnection::SENDER_PATCH:
                HandleCurlDonePatchSender();
                break;
            case enumConnection::RECEIVER_PATCH:
                HandleCurlDonePatchReceiver();
                break;
         }
}
}

void ClientApiImpl::HandleCurlDoneTarget()
{
    //don't need to check for m_pPoster as check in HandleCurlDon
    Json::Value jsData = ConvertToJson(m_asyncResponse.sResponse);
    if(202 == m_asyncResponse.nCode && jsData["id"].isString())
    {
        pmlLog(pml::LOG_DEBUG) << "NMOS: " << m_asyncResponse.nCode << ": " << jsData["id"].asString() ;
        m_pPoster->_RequestTargetResult(202, jsData["id"].asString(), m_sCurlResourceId);
        return;
    }

    if(jsData["error"].isString())
    {
        pmlLog(pml::LOG_DEBUG) << "NMOS: " << m_asyncResponse.nCode << ": " << jsData["error"].asString() ;
        m_pPoster->_RequestTargetResult(m_asyncResponse.nCode, jsData["error"].asString(), m_sCurlResourceId);
        return;
    }
    pmlLog(pml::LOG_DEBUG) << "NMOS: " << m_asyncResponse.nCode << ": " << m_asyncResponse.sResponse ;
    m_pPoster->_RequestTargetResult(m_asyncResponse.nCode, m_asyncResponse.sResponse, m_sCurlResourceId);
}

void ClientApiImpl::HandleCurlDonePatchSender()
{
    if(m_asyncResponse.nCode == 200 || m_asyncResponse.nCode == 202)
    {
        connectionSender<activationResponse> con(ConvertToJson(m_asyncResponse.sResponse));
        m_pPoster->_RequestPatchSenderResult(m_asyncResponse, con, m_sCurlResourceId);
    }
    else
    {
        m_pPoster->_RequestPatchSenderResult(m_asyncResponse, {}, m_sCurlResourceId);
    }
}

void ClientApiImpl::HandleCurlDonePatchReceiver()
{
    if(m_asyncResponse.nCode == 200 || m_asyncResponse.nCode == 202)
    {
        connectionReceiver<activationResponse> con(ConvertToJson(m_asyncResponse.sResponse));
        m_pPoster->_RequestPatchReceiverResult(m_asyncResponse, con, m_sCurlResourceId);
    }
    else
    {
        m_pPoster->_RequestPatchReceiverResult(m_asyncResponse, {}, m_sCurlResourceId);
    }
}

void ClientApiImpl::HandleCurlDoneGetSenderStaged()
{
    if(m_asyncResponse.nCode == 200)
    {
        connectionSender<activationResponse> con(ConvertToJson(m_asyncResponse.sResponse));
        m_pPoster->_RequestGetSenderStagedResult(m_asyncResponse, con, m_sCurlResourceId);
    }
    else
    {
        m_pPoster->_RequestGetSenderStagedResult(m_asyncResponse, {}, m_sCurlResourceId);
    }
}

void ClientApiImpl::HandleCurlDoneGetSenderActive()
{
    if(m_asyncResponse.nCode == 200)
    {
        connectionSender<activationResponse> con(ConvertToJson(m_asyncResponse.sResponse));
        m_pPoster->_RequestGetSenderActiveResult(m_asyncResponse, con, m_sCurlResourceId);
    }
    else
    {
        m_pPoster->_RequestGetSenderActiveResult(m_asyncResponse, {}, m_sCurlResourceId);
    }
}

void ClientApiImpl::HandleCurlDoneGetSenderConstraints()
{
    if(m_asyncResponse.nCode == 200)
    {
        m_pPoster->_RequestGetSenderConstraintsResult(m_asyncResponse, CreateConstraints(ConvertToJson(m_asyncResponse.sResponse)), m_sCurlResourceId);
    }
    else
    {
        m_pPoster->_RequestGetSenderConstraintsResult(m_asyncResponse, {}, m_sCurlResourceId);
    }
}

void ClientApiImpl::HandleCurlDoneGetReceiverConstraints()
{
    if(m_asyncResponse.nCode == 200)
    {
        m_pPoster->_RequestGetReceiverConstraintsResult(m_asyncResponse, CreateConstraints(ConvertToJson(m_asyncResponse.sResponse)), m_sCurlResourceId);
    }
    else
    {
        m_pPoster->_RequestGetReceiverConstraintsResult(m_asyncResponse, {}, m_sCurlResourceId);
    }
}
void ClientApiImpl::HandlCurlDoneGetSenderTransportFile()
{
    if(m_asyncResponse.nCode == 200)
    {
        m_pPoster->_RequestGetSenderTransportFileResult(m_asyncResponse, m_asyncResponse.sResponse, m_sCurlResourceId);
    }
    else
    {
        m_pPoster->_RequestGetSenderTransportFileResult(m_asyncResponse, {}, m_sCurlResourceId);
    }
}

void ClientApiImpl::HandleCurlDoneGetReceiverStaged()
{
    if(m_asyncResponse.nCode == 200)
    {
        connectionReceiver<activationResponse> con(ConvertToJson(m_asyncResponse.sResponse));
        m_pPoster->_RequestGetReceiverStagedResult(m_asyncResponse, con, m_sCurlResourceId);
    }
    else
    {
        m_pPoster->_RequestGetReceiverStagedResult(m_asyncResponse, {}, m_sCurlResourceId);
    }
}

void ClientApiImpl::HandleCurlDoneGetReceiverActive()
{
    if(m_asyncResponse.nCode == 200)
    {
        connectionReceiver<activationResponse> con(ConvertToJson(m_asyncResponse.sResponse));
        m_pPoster->_RequestGetReceiverActiveResult(m_asyncResponse, con, m_sCurlResourceId);
    }
    else
    {
        m_pPoster->_RequestGetReceiverActiveResult(m_asyncResponse, {}, m_sCurlResourceId);
    }
}

void ClientApiImpl::ConnectToQueryServer()
{
    // @todo ConnectToQueryServer
}

void ClientApiImpl::GetNodeDetails()
{
    if(m_lstResolve.empty() == false)
    {
        pmlLog(pml::LOG_DEBUG) << "NMOS: ClientApiImpl::GetNodeDetails" ;

        //need to get all the node details.
        //lets launch a thread that will ask for self, devices, sources, flows, senders, receivers
        auto pInstance = m_lstResolve.front();
        pml::ThreadPool::Get().Submit(NodeBrowser, this, pInstance);
        //std::thread th(NodeBrowser, this, m_lstResolve.front());
        //th.detach();    //@todo better to wait on the thread at close down
        m_lstResolve.erase(m_lstResolve.begin());
    }
}

ClientApiImpl::enumMode ClientApiImpl::GetMode()
{
    std::lock_guard<std::mutex> lg(m_mutex);
    return m_eMode;
}



resourcechanges<Self> ClientApiImpl::AddNode(const std::string& sIpAddress, const Json::Value& jsData)
{
    std::lock_guard<std::mutex> lg(m_mutex);

    resourcechanges<Self> changed;
    if(jsData["id"].isString())
    {
        auto pSelf = m_nodes.UpdateResource(jsData);
        if(!pSelf)
        {
            pSelf = Self::Create(jsData);
            if(pSelf)
            {
                m_nodes.AddResource(sIpAddress, pSelf);
                changed.lstAdded.push_back(pSelf);

                pmlLog(pml::LOG_INFO) << "NMOS: " << "Node: " << pSelf->GetId() << " found at " << sIpAddress ;
            }
            else
            {
                pmlLog(pml::LOG_INFO) << "NMOS: " << "Found node but json data incorrect: " << jsData;
            }
        }
        else
        {
            changed.lstUpdated.push_back(pSelf);
        }
    }
    else
    {
        pmlLog(pml::LOG_INFO) << "NMOS: " << "Reply from " << sIpAddress << "but not valid JSON - id not correct" ;
    }
    return changed;
}

resourcechanges<Device> ClientApiImpl::AddDevices(const std::string& sIpAddress, const Json::Value& jsData)
{
    std::lock_guard<std::mutex> lg(m_mutex);
    resourcechanges<Device> changed;

    if(jsData.isArray())
    {
        for(Json::ArrayIndex ai = 0; ai < jsData.size(); ++ai)
        {
            if(jsData[ai].isObject() && jsData[ai]["id"].isString())
            {
                changed += AddDevice(sIpAddress, jsData[ai]);
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
    return changed;
}

resourcechanges<Device> ClientApiImpl::AddDevice(const std::string& sIpAddress,const Json::Value& jsData)
{
    resourcechanges<Device> changed;
    auto pResource = m_devices.UpdateResource(jsData);
    if(!pResource)
    {
        pResource = Device::Create(jsData);
        if(pResource)
        {
            m_devices.AddResource(sIpAddress, pResource);
            changed.lstAdded.push_back(pResource);
            pmlLog(pml::LOG_INFO) << "NMOS: " << "Device: " << pResource->GetId() << " found at " << sIpAddress ;
        }
        else
        {
            pmlLog(pml::LOG_INFO) << "NMOS: " << "Found device but json data incorrect: " << jsData;
        }
    }
    else
    {
        changed.lstUpdated.push_back(pResource);
    }
    return changed;
}

resourcechanges<Source> ClientApiImpl::AddSources(const std::string& sIpAddress, const Json::Value& jsData)
{
    std::lock_guard<std::mutex> lg(m_mutex);
    resourcechanges<Source> changed;

    if(jsData.isArray())
    {
        for(Json::ArrayIndex ai = 0; ai < jsData.size(); ++ai)
        {
            if(jsData[ai].isObject() && jsData[ai]["format"].isString() && jsData[ai]["id"].isString())
            {
                changed += AddSource(sIpAddress, jsData[ai]);
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

    return changed;
}

resourcechanges<Source> ClientApiImpl::AddSource(const std::string& sIpAddress, const Json::Value& jsData)
{
    resourcechanges<Source> changed;
    auto pSourceCore = m_sources.UpdateResource(jsData);
    if(!pSourceCore)
    {
        if(jsData["format"].asString().find("urn:x-nmos:format:audio") != std::string::npos)
        {   //Audio
            auto pResource = SourceAudio::Create(jsData);
            if(pResource)
            {
                m_sources.AddResource(sIpAddress, pResource);
                changed.lstAdded.push_back(pResource);
                pmlLog(pml::LOG_INFO) << "NMOS: " << "SourceAudio: " << pResource->GetId() << " found at " << sIpAddress ;
            }
            else
            {
                pmlLog(pml::LOG_INFO) << "NMOS: " << "Found Source but json data incorrect: " << jsData;
            }
        }
        else
        {   //Generic
            auto pResource = SourceGeneric::Create(jsData);
            if(pResource)
            {
                m_sources.AddResource(sIpAddress, pResource);

                changed.lstAdded.push_back(pResource);
                pmlLog(pml::LOG_INFO) << "NMOS: " << "SourceGeneric: " << pResource->GetId() << " found at " << sIpAddress ;
            }
            else
            {
                pmlLog(pml::LOG_INFO) << "NMOS: " << "Found Source but json data incorrect: " << jsData;
            }
        }
    }
    else
    {
        changed.lstUpdated.push_back(pSourceCore);
    }
    return changed;
}

resourcechanges<Flow> ClientApiImpl::AddFlows(const std::string& sIpAddress, const Json::Value& jsData)
{
    std::lock_guard<std::mutex> lg(m_mutex);

    resourcechanges<Flow> changed;

    if(jsData.isArray())
    {
        for(Json::ArrayIndex ai = 0; ai < jsData.size(); ++ai)
        {
            if(jsData[ai].isObject() && jsData[ai]["format"].isString() && jsData[ai]["id"].isString())
            {
                changed += AddFlow(sIpAddress, jsData[ai]);
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
    return changed;
}

resourcechanges<Flow> ClientApiImpl::AddFlow(const std::string& sIpAddress, const Json::Value& jsData)
{
    resourcechanges<Flow> changed;

    auto pFlowCore = m_flows.UpdateResource(jsData);
    if(!pFlowCore)
    {
        auto pFlow = CreateFlow(jsData, sIpAddress);
        if(pFlow)
        {
            changed.lstAdded.push_back(pFlow);
        }
    }
    else
    {
        changed.lstUpdated.push_back(pFlowCore);

    }
    return changed;
}

std::shared_ptr<const Flow> ClientApiImpl::CreateFlow(const Json::Value& jsData, const std::string& sIpAddress)
{

    if(jsData["format"].asString().find("urn:x-nmos:format:audio") != std::string::npos)
    {
        return CreateFlowAudio(jsData, sIpAddress);
    }
    else if(jsData["format"].asString().find("urn:x-nmos:format:video") != std::string::npos)
    {
        return CreateFlowVideo(jsData, sIpAddress);
    }
    else if(jsData["format"].asString().find("urn:x-nmos:format:data") != std::string::npos)
    {
        return CreateFlowData(jsData, sIpAddress);
    }
    else if(jsData["format"].asString().find("urn:x-nmos:format:mux") != std::string::npos)
    {
        return CreateFlowMux(jsData, sIpAddress);
    }
    else
    {
        pmlLog(pml::LOG_WARN) << "NMOS: ClientApi: Unknown flow format: " << jsData["format"].asString();
        return nullptr;
    }

}

std::shared_ptr<const Flow> ClientApiImpl::CreateFlowAudio(const Json::Value& jsData, const std::string& sIpAddress)
{
    if(jsData["media_type"].isString())
    {
        if(jsData["media_type"].asString() == "audio/L24" || jsData["media_type"].asString() == "audio/L20" || jsData["media_type"].asString() == "audio/L16" || jsData["media_type"].asString() == "audio/L8")
        {   //Raw Audio
            return CreateFlowAudioRaw(jsData, sIpAddress);
        }
        else
        {   //Code audio
            return CreateFlowAudioCoded(jsData, sIpAddress);
        }
    }
    return nullptr;
}

std::shared_ptr<const Flow> ClientApiImpl::CreateFlowAudioCoded(const Json::Value& jsData, const std::string& sIpAddress)
{
    auto pResource = FlowAudioCoded::Create(jsData);
    if(pResource)
    {
        m_flows.AddResource(sIpAddress, pResource);
        pmlLog(pml::LOG_INFO) << "NMOS: " << "FlowAudioCoded: " << pResource->GetId() << " found at " << sIpAddress ;
        return pResource;
    }
    else
    {
        pmlLog(pml::LOG_INFO) << "NMOS: " << "Found Flow but json data incorrect: " << jsData;
        return nullptr;
    }
}

std::shared_ptr<const Flow> ClientApiImpl::CreateFlowAudioRaw(const Json::Value& jsData, const std::string& sIpAddress)
{
    auto pResource = FlowAudioRaw::Create(jsData);
    if(pResource)
    {
        m_flows.AddResource(sIpAddress, pResource);
        pmlLog(pml::LOG_INFO) << "NMOS: " << "FlowAudioRaw: " << pResource->GetId() << " found at " << sIpAddress ;
        return pResource;
    }
    else
    {
        pmlLog(pml::LOG_INFO) << "NMOS: " << "Found Flow but json data incorrect: " << jsData;
        return nullptr;
    }

}

std::shared_ptr<const Flow> ClientApiImpl::CreateFlowVideo(const Json::Value& jsData, const std::string& sIpAddress)
{
    if(jsData["media_type"].isString())
    {//Coded vidoe
        if(jsData["media_type"].asString() == "vidoe/raw")
        {
            return CreateFlowVideoRaw(jsData, sIpAddress);
        }
        else
        {
            return CreateFlowVideoCoded(jsData, sIpAddress);
        }
    }
    return nullptr;
}

std::shared_ptr<const Flow> ClientApiImpl::CreateFlowVideoRaw(const Json::Value& jsData, const std::string& sIpAddress)
{
    auto pResource = FlowVideoRaw::Create(jsData);
    if(pResource)
    {
        m_flows.AddResource(sIpAddress, pResource);
        pmlLog(pml::LOG_INFO) << "NMOS: " << "FlowVideoRaw: " << pResource->GetId() << " found at " << sIpAddress ;
        return pResource;
    }
    else
    {
        pmlLog(pml::LOG_INFO) << "NMOS: " << "Found Flow but json data incorrect: " << jsData;
        return nullptr;
    }
}

std::shared_ptr<const Flow> ClientApiImpl::CreateFlowVideoCoded(const Json::Value& jsData, const std::string& sIpAddress)
{
    auto pResource = FlowVideoCoded::Create(jsData);
    if(pResource)
    {
        m_flows.AddResource(sIpAddress, pResource);
        pmlLog(pml::LOG_INFO) << "NMOS: " << "FlowVideoCoded: " << pResource->GetId() << " found at " << sIpAddress ;
        return pResource;
    }
    else
    {
        pmlLog(pml::LOG_INFO) << "NMOS: " << "Found Flow but json data incorrect: " << jsData;
        return nullptr;
    }
}

std::shared_ptr<const Flow> ClientApiImpl::CreateFlowData(const Json::Value& jsData, const std::string& sIpAddress)
{
    if(jsData["media_type"] == "video/smpte291")
    {
        auto pResource = FlowDataSdiAnc::Create(jsData);
        if(pResource)
        {
            m_flows.AddResource(sIpAddress, pResource);
            pmlLog(pml::LOG_INFO) << "NMOS: " << "FlowDataSdiAnc: " << pResource->GetId() << " found at " << sIpAddress ;
            return pResource;
        }
        else
        {
            pmlLog(pml::LOG_INFO) << "NMOS: " << "Found Flow but json data incorrect: " << jsData;
            return nullptr;
        }
    }
    else if(jsData["media_type"] == "application/json")
    {
        auto pResource = FlowDataJson::Create(jsData);
        if(pResource)
        {
            m_flows.AddResource(sIpAddress, pResource);
            pmlLog(pml::LOG_INFO) << "NMOS: " << "FlowDataSdiAnc: " << pResource->GetId() << " found at " << sIpAddress ;
            return pResource;
        }
        else
        {
            pmlLog(pml::LOG_INFO) << "NMOS: " << "Found Flow but json data incorrect: " << jsData;
            return nullptr;
        }
    }
    else
    {
        return nullptr;
    }
}

std::shared_ptr<const Flow> ClientApiImpl::CreateFlowMux(const Json::Value& jsData, const std::string& sIpAddress)
{
    //Mux only at the momemnt
    auto pResource = FlowMux::Create(jsData);
    if(pResource)
    {
        m_flows.AddResource(sIpAddress, pResource);
        pmlLog(pml::LOG_INFO) << "NMOS: " << "FlowMux: " << pResource->GetId() << " found at " << sIpAddress ;
        return pResource;
    }
    else
    {
        pmlLog(pml::LOG_INFO) << "NMOS: " << "Found Flow but json data incorrect: " << jsData;
        return nullptr;
    }
}


resourcechanges<Sender> ClientApiImpl::AddSenders(const std::string& sIpAddress, const Json::Value& jsData)
{
    std::lock_guard<std::mutex> lg(m_mutex);
    resourcechanges<Sender> changed;
    if(jsData.isArray())
    {
        for(Json::ArrayIndex ai = 0; ai < jsData.size(); ++ai)
        {
            if(jsData[ai].isObject() && jsData[ai]["id"].isString())
            {
                changed += AddSender(sIpAddress, jsData[ai]);
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
    return changed;
}

resourcechanges<Sender> ClientApiImpl::AddSender(const std::string& sIpAddress, const Json::Value& jsData)
{
    resourcechanges<Sender> changed;
    auto pResource = m_senders.UpdateResource(jsData);
    if(!pResource)
    {
        pResource = Sender::Create(jsData);
        if(pResource)
        {
            m_senders.AddResource(sIpAddress, pResource);
            changed.lstAdded.push_back(pResource);
            pmlLog(pml::LOG_INFO) << "NMOS: " << "Sender: " << pResource->GetId() << " found at " << sIpAddress ;
        }
        else
        {
            pmlLog(pml::LOG_INFO) << "NMOS: " << "Found Sender but json data incorrect: " << jsData ;
        }
    }
    else
    {
        changed.lstUpdated.push_back(pResource);
    }
    return changed;
}

resourcechanges<Receiver> ClientApiImpl::AddReceivers(const std::string& sIpAddress, const Json::Value& jsData)
{
    std::lock_guard<std::mutex> lg(m_mutex);
    resourcechanges<Receiver> changed;
    if(jsData.isArray())
    {
        for(Json::ArrayIndex ai = 0; ai < jsData.size(); ++ai)
        {
            if(jsData[ai].isObject() && jsData[ai]["id"].isString())
            {
                changed += AddReceiver(sIpAddress, jsData[ai]);
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
    return changed;
}

resourcechanges<Receiver> ClientApiImpl::AddReceiver(const std::string& sIpAddress, const Json::Value& jsData)
{
    resourcechanges<Receiver> changed;
    auto pResource = m_receivers.UpdateResource(jsData);
    if(!pResource)
    {
        pResource = Receiver::Create(jsData);
        if(pResource)
        {
            m_receivers.AddResource(sIpAddress, pResource);
            changed.lstAdded.push_back(pResource);
            pmlLog(pml::LOG_INFO) << "NMOS: " << "Receiver: " << pResource->GetId() << " found at " << sIpAddress ;
        }
        else
        {
            pmlLog(pml::LOG_INFO) << "NMOS: " << "Found Receiver but json data incorrect: " << jsData ;
        }
    }
    else
    {
        changed.lstUpdated.push_back(pResource);
    }
    return changed;

}

void ClientApiImpl::RemoveResources(const std::string& sIpAddress)
{
    auto changedSelf = m_nodes.RemoveResources(sIpAddress);
    auto changedDevices = m_devices.RemoveResources(sIpAddress);
    auto changedSources = m_sources.RemoveResources(sIpAddress);
    auto changedFlows = m_flows.RemoveResources(sIpAddress);
    auto changedSenders = m_senders.RemoveResources(sIpAddress);
    auto changedReceivers = m_receivers.RemoveResources(sIpAddress);

    if(RunQuery(changedSelf.lstRemoved, ClientApiImpl::NODES))
    {
        m_pPoster->_NodeChanged(changedSelf);
    }

    if(RunQuery(changedDevices.lstRemoved, ClientApiImpl::DEVICES))
    {
        m_pPoster->_DeviceChanged(changedDevices);
    }

    if(RunQuery(changedSources.lstRemoved, ClientApiImpl::SOURCES))
    {
        m_pPoster->_SourceChanged(changedSources);
    }

    if(RunQuery(changedFlows.lstRemoved, ClientApiImpl::FLOWS))
    {
        m_pPoster->_FlowChanged(changedFlows);
    }

    if(RunQuery(changedSenders.lstRemoved, ClientApiImpl::SENDERS))
    {
        m_pPoster->_SenderChanged(changedSenders);
    }

    if(RunQuery(changedReceivers.lstRemoved, ClientApiImpl::RECEIVERS))
    {
        m_pPoster->_ReceiverChanged(changedReceivers);
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


std::shared_ptr<const Self> ClientApiImpl::FindNode(const std::string& sUid)
{
    return m_nodes.GetNmosResource(sUid);
}

const std::map<std::string, std::shared_ptr<Device> >& ClientApiImpl::GetDevices()
{
    return m_devices.GetResources();
}


std::shared_ptr<const Device> ClientApiImpl::FindDevice(const std::string& sUid)
{
    return m_devices.GetNmosResource(sUid);
}


const std::map<std::string, std::shared_ptr<Source> >& ClientApiImpl::GetSources()
{
    return m_sources.GetResources();
}


std::shared_ptr<const Source>ClientApiImpl::FindSource(const std::string& sUid)
{
    return m_sources.GetNmosResource(sUid);
}


const std::map<std::string, std::shared_ptr<Flow> >& ClientApiImpl::GetFlows()
{
    return m_flows.GetResources();
}


std::shared_ptr<const Flow>ClientApiImpl::FindFlow(const std::string& sUid)
{
    return m_flows.GetNmosResource(sUid);
}



const std::map<std::string, std::shared_ptr<Sender> >& ClientApiImpl::GetSenders()
{
    return m_senders.GetResources();
}


std::shared_ptr<const Sender>ClientApiImpl::FindSender(const std::string& sUid)
{
    return m_senders.GetNmosResource(sUid);
}



const std::map<std::string, std::shared_ptr<Receiver> >& ClientApiImpl::GetReceivers()
{
    return m_receivers.GetResources();
}

std::shared_ptr<const Receiver> ClientApiImpl::FindReceiver(const std::string& sUid)
{
    return m_receivers.GetNmosResource(sUid);
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

        m_pCurl->PutPatchAsync(sUrl, ConvertFromJson(pSender->GetJson(version)), static_cast<long>(enumConnection::TARGET), true, pReceiver->GetId());
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
        m_pCurl->PutPatchAsync(sUrl, "{}", static_cast<long>(enumConnection::TARGET), true, pReceiver->GetId());

        return true;
    }
    return false;
}

std::string ClientApiImpl::GetTargetUrl(std::shared_ptr<Receiver> pReceiver, ApiVersion& version)
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



std::string ClientApiImpl::GetConnectionUrlSingle(std::shared_ptr<Resource> pResource, const std::string& sDirection, const std::string& sEndpoint)
{
    auto sUrl = GetConnectionUrl(pResource);
    if(sUrl.empty() == false)
    {
        sUrl += "/single/"+sDirection+"/"+pResource->GetId()+"/"+sEndpoint;
    }
    return sUrl;
}

std::string ClientApiImpl::GetConnectionUrl(std::shared_ptr<Resource> pResource)
{
    //get the device Id
    auto itDevice =  m_devices.FindNmosResource(pResource->GetParentResourceId());
    if(itDevice == m_devices.GetResources().end())
    {
        pmlLog(pml::LOG_ERROR) << "NMOS: " << "Device: " << pResource->GetParentResourceId() << " not found" ;
        return std::string();
    }

    //have we a preferred url to control this device?
    auto theControl = control("urn:x-nmos:control:sr-ctrl/"+m_version.GetVersionAsString());
    auto uri = itDevice->second->GetPreferredUrl(theControl);
    if(uri.Get().empty() == false)
    {
        return uri.Get();
    }

    //work our way through the possible endpoints and find one which will reply to us
    std::set<url> setTried;
    for(auto itControl = itDevice->second->GetControls().lower_bound(theControl); itControl != itDevice->second->GetControls().upper_bound(theControl); ++itControl)
    {
        //is it in the same subnet as any of our interfaces
        for(auto pairInterface : m_mSubnetMasks)
        {
            //same subnet mask and not tried this ip address before
            auto vSplit = SplitString(itControl->second.Get(), '/');

            if(CheckSubnet(url(vSplit[1].substr(0, vSplit[1].find(':'))), pairInterface.first, pairInterface.second) && setTried.insert(itControl->second).second)
            {
                //we should be able to connect on this
                auto curlResp = CurlRegister::Get(itControl->second.Get());
                if(curlResp.nCode == 200)
                {
                    //found one - make it our preferred one
                    itDevice->second->SetPreferredUrl(theControl, itControl->second);
                    return itControl->second.Get();
                }
            }
        }
    }

    //couldn't connect so try from beginning ignoring the one we couldn't connect on
    for(auto itControl = itDevice->second->GetControls().lower_bound(theControl); itControl != itDevice->second->GetControls().upper_bound(theControl); ++itControl)
    {
        //same subnet mask and not tried this ip address before
        if(setTried.insert(itControl->second).second)
        {
            auto curlResp = CurlRegister::Get(itControl->second.Get());
            if(curlResp.nCode == 200)
            {
                //found one - make it our preferred one
                itDevice->second->SetPreferredUrl(theControl, itControl->second);
                return itControl->second.Get();
            }
        }
    }
    return std::string();
}

curlResponse CreateCurlResponse(int nCode, const std::string& sMessage)
{
    curlResponse resp;
    resp.nCode = nCode;
    Json::Value jsData;
    jsData["code"] = nCode;
    jsData["error"] = sMessage;
    resp.sResponse = ConvertFromJson(jsData);
    return resp;
}

curlResponse ClientApiImpl::RequestSender(const std::string& sSenderId, enumConnection eType, bool bAsync, bool bJson)
{
    std::lock_guard<std::mutex> lg(m_mutex);

    auto pSender = GetSender(sSenderId);
    if(!pSender)
    {
        return CreateCurlResponse(404, "Sender not found with id "+sSenderId);
    }

    std::string sConnectionUrl(GetConnectionUrlSingle(pSender, "senders", STR_CONNECTION[eType]));
    if(sConnectionUrl.empty())
    {
        return CreateCurlResponse(408, "Control endpoint not found");
    }

    if(bAsync)
    {
        m_pCurl->GetAsync(sConnectionUrl, (long)eType, bJson);
        return CreateCurlResponse(202, "Request actioned");
    }
    else
    {
        return CurlRegister::Get(sConnectionUrl, bJson);
    }
}

curlResponse ClientApiImpl::RequestReceiver(const std::string& sReceiverId, enumConnection eType, bool bAsync, bool bJson)
{
    std::lock_guard<std::mutex> lg(m_mutex);
    auto pReceiver = GetReceiver(sReceiverId);
    if(!pReceiver)
    {
        return CreateCurlResponse(404, "Receiver not found with id "+sReceiverId);
    }

    std::string sConnectionUrl(GetConnectionUrlSingle(pReceiver, "receivers", STR_CONNECTION[eType]));
    if(sConnectionUrl.empty())
    {
        return CreateCurlResponse(408, "Control endpoint not found");
    }

    if(bAsync)
    {
        m_pCurl->GetAsync(sConnectionUrl, (long)eType, bJson);
        return CreateCurlResponse(202, "Request actioned");
    }
    else
    {
        return CurlRegister::Get(sConnectionUrl, bJson);
    }
}

std::pair<curlResponse, std::vector<Constraints>> ClientApiImpl::RequestSenderConstraints(const std::string& sSenderId, bool bAsync)
{
    auto resp =  RequestSender(sSenderId, enumConnection::SENDER_CONSTRAINTS, bAsync);
    if(resp.nCode != 200)
    {
        return {resp, {}};
    }
    else
    {
        return {resp, CreateConstraints(ConvertToJson(resp.sResponse))};
    }
}

std::pair<curlResponse, std::experimental::optional<connectionSender<activationResponse>>> ClientApiImpl::RequestSenderConnection(const std::string& sSenderId,enumConnection eType, bool bAsync)
{
    auto resp = RequestSender(sSenderId, eType, bAsync);
    if(resp.nCode != 200)
    {
        return {resp, {}};
    }
    else
    {
        return {resp, connectionSender<activationResponse>(ConvertToJson(resp.sResponse))};
    }
}

std::pair<curlResponse, std::experimental::optional<connectionSender<activationResponse>>> ClientApiImpl::RequestSenderStaged(const std::string& sSenderId, bool bAsync)
{
    return RequestSenderConnection(sSenderId, enumConnection::SENDER_STAGED, bAsync);
}

std::pair<curlResponse, std::experimental::optional<connectionSender<activationResponse>>>  ClientApiImpl::RequestSenderActive(const std::string& sSenderId, bool bAsync)
{
    return RequestSenderConnection(sSenderId, enumConnection::SENDER_ACTIVE, bAsync);
}

std::pair<curlResponse, std::experimental::optional<std::string>>  ClientApiImpl::RequestSenderTransportFile(const std::string& sSenderId, bool bAsync)
{
    auto resp = RequestSender(sSenderId, enumConnection::SENDER_TRANSPORTFILE, bAsync, false);
    if(resp.nCode != 200)
    {
        return {resp, {}};
    }
    return {resp, resp.sResponse};
}

std::pair<curlResponse, std::vector<Constraints>> ClientApiImpl::RequestReceiverConstraints(const std::string& sReceiverId, bool bAsync)
{
    auto resp =  RequestReceiver(sReceiverId, enumConnection::RECEIVER_CONSTRAINTS, bAsync);
    if(resp.nCode != 200)
    {
        return {resp, {}};
    }
    else
    {
        return {resp, CreateConstraints(ConvertToJson(resp.sResponse))};
    }
}

std::pair<curlResponse, std::experimental::optional<connectionReceiver<activationResponse>>> ClientApiImpl::RequestReceiverConnection(const std::string& sReceiverId,enumConnection eType, bool bAsync)
{
    auto resp = RequestReceiver(sReceiverId, eType, bAsync);
    if(resp.nCode != 200)
    {
        return {resp, {}};
    }
    else
    {
        return {resp, connectionReceiver<activationResponse>(ConvertToJson(resp.sResponse))};
    }
}

std::pair<curlResponse, std::experimental::optional<connectionReceiver<activationResponse>>> ClientApiImpl::RequestReceiverStaged(const std::string& sReceiverId, bool bAsync)
{
    return RequestReceiverConnection(sReceiverId, enumConnection::RECEIVER_STAGED, bAsync);
}

std::pair<curlResponse, std::experimental::optional<connectionReceiver<activationResponse>>> ClientApiImpl::RequestReceiverActive(const std::string& sReceiverId, bool bAsync)
{
    return RequestReceiverConnection(sReceiverId, enumConnection::RECEIVER_ACTIVE, bAsync);
}


std::pair<curlResponse, std::experimental::optional<connectionSender<activationResponse>>> ClientApiImpl::PatchSenderStaged(const std::string& sSenderId, const connectionSender<activationRequest>& aConnection, bool bAsync)
{
    std::lock_guard<std::mutex> lg(m_mutex);
    auto pSender = GetSender(sSenderId);
    if(!pSender)
    {
        return {CreateCurlResponse(404, "Sender not found with id "+sSenderId), {}};
    }

    std::string sConnectionUrl(GetConnectionUrlSingle(pSender, "senders", STR_CONNECTION[enumConnection::SENDER_STAGED]));
    if(sConnectionUrl.empty())
    {
        return {CreateCurlResponse(408, "Control endpoint not found"), {}};
    }

    if(bAsync)
    {
        m_pCurl->PutPatchAsync(sConnectionUrl, ConvertFromJson(aConnection.GetJson()), static_cast<long>(enumConnection::SENDER_PATCH), false, sSenderId);
        return {CreateCurlResponse(202, "Request actioned"), {}};
    }
    else
    {
        auto resp = CurlRegister::PutPatch(sConnectionUrl, ConvertFromJson(aConnection.GetJson()), false, sSenderId);
        if(resp.nCode != 200)
        {
            return {resp, {}};
        }
        else
        {
            return {resp, connectionSender<activationResponse>(ConvertToJson(resp.sResponse))};
        }
    }
}

std::pair<curlResponse, std::experimental::optional<connectionReceiver<activationResponse>>> ClientApiImpl::PatchReceiverStaged(const std::string& sReceiverId, const connectionReceiver<activationRequest>& aConnection, bool bAsync)
{
    std::lock_guard<std::mutex> lg(m_mutex);
    auto pReceiver = GetReceiver(sReceiverId);
    if(!pReceiver)
    {
        return {CreateCurlResponse(404, "Receiver not found with id "+sReceiverId), {}};
    }

    std::string sConnectionUrl(GetConnectionUrlSingle(pReceiver, "receivers", STR_CONNECTION[enumConnection::SENDER_STAGED]));
    if(sConnectionUrl.empty())
    {
        return {CreateCurlResponse(408, "Control endpoint not found"), {}};
    }

    if(bAsync)
    {
        m_pCurl->PutPatchAsync(sConnectionUrl, ConvertFromJson(aConnection.GetJson()), static_cast<long>(enumConnection::RECEIVER_PATCH), false, sReceiverId);
        return {CreateCurlResponse(202, "Request actioned"), {}};
    }
    else
    {
        auto resp =  CurlRegister::PutPatch(sConnectionUrl, ConvertFromJson(aConnection.GetJson()), false, sReceiverId);
        if(resp.nCode != 200)
        {
            return {resp, {}};
        }
        else
        {
            return {resp, connectionReceiver<activationResponse>(ConvertToJson(resp.sResponse))};
        }
    }

}


std::shared_ptr<Sender> ClientApiImpl::GetSender(const std::string& sSenderId)
{
    auto itSender = m_senders.FindNmosResource(sSenderId);
    if(itSender == m_senders.GetResources().end())
    {
        pmlLog(pml::LOG_ERROR) << "NMOS: " << "Sender: " << sSenderId << " not found." ;
        return 0;
    }
    return itSender->second;
}

std::shared_ptr<Receiver> ClientApiImpl::GetReceiver(const std::string& sReceiverId)
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

resourcechanges<Device> ClientApiImpl::RemoveStaleDevices()
{
    resourcechanges<Device> changes;
    changes.lstRemoved = m_devices.RemoveStaleResources();
    return changes;
}

resourcechanges<Source> ClientApiImpl::RemoveStaleSources()
{
    resourcechanges<Source> changes;
    changes.lstRemoved = m_sources.RemoveStaleResources();
    return changes;
}

resourcechanges<Flow> ClientApiImpl::RemoveStaleFlows()
{
    resourcechanges<Flow> changes;
    changes.lstRemoved = m_flows.RemoveStaleResources();
    return changes;
}

resourcechanges<Sender> ClientApiImpl::RemoveStaleSenders()
{
    resourcechanges<Sender> changes;
    changes.lstRemoved = m_senders.RemoveStaleResources();
    return changes;
}

resourcechanges<Receiver> ClientApiImpl::RemoveStaleReceivers()
{
    resourcechanges<Receiver> changes;
    changes.lstRemoved = m_receivers.RemoveStaleResources();
    return changes;
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

    pmlLog(pml::LOG_DEBUG) << "NMOS: " << "ClientApiImpl::Connect Sender: " << sSenderId;

    auto sSenderUrl = GetConnectionUrl(pSender);
    if(sSenderUrl.empty())
    {
        return false;
    }
    auto sReceiverUrl = GetConnectionUrl(pReceiver);
    if(sReceiverUrl.empty())
    {
        return false;
    }

    pml::ThreadPool::Get().Submit(ConnectThread, this, sSenderId, sReceiverId, sSenderUrl, sReceiverUrl);

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
        pmlLog(pml::LOG_DEBUG) << "NMOS: " << "ClientApiImpl::Disconnect No Sender: " << pReceiver->GetSender();
        return false;
    }

    std::string sSenderStageUrl(GetConnectionUrlSingle(pSender, "senders", STR_CONNECTION[enumConnection::SENDER_STAGED]));
    std::string sSenderTransportUrl(GetConnectionUrlSingle(pSender, "senders", STR_CONNECTION[enumConnection::SENDER_TRANSPORTFILE]));
    std::string sReceiverStageUrl(GetConnectionUrlSingle(pReceiver, "receivers", STR_CONNECTION[enumConnection::RECEIVER_STAGED]));


    if(sSenderStageUrl.empty() || sSenderTransportUrl.empty() || sReceiverStageUrl.empty())
    {
        pmlLog(pml::LOG_DEBUG) << "NMOS: " << "ClientApiImpl::Disconnect No URLS" ;
        return false;
    }

    pml::ThreadPool::Get().Submit(DisconnectThread, this, pSender->GetId(), sReceiverId, sSenderStageUrl, sSenderTransportUrl, sReceiverStageUrl);
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
    //store the query
    query aQuery;
    aQuery.sId = CreateGuid();
    aQuery.sQuery = sQuery;
    aQuery.nResource = nResource;
    m_mmQuery.insert(std::make_pair(nResource, aQuery));

    //we get t
    bool bSuccess = QueryQueryServer(GetQueryServer(), aQuery);
    if(bSuccess)
    {
        //call the poster to let the client know the id
        if(m_pPoster)
        {
            m_pPoster->_QuerySubscription(aQuery.sId, nResource, sQuery);
        }
    }
    return bSuccess;
}

bool ClientApiImpl::RemoveQuerySubscriptionRegistry(const std::string& sSubscriptionId)
{
    //@todo
    auto urlServer = GetQueryServer();
    for(auto& pairQuery : m_mmQuery)
    {
        if(pairQuery.second.sId == sSubscriptionId)
        {
            if(pairQuery.second.jsSubscription["id"].isString())
            {
                //if a persitent query then we need to send a delete to the server
                if(pairQuery.second.jsSubscription["persist"].isBool() && pairQuery.second.jsSubscription["persist"].asBool())
                {
                    auto curlResp = CurlRegister::Delete(urlServer.Get(), "subscriptions", pairQuery.second.jsSubscription["id"].asString());
                    if(curlResp.nCode == 204)
                    {
                        if(m_pPoster)
                        {
                            m_pPoster->_QuerySubscriptionRemoved(sSubscriptionId);
                        }
                        m_mmQuery.erase(pairQuery.first);
                        return true;
                    }
                    else
                    {
                        pmlLog(pml::LOG_WARN) << "Failed to remove query subscription " << sSubscriptionId << ": code: " << curlResp.nCode << " '" << curlResp.sResponse << "'";
                        return false;
                    }
                }
                else if(pairQuery.second.jsSubscription["ws_href"].isString())
                {
                    //we simply need to close the websocket
                    m_pWebSocket->CloseConnection(url(pairQuery.second.jsSubscription["ws_href"].asString()));
                    m_mmQuery.erase(pairQuery.first);
                }
                else
                {
                    //something seriously wrong
                    m_mmQuery.erase(pairQuery.first);
                }
            }
            else
            {
                m_mmQuery.erase(pairQuery.first);
                return true;
            }
        }
    }

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
            auto changes = m_nodes.GetResourcesAsList();
            if(RunQuery(changes.lstAdded, sQuery))
            {
                m_pPoster->_NodeChanged(changes);
            }
        }
        break;
    case DEVICES:
        {
            auto changes = m_devices.GetResourcesAsList();
            if(RunQuery(changes.lstAdded, sQuery))
            {
                m_pPoster->_DeviceChanged(changes);
            }
        }
        break;
    case SOURCES:
        {
            auto changes = m_sources.GetResourcesAsList();
            if(RunQuery(changes.lstAdded, sQuery))
            {
                m_pPoster->_SourceChanged(changes);
            }
        }
        break;
    case FLOWS:
        {
            auto changes = m_flows.GetResourcesAsList();
            if(RunQuery(changes.lstAdded, sQuery))
            {
                m_pPoster->_FlowChanged(changes);
            }
        }
        break;
    case SENDERS:
        {
            auto changes = m_senders.GetResourcesAsList();
            if(RunQuery(changes.lstAdded, sQuery))
            {
                m_pPoster->_SenderChanged(changes);
            }
        }
        break;
    case RECEIVERS:
        {
            auto changes = m_receivers.GetResourcesAsList();
            if(RunQuery(changes.lstAdded, sQuery))
            {
                m_pPoster->_ReceiverChanged(changes);
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

template<class T> bool ClientApiImpl::RunQuery(std::list<std::shared_ptr<const T> >& lstCheck, const std::string& sQuery)
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

template<class T> bool ClientApiImpl::RunQuery(std::list<std::shared_ptr<const T> >& lstCheck, int nResource)
{
    if(lstCheck.empty())
    {
        return true;
    }
    if(m_mmQuery.empty())
    {
        pmlLog(pml::LOG_DEBUG) << "NMOS: " << "ClientApiImpl::RunQuery - m_mmQuery empty" ;
    }

    for(auto itResource = lstCheck.begin(); itResource != lstCheck.end(); )
    {
        bool bKeep(false);
        for(auto itQuery = m_mmQuery.lower_bound(nResource); itQuery != m_mmQuery.upper_bound(nResource); ++itQuery)
        {
            if(MeetsQuery(itQuery->second.sQuery, (*itResource)))
            {
                bKeep = true;
                break;
            }
        }

        if(bKeep)
        {
            ++itResource;
        }
        else
        {
            pmlLog() << nResource << " queries not met";
            itResource = lstCheck.erase(itResource);
        }
    }
    return (lstCheck.empty() == false);
}

template<class T> bool ClientApiImpl::RunQuery(resourcechanges<T>& changed, int nResourceType)
{
    if(m_pPoster)
    {
        return (RunQuery(changed.lstAdded, nResourceType) || RunQuery(changed.lstUpdated, nResourceType) || RunQuery(changed.lstRemoved, nResourceType));
    }
    return false;
}

bool ClientApiImpl::MeetsQuery(const std::string& sQuery, std::shared_ptr<const Resource> pResource)
{
    return true;

    //if(sQuery.empty())
   // {
    //    return true;
    //}
    // @todo run the query properly
    //return false;9999
}

url ClientApiImpl::GetQueryServer(const ApiVersion& version)
{
//@todo ClientApiImpl::SubscribeToQueryServer Allow other versions than 1.2
    if(m_mmQueryNodes.empty() == false)
    {
        std::stringstream ssUrl;
        ssUrl <<  m_mmQueryNodes.begin()->second->sHostIP << ":" << m_mmQueryNodes.begin()->second->nPort << "/x-nmos/query/" << version.GetVersionAsString();
        return url(ssUrl.str());
    }
    return url("");
}

void ClientApiImpl::SubscribeToQueryServer()
{
    pmlLog(pml::LOG_DEBUG) << "NMOS: ClientApi: SubscribeToQueryServer";
    if(m_mmQueryNodes.empty() == false)
    {
        url urlQuery = GetQueryServer();
        pmlLog(pml::LOG_DEBUG) << "NMOS: " << "QUERY URL: " << urlQuery;

        //we run through all our queries and post them to the query server asking for the websocket
        for(auto& pairQuery : m_mmQuery)
        {
            QueryQueryServer(urlQuery, pairQuery.second);
        }
        if(m_pPoster)
        {
            m_pPoster->_QueryServerChanged(urlQuery.Get());
        }
    }
}

bool ClientApiImpl::QueryQueryServer(const url& theUrl, ClientApiImpl::query& theQuery)
{
    //create our json query
    Json::Value jsQuery;
    jsQuery["max_update_rate_ms"] = (int)theQuery.nRefreshRate;
    jsQuery["resource_path"] = "/" + STR_RESOURCE[theQuery.nResource] + "s";
    jsQuery["persist"] = false;
    jsQuery["secure"] = false;
    jsQuery["authorization"] = false;

    //@todo params
    Json::Value jsParams(Json::objectValue);


    auto vQuery = SplitString(theQuery.sQuery, '&');
    for(const auto& sKeyValue : vQuery)
    {
        auto vKeyValue = SplitString(sKeyValue, '=');
        if(vKeyValue.size() == 2)
        {
            jsParams[vKeyValue[0]] = vKeyValue[1];
        }
    }
    jsQuery["params"] = jsParams;

    auto curlresp = CurlRegister::Post(theUrl.Get()+"/subscriptions", ConvertFromJson(jsQuery));

    pmlLog(pml::LOG_DEBUG) << "NMOS: ClientApi - query QueryServer: " << theUrl << ": " << theQuery.nResource << " " << ConvertFromJson(jsQuery);

    theQuery.jsSubscription = ConvertToJson(curlresp.sResponse);
    HandleQuerySubscriptionResponse(curlresp.nCode, theQuery);

    return (theQuery.jsSubscription["id"].isString());
}

void ClientApiImpl::HandleQuerySubscriptionResponse(unsigned short nCode, const ClientApiImpl::query& theQuery)
{
    if(nCode == 200 || nCode == 201)
    {
        HandleSuccessfulQuerySubscription(theQuery);
    }
    else if(nCode == 400)
    {
        pmlLog(pml::LOG_ERROR) << "NMOS: Could not subscribe to query server " << theQuery.jsSubscription["error"].asString();
    }
    else if(nCode == 501)
    {
        pmlLog(pml::LOG_ERROR) << "NMOS: Query server does not suppory query " << theQuery.jsSubscription["error"].asString();
    }
    else
    {
        pmlLog(pml::LOG_ERROR) << "NMOS: Unknown error attempting to subscribe to query server";
    }
}

void ClientApiImpl::HandleSuccessfulQuerySubscription(const ClientApiImpl::query& theQuery)
{
    pmlLog(pml::LOG_DEBUG) << "NMOS: Successful query subscription  - now create websocket connection";

    if(theQuery.jsSubscription["ws_href"].isString() && theQuery.jsSubscription["id"].isString())
    {
        if(m_pWebSocket->Connect(url(theQuery.jsSubscription["ws_href"].asString())))
        {
            pmlLog(pml::LOG_INFO) << "NMOS: Attempting websocket connection to " << theQuery.jsSubscription["ws_href"].asString();
        }
        else
        {
            pmlLog(pml::LOG_ERROR) << "NMOS: Failed in websocket connection to " << theQuery.jsSubscription["ws_href"].asString();
        }
    }
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
        return m_mBrowser.insert(std::make_pair(sDomain, std::make_unique<pml::dnssd::Browser>(sDomain))).second;
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


bool ClientApiImpl::WebsocketConnected(const url& theUrl)
{
    pmlLog(pml::LOG_DEBUG) << "NMOS: ClientApi Websocket connected to " << theUrl;
    return true;
}

bool ClientApiImpl::WebsocketMessage(const url& theUrl, const std::string& sMessage)
{
    pmlLog(pml::LOG_DEBUG) << "NMOS: ClientApi: Message: " << sMessage;
    auto jsGrain = ConvertToJson(sMessage);
    //check requirements
    if(!CheckJson(jsGrain, {{"grain_type",{jsondatatype::_STRING}},
                                    {"source_id",{jsondatatype::_STRING}},
                                    {"flow_id",{jsondatatype::_STRING}},
                                    {"origin_timestamp",{jsondatatype::_STRING}},
                                    {"sync_timestamp",{jsondatatype::_STRING}},
                                    {"creation_timestamp",{jsondatatype::_STRING}},
                                    {"rate",{jsondatatype::_OBJECT}},
                                    {"duration",{jsondatatype::_OBJECT}},
                                    {"grain",{jsondatatype::_OBJECT}}}))
    {
        pmlLog(pml::LOG_WARN) << "NMOS: ClientApi - websocket invalid message received '" << sMessage << "'";
    }
    else if(jsGrain["grain_type"].asString() == "event")
    {
        HandleGrainEvent(jsGrain["source_id"].asString(), jsGrain["grain"]);
    }
    else
    {
        pmlLog(pml::LOG_WARN) << "NMOS: ClientApi - unknown grain type'" << jsGrain["grain_type"].asString() << "'";
    }
    return true;
}


void ClientApiImpl::HandleGrainEvent(const std::string& sSourceId, const Json::Value& jsGrain)
{
    if(CheckJson(jsGrain, {{"type", {jsondatatype::_STRING}},
                           {"topic",{jsondatatype::_STRING}}, {"data", {jsondatatype::_ARRAY}}}))
    {
        if(jsGrain["type"].asString() == "urn:x-nmos:format:data.event")
        {
            auto itUpdate = m_mGrainUpdate.find(jsGrain["topic"].asString());
            if(itUpdate != m_mGrainUpdate.end() && jsGrain["data"].isArray())
            {
                itUpdate->second(sSourceId, jsGrain["data"]);
            }
            else
            {
                pmlLog(pml::LOG_WARN) << "No grain handler";
            }
        }
        else
        {
            pmlLog(pml::LOG_WARN) << "NMOS: ClientApi - unknown grain urn'" << jsGrain["type"].asString() << "'";
        }
    }
}


ClientApiImpl::enumGrain ClientApiImpl::WorkoutAction(const Json::Value& jsData)
{
    if(jsData.isMember("post"))
    {
        if(jsData.isMember("pre"))
        {
            return enumGrain::ADD;
        }
        else
        {
            return enumGrain::UPDATE;
        }

    }
    else if(jsData.isMember("pre"))
    {
        return enumGrain::DELETE;
    }
    return enumGrain::UNKNOWN;
}


void ClientApiImpl::GrainUpdateNode(const std::string& sSourceId, const Json::Value& jsData)
{
    resourcechanges<Self> changed;

    for(Json::ArrayIndex ai = 0; ai < jsData.size(); ai++)
    {
        switch(WorkoutAction(jsData[ai]))
        {
            case enumGrain::ADD:
            case enumGrain::UPDATE:
                changed += AddNode(sSourceId, jsData[ai]["post"]);
                break;
            case enumGrain::DELETE:
                if(jsData[ai]["pre"]["id"].isString())
                {
                    auto pResource = m_nodes.RemoveResource(jsData[ai]["pre"]["id"].asString());
                    if(pResource)
                    {
                        changed.lstRemoved.push_back(pResource);
                    }
                }
                break;
            case enumGrain::UNKNOWN:
                pmlLog(pml::LOG_WARN) << "Unknown grain action";
                break;
        }
    }

    if(m_pPoster)
    {
        m_pPoster->_NodeChanged(changed);
    }
}

void ClientApiImpl::GrainUpdateDevice(const std::string& sSourceId, const Json::Value& jsData)
{
    resourcechanges<Device> changed;

    for(Json::ArrayIndex ai = 0; ai < jsData.size(); ai++)
    {
        switch(WorkoutAction(jsData[ai]))
        {
            case enumGrain::ADD:
            case enumGrain::UPDATE:
                changed += AddDevice(sSourceId, jsData[ai]["post"]);
                break;
            case enumGrain::DELETE:
                if(jsData[ai]["pre"]["id"].isString())
                {
                    auto pResource = m_devices.RemoveResource(jsData[ai]["pre"]["id"].asString());
                    if(pResource)
                    {
                        changed.lstRemoved.push_back(pResource);
                    }
                }
                break;
            case enumGrain::UNKNOWN:
                pmlLog(pml::LOG_WARN) << "Unknown grain action";
                break;
        }
    }

    if(m_pPoster)
    {
        m_pPoster->_DeviceChanged(changed);
    }
}

void ClientApiImpl::GrainUpdateSource(const std::string& sSourceId, const Json::Value& jsData)
{
    resourcechanges<Source> changed;

    for(Json::ArrayIndex ai = 0; ai < jsData.size(); ai++)
    {
        switch(WorkoutAction(jsData[ai]))
        {
            case enumGrain::ADD:
            case enumGrain::UPDATE:
                changed += AddSource(sSourceId, jsData[ai]["post"]);
                break;
            case enumGrain::DELETE:
                if(jsData[ai]["pre"]["id"].isString())
                {
                    auto pResource = m_sources.RemoveResource(jsData[ai]["pre"]["id"].asString());
                    if(pResource)
                    {
                        changed.lstRemoved.push_back(pResource);
                    }
                }
                break;
            case enumGrain::UNKNOWN:
                pmlLog(pml::LOG_WARN) << "Unknown grain action";
                break;
        }
    }

    if(m_pPoster)
    {
        m_pPoster->_SourceChanged(changed);
    }
}

void ClientApiImpl::GrainUpdateFlow(const std::string& sSourceId, const Json::Value& jsData)
{
    resourcechanges<Flow> changed;

    pmlLog(pml::LOG_DEBUG) << "NMOS: ClientApi: GrainUpdateFlow: ";
    for(Json::ArrayIndex ai = 0; ai < jsData.size(); ai++)
    {
        switch(WorkoutAction(jsData[ai]))
        {
            case enumGrain::ADD:
            case enumGrain::UPDATE:
                changed += AddFlow(sSourceId, jsData[ai]["post"]);
                break;
            case enumGrain::DELETE:
                if(jsData[ai]["pre"]["id"].isString())
                {
                    auto pResource = m_flows.RemoveResource(jsData[ai]["pre"]["id"].asString());
                    if(pResource)
                    {
                        changed.lstRemoved.push_back(pResource);
                    }
                }
                break;
            case enumGrain::UNKNOWN:
                pmlLog(pml::LOG_WARN) << "Unknown grain action";
                break;
        }
    }

    if(m_pPoster)
    {
        m_pPoster->_FlowChanged(changed);
    }
}

void ClientApiImpl::GrainUpdateSender(const std::string& sSourceId, const Json::Value& jsData)
{
    resourcechanges<Sender> changed;

    for(Json::ArrayIndex ai = 0; ai < jsData.size(); ai++)
    {
        switch(WorkoutAction(jsData[ai]))
        {
            case enumGrain::ADD:
            case enumGrain::UPDATE:
                changed += AddSender(sSourceId, jsData[ai]["post"]);
                break;
            case enumGrain::DELETE:
                if(jsData[ai]["pre"]["id"].isString())
                {
                    auto pResource = m_senders.RemoveResource(jsData[ai]["pre"]["id"].asString());
                    if(pResource)
                    {
                        changed.lstRemoved.push_back(pResource);
                    }
                }
                break;
            case enumGrain::UNKNOWN:
                pmlLog(pml::LOG_WARN) << "Unknown grain action";
                break;
        }
    }

    if(m_pPoster)
    {
        m_pPoster->_SenderChanged(changed);
    }
}

void ClientApiImpl::GrainUpdateReceiver(const std::string& sSourceId, const Json::Value& jsData)
{
    resourcechanges<Receiver> changed;

    for(Json::ArrayIndex ai = 0; ai < jsData.size(); ai++)
    {
        switch(WorkoutAction(jsData[ai]))
        {
            case enumGrain::ADD:
            case enumGrain::UPDATE:
                changed += AddReceiver(sSourceId, jsData[ai]["post"]);
                break;
            case enumGrain::DELETE:
                if(jsData[ai]["pre"]["id"].isString())
                {
                    auto pResource = m_receivers.RemoveResource(jsData[ai]["pre"]["id"].asString());
                    if(pResource)
                    {
                        changed.lstRemoved.push_back(pResource);
                    }
                }
                break;
            case enumGrain::UNKNOWN:
                pmlLog(pml::LOG_WARN) << "Unknown grain action";
                break;
        }
    }

    if(m_pPoster)
    {
        m_pPoster->_ReceiverChanged(changed);
    }
}

void ClientApiImpl::GetSubnetMasks()
{
    #ifdef __GNU__
    struct ifaddrs *interfaces = NULL;
    struct ifaddrs *temp_addr = NULL;
    int success = getifaddrs(&interfaces);
    if (success == 0)
    {
        // Loop through linked list of interfaces
        temp_addr = interfaces;
        while(temp_addr != NULL)
        {
            if(temp_addr->ifa_addr->sa_family == AF_INET)
            {
                std::string sInterface(temp_addr->ifa_name);
                std::string sAddress(inet_ntoa(((sockaddr_in*)temp_addr->ifa_addr)->sin_addr));

                m_mSubnetMasks.insert({url(sAddress), ((sockaddr_in*)temp_addr->ifa_netmask)->sin_addr.s_addr});
            }
            // @todo AF_INET6
            temp_addr = temp_addr->ifa_next;
        }
    }
    // Free memory
    freeifaddrs(interfaces);
    #endif
}
