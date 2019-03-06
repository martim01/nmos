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
#include "sender.h"
#include "receiver.h"
#include "receiver.h"
#include "clientapiposter.h"
#include "utils.h"


using namespace std;

const string ClientApiImpl::STR_RESOURCE[6] = {"node", "device", "source", "flow", "sender", "receiver"};



void ClientThread(ClientApiImpl* pApi)
{
    //start the browser
    ServiceBrowser::Get().AddService("_nmos-node._tcp", pApi->GetClientPoster());
    ServiceBrowser::Get().AddService("_nmos-query._tcp", pApi->GetClientPoster());
    ServiceBrowser::Get().StartBrowser();

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
                default:
                    break;
            }
            pApi->GetNodeDetails();
        }
    }
    pApi->DeleteServiceBrowser();
}

bool VersionChanged(shared_ptr<dnsInstance> pInstance, const string& sVersion)
{
    if(pInstance->nUpdate != 0)
    {
        map<string, string>::iterator itVer = pInstance->mTxt.find(sVersion);
        map<string, string>::iterator itVerLast = pInstance->mTxtLast.find(sVersion);

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

static void NodeBrowser(ClientApiImpl* pApi, shared_ptr<dnsInstance> pInstance)
{
    map<string, string>::iterator itVersion = pInstance->mTxt.find("api_ver");
    if(itVersion != pInstance->mTxt.end())
    {
        if(itVersion->second.find("v1.2") != string::npos)
        {   //@todo allow choosing of version I guess
            stringstream ss;
            ss << pInstance->sHostIP << ":" << pInstance->nPort << "/x-nmos/node/v1.2/";
            string sResponse;
            int nFlags = pApi->GetInterestFlags();

            if(VersionChanged(pInstance, "ver_slf") && (nFlags&ClientApi::NODES))
            {
                CurlRegister::Get(string(ss.str()+"self"), sResponse, true);
                list<shared_ptr<Self> > lstAdded;
                list<shared_ptr<Self> > lstUpdated;
                list<shared_ptr<Self> > lstRemoved;
                pApi->AddNode(lstAdded, lstUpdated, pInstance->sHostIP, sResponse);

                if(pApi->RunQuery(lstAdded, lstUpdated, lstRemoved, ClientApiImpl::NODES))
                {
                    pApi->GetPoster()->_NodeChanged(lstAdded, lstUpdated, lstRemoved);
                }
            }
            if(VersionChanged(pInstance, "ver_dvc") && (nFlags&ClientApi::DEVICES))
            {
                CurlRegister::Get(string(ss.str()+"devices"), sResponse, true);

                list<shared_ptr<Device> > lstAdded;
                list<shared_ptr<Device> > lstUpdated;
                list<shared_ptr<Device> > lstRemoved;
                //store the devices this ip address currently provides
                pApi->StoreDevices(pInstance->sHostIP);
                //add or update all devices on this ip address
                pApi->AddDevices(lstAdded, lstUpdated, pInstance->sHostIP, sResponse);
                //remove any devices that we previously stored but didn;t update. i.e. ones that no longer exist
                pApi->RemoveStaleDevices(lstRemoved);

                if(pApi->RunQuery(lstAdded, lstUpdated, lstRemoved, ClientApiImpl::DEVICES))
                {
                    pApi->GetPoster()->_DeviceChanged(lstAdded, lstUpdated, lstRemoved);
                }
            }
            if(VersionChanged(pInstance, "ver_src") && (nFlags&ClientApi::SOURCES))
            {
                CurlRegister::Get(string(ss.str()+"sources"), sResponse, true);

                list<shared_ptr<Source> > lstAdded;
                list<shared_ptr<Source> > lstUpdated;
                list<shared_ptr<Source> > lstRemoved;

                pApi->StoreSources(pInstance->sHostIP);
                pApi->AddSources(lstAdded, lstUpdated, pInstance->sHostIP, sResponse);
                pApi->RemoveStaleSources(lstRemoved);

                if(pApi->RunQuery(lstAdded, lstUpdated, lstRemoved, ClientApiImpl::SOURCES))
                {
                    pApi->GetPoster()->_SourceChanged(lstAdded, lstUpdated, lstRemoved);
                }
            }
            if(VersionChanged(pInstance, "ver_flw") && (nFlags&ClientApi::FLOWS))
            {
                CurlRegister::Get(string(ss.str()+"flows"), sResponse, true);

                list<shared_ptr<Flow> > lstAdded;
                list<shared_ptr<Flow> > lstUpdated;
                list<shared_ptr<Flow> > lstRemoved;

                pApi->StoreFlows(pInstance->sHostIP);
                pApi->AddFlows(lstAdded, lstUpdated, pInstance->sHostIP, sResponse);
                pApi->RemoveStaleFlows(lstRemoved);

                if(pApi->RunQuery(lstAdded, lstUpdated, lstRemoved, ClientApiImpl::FLOWS))
                {
                    pApi->GetPoster()->_FlowChanged(lstAdded, lstUpdated, lstRemoved);
                }
            }
            if(VersionChanged(pInstance, "ver_snd") && (nFlags&ClientApi::SENDERS))
            {
                CurlRegister::Get(string(ss.str()+"senders"), sResponse, true);

                list<shared_ptr<Sender> > lstAdded;
                list<shared_ptr<Sender> > lstUpdated;
                list<shared_ptr<Sender> > lstRemoved;

                pApi->StoreSenders(pInstance->sHostIP);
                pApi->AddSenders(lstAdded, lstUpdated, pInstance->sHostIP, sResponse);
                pApi->RemoveStaleSenders(lstRemoved);

                if(pApi->RunQuery(lstAdded, lstUpdated, lstRemoved, ClientApiImpl::SENDERS))
                {
                    pApi->GetPoster()->_SenderChanged(lstAdded, lstUpdated, lstRemoved);
                }
            }
            if(VersionChanged(pInstance, "ver_rcv") && (nFlags&ClientApi::RECEIVERS))
            {
                CurlRegister::Get(string(ss.str()+"receivers"), sResponse, true);

                list<shared_ptr<Receiver> > lstAdded;
                list<shared_ptr<Receiver> > lstUpdated;
                list<shared_ptr<Receiver> > lstRemoved;
                pApi->StoreReceivers(pInstance->sHostIP);
                pApi->AddReceivers(lstAdded, lstUpdated, pInstance->sHostIP, sResponse);
                pApi->RemoveStaleReceivers(lstRemoved);

                if(pApi->RunQuery(lstAdded, lstUpdated, lstRemoved, ClientApiImpl::RECEIVERS))
                {
                    pApi->GetPoster()->_ReceiverChanged(lstAdded, lstUpdated, lstRemoved);
                }
            }

        }
    }
    pApi->Signal(ClientApiImpl::CLIENT_SIG_NODE_BROWSED);
}


void ConnectThread(ClientApiImpl* pApi, const string& sSenderId, const string& sReceiverId, const string& sSenderStage, const string& sSenderTransport, const string& sReceiverStage)
{
    // @todo ConnectThread - if a unicast stream then tell the sender where it should be sending stuff
    Json::StyledWriter stw;
    string sResponse;
    connectionSender aCon(connection::FP_ACTIVATION | connection::FP_ENABLE | connection::FP_TRANSPORT_PARAMS);
    aCon.eActivate = connection::ACT_NOW;
    aCon.bMasterEnable = true;
    aCon.tpSender.bRtpEnabled = true;


    int nResult = CurlRegister::PutPatch(sSenderStage, stw.write(aCon.GetJson(ApiVersion(1,0))), sResponse, false, "");
    cout << "Patch Sender: " << nResult << endl;
    if(nResult != 200)
    {
        pApi->HandleConnect(sSenderId, sReceiverId, false, sResponse);
    }
    else
    {
        connectionReceiver aConR(connection::FP_TRANSPORT_FILE | connection::FP_ACTIVATION | connection::FP_ENABLE | connection::FP_ID | connection::FP_TRANSPORT_PARAMS);
        aConR.eActivate = connection::ACT_NOW;
        aConR.bMasterEnable = true;
        aConR.tpReceiver.bRtpEnabled = true;
        aConR.sTransportFileType = "application/sdp";
        aConR.sSenderId = sSenderId;


        nResult = CurlRegister::Get(sSenderTransport, aConR.sTransportFileData, false);
        cout << "Get SDP: " << nResult << endl;
        if(nResult != 200)
        {
            pApi->HandleConnect(sSenderId, sReceiverId, false, aConR.sTransportFileData);
        }
        else
        {
            string sData(stw.write(aConR.GetJson(ApiVersion(1,0))));

            nResult = CurlRegister::PutPatch(sReceiverStage, sData, sResponse, false, "");
            cout << "Patch Receiver: " << sData << endl;
            if(nResult != 200)
            {
                pApi->HandleConnect(sSenderId, sReceiverId, false, sResponse);
            }
            else
            {
                pApi->HandleConnect(sSenderId, sReceiverId, true, sResponse);
            }
        }
    }
}


void DisconnectThread(ClientApiImpl* pApi, const string& sSenderId, const string& sReceiverId, const string& sSenderStage, const string& sSenderTransport, const string& sReceiverStage)
{
    Json::StyledWriter stw;
    string sResponse;
    int nResult(200);
    if(false)
    {// @todo ConnectThread - if a unicast stream then tell the sender to stop sending stuff
        connectionSender aCon(connection::FP_ACTIVATION | connection::FP_ENABLE | connection::FP_TRANSPORT_PARAMS);
        aCon.eActivate = connection::ACT_NOW;
        aCon.bMasterEnable = true;
        aCon.tpSender.bRtpEnabled = true;

        nResult = CurlRegister::PutPatch(sSenderStage, stw.write(aCon.GetJson(ApiVersion(1,0))), sResponse, false, "");
        cout << "Patch Sender: " << nResult << endl;
        if(nResult != 200)
        {
            pApi->HandleConnect(sSenderId, sReceiverId, false, sResponse);
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

    nResult = CurlRegister::PutPatch(sReceiverStage, stw.write(aConR.GetJson(ApiVersion(1,0))), sResponse, false, "");
    cout << "DISCONNECT THREAD Receiver: " << nResult << endl;
    if(nResult != 200)
    {
        pApi->HandleConnect("", sReceiverId, false, sResponse);
    }
    else
    {
        pApi->HandleConnect(sSenderId, sReceiverId, true, sResponse);
    }
}


/************************************************
Class Start
************************************************/

void ClientApiImpl::Start(int nFlags)
{
    if(m_bStarted == false)
    {
        m_bStarted = true;
        m_nFlags = nFlags;
        thread th(ClientThread, this);
        th.detach();
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
    m_pClientPoster(make_shared<ClientPoster>()),
    m_pCurl(new CurlRegister(m_pClientPoster)),
    m_bStarted(false)
{

}

ClientApiImpl::~ClientApiImpl()
{
    Stop();
}

void ClientApiImpl::SetPoster(shared_ptr<ClientApiPoster> pPoster)
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

    unique_lock<mutex> ul(m_mutex);
    return (m_cvBrowse.wait_for(ul, chrono::milliseconds(nMilliseconds)) == cv_status::no_timeout);
}

bool ClientApiImpl::IsRunning()
{
    lock_guard<mutex> lg(m_mutex);
    return m_bRun;
}

void ClientApiImpl::StopRun()
{
    lock_guard<mutex> lg(m_mutex);
    m_bRun = false;

}

shared_ptr<EventPoster> ClientApiImpl::GetClientPoster()
{
    return m_pClientPoster;
}

ClientApiImpl::enumSignal ClientApiImpl::GetSignal()
{
    lock_guard<mutex> lg(m_mutex);
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

void ClientApiImpl::SetInstanceResolved(shared_ptr<dnsInstance> pInstance)
{
    m_mutex.lock();
    m_pInstance = pInstance;
    m_eSignal = CLIENT_SIG_INSTANCE_RESOLVED;
    m_mutex.unlock();
    m_cvBrowse.notify_one();
}

void ClientApiImpl::SetInstanceRemoved(shared_ptr<dnsInstance> pInstance)
{
    m_mutex.lock();
    m_pInstance = pInstance;
    m_eSignal = CLIENT_SIG_INSTANCE_REMOVED;
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

void ClientApiImpl::HandleInstanceResolved()
{
    lock_guard<mutex> lg(m_mutex);
    if(m_pInstance)
    {
        if(m_pInstance->sService == "_nmos-query._tcp")
        {
            Log::Get(Log::LOG_INFO) << "Query node found: " << m_pInstance->sName << endl;
            m_lstResolve.clear();

            //add the node to our priority sorted list of query servers
            map<string, string>::iterator itTxt = m_pInstance->mTxt.find("pri");
            if(itTxt != m_pInstance->mTxt.end())
            {
                try
                {
                    m_mQueryNodes.insert(make_pair(stoi(itTxt->second), m_pInstance));
                    if(m_eMode == MODE_P2P)
                    {   //change the mode away from peer to peer
                        m_eMode = MODE_REGISTRY;
                        if(m_pPoster)
                        {
                            m_pPoster->_ModeChanged(true);
                        }
                    }
                }
                catch(invalid_argument& ia)
                {
                    Log::Get(Log::LOG_ERROR) << "Priority '" << itTxt->second << "' invalid" << endl;
                }
            }
        }
        else if(m_pInstance->sService == "_nmos-node._tcp" && m_eMode == MODE_P2P)
        {
            if(m_pInstance->nUpdate == 0)
            {
                Log::Get(Log::LOG_INFO) << "Nmos node found" << endl;
                m_lstResolve.push_back(m_pInstance);
            }
            else
            {
                Log::Get(Log::LOG_INFO) << "Nmos node updated" << endl;
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
            Log::Get(Log::LOG_INFO) << "Query node: " << m_pInstance->sName << "removed" << endl;
            if(m_eMode == MODE_REGISTRY)
            {   //@todo should we check here if we have another node??
                m_eMode = MODE_P2P;
                if(m_pPoster)
                {
                    m_pPoster->_ModeChanged(false);
                }
            }
        }
        else if(m_pInstance->sService == "_nmos-node._tcp")
        {
            Log::Get(Log::LOG_INFO) << "Nmos node Removed: " << m_pInstance->sName << endl;
            for(list<shared_ptr<dnsInstance> >::iterator itInstance = m_lstResolve.begin(); itInstance != m_lstResolve.end(); ++itInstance)
            {
                if(*itInstance == m_pInstance)
                {
                    m_lstResolve.erase(itInstance);
                    break;
                }
            }
            RemoveResources(m_pInstance->sHostIP);
        }
        m_pInstance = 0;
    }
}


void ClientApiImpl::HandleConnect(const string& sSenderId, const string& sReceiverId, bool bSuccess, const std::string& sResponse)
{
    lock_guard<mutex> lg(m_mutex);
    if(m_pPoster)
    {
        m_pPoster->_RequestConnectResult(sSenderId, sReceiverId, bSuccess, sResponse);
    }
}


void ClientApiImpl::HandleCurlDone()
{
    lock_guard<mutex> lg(m_mutex);
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
    Json::Value jsData;
    Json::Reader jsReader;
    if(jsReader.parse(m_sCurlResponse, jsData))
    {
        if(202 == m_nCurlResult && jsData["id"].isString())
        {
            Log::Get(Log::LOG_DEBUG) << m_nCurlResult << ": " << jsData["id"].asString() << endl;
            m_pPoster->_RequestTargetResult(202, jsData["id"].asString(), m_sCurlResourceId);
            return;
        }

        if(jsData["error"].isString())
        {
            Log::Get(Log::LOG_DEBUG) << m_nCurlResult << ": " << jsData["error"].asString() << endl;
            m_pPoster->_RequestTargetResult(m_nCurlResult, jsData["error"].asString(), m_sCurlResourceId);
            return;
        }
    }
    Log::Get(Log::LOG_DEBUG) << m_nCurlResult << ": " << m_sCurlResponse << endl;
    m_pPoster->_RequestTargetResult(m_nCurlResult, m_sCurlResponse, m_sCurlResourceId);
}

void ClientApiImpl::HandleCurlDonePatchSender()
{
    Json::Value jsData;
    Json::Reader jsReader;
    if(jsReader.parse(m_sCurlResponse, jsData))
    {
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
}

void ClientApiImpl::HandleCurlDonePatchReceiver()
{
    //don't need to check for m_pPoster as check in HandleCurlDon
    Json::Value jsData;
    Json::Reader jsReader;
    if(jsReader.parse(m_sCurlResponse, jsData))
    {
        if(m_nCurlResult != 200 && m_nCurlResult != 202 && jsData["error"].isString())
        {
            m_pPoster->_RequestPatchReceiverResult(m_nCurlResult, jsData["error"].asString(), m_sCurlResourceId);
        }
        else
        {
            m_pPoster->_RequestPatchReceiverResult(m_nCurlResult, m_sCurlResponse, m_sCurlResourceId);
        }
    }
}

void ClientApiImpl::HandleCurlDoneGetSenderStaged()
{
    //don't need to check for m_pPoster as check in HandleCurlDon
    Json::Value jsData;
    Json::Reader jsReader;
    if(jsReader.parse(m_sCurlResponse, jsData))
    {
        if(m_nCurlResult != 200 && jsData["error"].isString())
        {
            m_pPoster->_RequestGetSenderStagedResult(m_nCurlResult, jsData["error"].asString(), m_sCurlResourceId);
        }
        else
        {
            m_pPoster->_RequestGetSenderStagedResult(m_nCurlResult, m_sCurlResponse, m_sCurlResourceId);
        }
    }
}

void ClientApiImpl::HandleCurlDoneGetSenderActive()
{
    //don't need to check for m_pPoster as check in HandleCurlDon
    Json::Value jsData;
    Json::Reader jsReader;
    if(jsReader.parse(m_sCurlResponse, jsData))
    {
        if(m_nCurlResult != 200 && jsData["error"].isString())
        {
            m_pPoster->_RequestGetSenderActiveResult(m_nCurlResult, jsData["error"].asString(), m_sCurlResourceId);
        }
        else
        {
            m_pPoster->_RequestGetSenderActiveResult(m_nCurlResult, m_sCurlResponse, m_sCurlResourceId);
        }
    }
}

void ClientApiImpl::HandlCurlDoneGetSenderTransportFile()
{
    //don't need to check for m_pPoster as check in HandleCurlDon
    Json::Value jsData;
    Json::Reader jsReader;
    if(jsReader.parse(m_sCurlResponse, jsData))
    {
        if(m_nCurlResult != 200 && jsData["error"].isString())
        {
            m_pPoster->_RequestGetSenderTransportFileResult(m_nCurlResult, jsData["error"].asString(), m_sCurlResourceId);
        }
        else
        {
            m_pPoster->_RequestGetSenderTransportFileResult(m_nCurlResult, m_sCurlResponse, m_sCurlResourceId);
        }
    }
}

void ClientApiImpl::HandleCurlDoneGetReceiverStaged()
{
    //don't need to check for m_pPoster as check in HandleCurlDon
    Json::Value jsData;
    Json::Reader jsReader;
    if(jsReader.parse(m_sCurlResponse, jsData))
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
    Json::Value jsData;
    Json::Reader jsReader;
    if(jsReader.parse(m_sCurlResponse, jsData))
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
        m_nCurlThreadCount++;
        //need to get all the node details.
        //lets launch a thread that will ask for self, devices, sources, flows, senders, receivers
        thread th(NodeBrowser, this, m_lstResolve.front());
        th.detach();
        m_lstResolve.erase(m_lstResolve.begin());
    }
}

ClientApiImpl::enumMode ClientApiImpl::GetMode()
{
    lock_guard<mutex> lg(m_mutex);
    return m_eMode;
}



void ClientApiImpl::AddNode(std::list<std::shared_ptr<Self> >& lstAdded, std::list<std::shared_ptr<Self> >& lstUpdated,const string& sIpAddress, const string& sData)
{
    lock_guard<mutex> lg(m_mutex);

    Json::Value jsData;
    Json::Reader jsReader;
    if(jsReader.parse(sData, jsData))
    {
        if(jsData["id"].isString())
        {
            shared_ptr<Self> pSelf = m_nodes.UpdateResource(jsData);
            if(!pSelf)
            {
                pSelf = make_shared<Self>();
                if(pSelf->UpdateFromJson(jsData))
                {
                    m_nodes.AddResource(sIpAddress, pSelf);
                    lstAdded.push_back(pSelf);

                    Log::Get() << "Node: " << pSelf->GetId() << " found at " << sIpAddress << endl;
                }
                else
                {
                    Log::Get() << "Found node but json data incorrect: " << pSelf->GetJsonParseError() << endl;
                }
            }
            else
            {
                lstUpdated.push_back(pSelf);
            }
        }
        else
        {
            Log::Get() << "Reply from " << sIpAddress << "but not valid JSON - id not correct" << endl;
        }
    }
    else
    {
        Log::Get() << "Reply from " << sIpAddress << "but not valid JSON" << endl;
    }
}

void ClientApiImpl::AddDevices(list<shared_ptr<Device> >& lstAdded, list<shared_ptr<Device> >& lstUpdated, const string& sIpAddress, const string& sData)
{
    lock_guard<mutex> lg(m_mutex);

    Json::Value jsData;
    Json::Reader jsReader;
    if(jsReader.parse(sData, jsData))
    {
        if(jsData.isArray())
        {
            for(Json::ArrayIndex ai = 0; ai < jsData.size(); ++ai)
            {
                if(jsData[ai].isObject() && jsData[ai]["id"].isString())
                {
                    shared_ptr<Device> pResource = m_devices.UpdateResource(jsData[ai]);
                    if(!pResource)
                    {
                        pResource = make_shared<Device>();
                        if(pResource->UpdateFromJson(jsData[ai]))
                        {
                            m_devices.AddResource(sIpAddress, pResource);
                            lstAdded.push_back(pResource);
                            Log::Get() << "Device: " << pResource->GetId() << " found at " << sIpAddress << endl;
                        }
                        else
                        {
                            Log::Get() << "Found device but json data incorrect: " << pResource->GetJsonParseError() << endl;
                        }
                    }
                    else
                    {
                        lstUpdated.push_back(pResource);
                    }
                }
                else
                {
                    Log::Get() << "Reply from " << sIpAddress << "but not JSON is not an array of objects" << endl;
                }
            }
        }
        else
        {
            Log::Get() << "Reply from " << sIpAddress << "but not JSON is not array" << endl;
        }
    }
    else
    {
        Log::Get() << "Reply from " << sIpAddress << "but not valid JSON" << endl;
    }
}

void ClientApiImpl::AddSources(list<shared_ptr<Source> >& lstAdded, list<shared_ptr<Source> >& lstUpdated, const string& sIpAddress, const string& sData)
{
    lock_guard<mutex> lg(m_mutex);
    Json::Value jsData;
    Json::Reader jsReader;
    if(jsReader.parse(sData, jsData))
    {
        if(jsData.isArray())
        {
            for(Json::ArrayIndex ai = 0; ai < jsData.size(); ++ai)
            {
                if(jsData[ai].isObject() && jsData[ai]["format"].isString() && jsData[ai]["id"].isString())
                {
                    shared_ptr<Source> pSourceCore = m_sources.UpdateResource(jsData[ai]);
                    if(!pSourceCore)
                    {
                        if(jsData[ai]["format"].asString().find("urn:x-nmos:format:audio") != string::npos)
                        {   //Audio
                            shared_ptr<SourceAudio> pResource = make_shared<SourceAudio>();
                            if(pResource->UpdateFromJson(jsData[ai]))
                            {
                                m_sources.AddResource(sIpAddress, pResource);
                                lstAdded.push_back(pResource);
                                Log::Get() << "SourceAudio: " << pResource->GetId() << " found at " << sIpAddress << endl;
                            }
                            else
                            {
                                Log::Get() << "Found Source but json data incorrect: " << pResource->GetJsonParseError() << endl;
                            }
                        }
                        else
                        {   //Generic
                            shared_ptr<SourceGeneric> pResource = make_shared<SourceGeneric>();
                            if(pResource->UpdateFromJson(jsData[ai]))
                            {
                                m_sources.AddResource(sIpAddress, pResource);

                                lstAdded.push_back(pResource);
                                Log::Get() << "SourceGeneric: " << pResource->GetId() << " found at " << sIpAddress << endl;
                            }
                            else
                            {
                                Log::Get() << "Found Source but json data incorrect: " << pResource->GetJsonParseError() << endl;
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
                    Log::Get() << "Reply from " << sIpAddress << "but not JSON 'format' is ill defined" << endl;
                }
            }
        }
        else
        {
            Log::Get() << "Reply from " << sIpAddress << "but not JSON is not array" << endl;
        }
    }
    else
    {
        Log::Get() << "Reply from " << sIpAddress << "but not valid JSON" << endl;
    }
}

void ClientApiImpl::AddFlows(list<shared_ptr<Flow> >& lstAdded, list<shared_ptr<Flow> >& lstUpdated, const string& sIpAddress, const string& sData)
{
    lock_guard<mutex> lg(m_mutex);
    Json::Value jsData;
    Json::Reader jsReader;
    if(jsReader.parse(sData, jsData))
    {
        if(jsData.isArray())
        {
            for(Json::ArrayIndex ai = 0; ai < jsData.size(); ++ai)
            {
                if(jsData[ai].isObject() && jsData[ai]["format"].isString() && jsData[ai]["id"].isString())
                {
                    shared_ptr<Flow> pFlowCore = m_flows.UpdateResource(jsData[ai]);
                    if(!pFlowCore)
                    {
                        if(jsData[ai]["format"].asString().find("urn:x-nmos:format:audio") != string::npos)
                        {
                            if(jsData[ai]["media_type"].isString())
                            {
                                if(jsData[ai]["media_type"].asString() == "audio/L24" || jsData[ai]["media_type"].asString() == "audio/L20" || jsData[ai]["media_type"].asString() == "audio/L16" || jsData[ai]["media_type"].asString() == "audio/L8")
                                {   //Raw Audio
                                    shared_ptr<FlowAudioRaw> pResource = make_shared<FlowAudioRaw>();
                                    if(pResource->UpdateFromJson(jsData[ai]))
                                    {
                                        m_flows.AddResource(sIpAddress, pResource);
                                        lstAdded.push_back(pResource);
                                        Log::Get() << "FlowAudioRaw: " << pResource->GetId() << " found at " << sIpAddress << endl;
                                    }
                                    else
                                    {
                                        Log::Get() << "Found Flow but json data incorrect: " << pResource->GetJsonParseError() << endl;
                                    }
                                }
                                else
                                {   //Code audio
                                    shared_ptr<FlowAudioCoded> pResource = make_shared<FlowAudioCoded>();
                                    if(pResource->UpdateFromJson(jsData[ai]))
                                    {
                                        m_flows.AddResource(sIpAddress, pResource);
                                        lstAdded.push_back(pResource);
                                        Log::Get() << "FlowAudioCoded: " << pResource->GetId() << " found at " << sIpAddress << endl;
                                    }
                                    else
                                    {
                                        Log::Get() << "Found Flow but json data incorrect: " << pResource->GetJsonParseError() << endl;
                                    }
                                }
                            }
                        }
                        else if(jsData[ai]["format"].asString().find("urn:x-nmos:format:video") != string::npos)
                        {
                            if(jsData[ai]["media_type"].isString())
                            {//Coded vidoe
                                if(jsData[ai]["media_type"].asString() == "vidoe/raw")
                                {
                                    shared_ptr<FlowVideoRaw> pResource = make_shared<FlowVideoRaw>();
                                    if(pResource->UpdateFromJson(jsData[ai]))
                                    {
                                        m_flows.AddResource(sIpAddress, pResource);
                                        lstAdded.push_back(pResource);
                                        Log::Get() << "FlowVideoRaw: " << pResource->GetId() << " found at " << sIpAddress << endl;
                                    }
                                    else
                                    {
                                        Log::Get() << "Found Flow but json data incorrect: " << pResource->GetJsonParseError() << endl;
                                    }
                                }
                                else
                                {
                                    shared_ptr<FlowVideoCoded> pResource = make_shared<FlowVideoCoded>(jsData[ai]["media_type"].asString());
                                    if(pResource->UpdateFromJson(jsData[ai]))
                                    {
                                        m_flows.AddResource(sIpAddress, pResource);
                                        lstAdded.push_back(pResource);
                                        Log::Get() << "FlowVideoCoded: " << pResource->GetId() << " found at " << sIpAddress << endl;
                                    }
                                    else
                                    {
                                        Log::Get() << "Found Flow but json data incorrect: " << pResource->GetJsonParseError() << endl;
                                    }
                                }
                            }
                        }
                        else if(jsData[ai]["format"].asString().find("urn:x-nmos:format:data") != string::npos)
                        {
                            if(jsData[ai]["media_type"] == "video/smpte291")
                            {
                                shared_ptr<FlowDataSdiAnc> pResource = make_shared<FlowDataSdiAnc>();
                                if(pResource->UpdateFromJson(jsData[ai]))
                                {
                                    m_flows.AddResource(sIpAddress, pResource);
                                    lstAdded.push_back(pResource);
                                    Log::Get() << "FlowDataSdiAnc: " << pResource->GetId() << " found at " << sIpAddress << endl;
                                }
                                else
                                {
                                    Log::Get() << "Found Flow but json data incorrect: " << pResource->GetJsonParseError() << endl;
                                }
                            }
                            //SDIAncData only at the moment
                        }
                        else if(jsData[ai]["format"].asString().find("urn:x-nmos:format:mux") != string::npos)
                        {
                            //Mux only at the momemnt
                            shared_ptr<FlowMux> pResource = make_shared<FlowMux>();
                            if(pResource->UpdateFromJson(jsData[ai]))
                            {
                                m_flows.AddResource(sIpAddress, pResource);
                                lstAdded.push_back(pResource);
                                Log::Get() << "FlowMux: " << pResource->GetId() << " found at " << sIpAddress << endl;
                            }
                            else
                            {
                                Log::Get() << "Found Flow but json data incorrect: " << pResource->GetJsonParseError() << endl;
                            }
                        }
                    }
                    else
                    {
                        lstUpdated.push_back(pFlowCore);
                    }
                }
                else
                {
                    Log::Get() << "Reply from " << sIpAddress << "but not JSON 'format' is ill defined" << endl;
                }
            }
        }
        else
        {
            Log::Get() << "Reply from " << sIpAddress << "but not JSON is not array" << endl;
        }
    }
    else
    {
        Log::Get() << "Reply from " << sIpAddress << "but not valid JSON" << endl;
    }
}

void ClientApiImpl::AddSenders(list<shared_ptr<Sender> >& lstAdded, list<shared_ptr<Sender> >& lstUpdated, const string& sIpAddress, const string& sData)
{
    lock_guard<mutex> lg(m_mutex);
    Json::Value jsData;
    Json::Reader jsReader;
    if(jsReader.parse(sData, jsData))
    {
        if(jsData.isArray())
        {
            for(Json::ArrayIndex ai = 0; ai < jsData.size(); ++ai)
            {
                if(jsData[ai].isObject() && jsData[ai]["id"].isString())
                {
                    shared_ptr<Sender> pResource = m_senders.UpdateResource(jsData[ai]);
                    if(!pResource)
                    {
                        pResource = make_shared<Sender>();
                        if(pResource->UpdateFromJson(jsData[ai]))
                        {
                            m_senders.AddResource(sIpAddress, pResource);
                            lstAdded.push_back(pResource);
                            Log::Get() << "Sender: " << pResource->GetId() << " found at " << sIpAddress << endl;
                        }
                        else
                        {
                            Log::Get() << "Found Sender but json data incorrect: " << pResource->GetJsonParseError() << endl;
                        }
                    }
                    else
                    {
                        lstUpdated.push_back(pResource);
                    }
                }
                else
                {
                    Log::Get() << "Reply from " << sIpAddress << "but not JSON is not an array of objects" << endl;
                }
            }
        }
        else
        {
            Log::Get() << "Reply from " << sIpAddress << "but not JSON is not array" << endl;
        }
    }
    else
    {
        Log::Get() << "Reply from " << sIpAddress << "but not valid JSON" << endl;
    }
}

void ClientApiImpl::AddReceivers(list<shared_ptr<Receiver> >& lstAdded, list<shared_ptr<Receiver> >& lstUpdated, const string& sIpAddress, const string& sData)
{
    lock_guard<mutex> lg(m_mutex);
    Json::Value jsData;
    Json::Reader jsReader;
    if(jsReader.parse(sData, jsData))
    {
        if(jsData.isArray())
        {
            for(Json::ArrayIndex ai = 0; ai < jsData.size(); ++ai)
            {
                if(jsData[ai].isObject() && jsData[ai]["id"].isString())
                {
                    shared_ptr<Receiver> pResource = m_receivers.UpdateResource(jsData[ai]);
                    if(!pResource)
                    {
                        pResource = make_shared<Receiver>();
                        if(pResource->UpdateFromJson(jsData[ai]))
                        {
                            m_receivers.AddResource(sIpAddress, pResource);
                            lstAdded.push_back(pResource);
                            Log::Get() << "Receiver: " << pResource->GetId() << " found at " << sIpAddress << endl;
                        }
                        else
                        {
                            Log::Get() << "Found Receiver but json data incorrect: " << pResource->GetJsonParseError() << endl;
                        }
                    }
                    else
                    {
                        lstUpdated.push_back(pResource);
                    }
                }
                else
                {
                    Log::Get() << "Reply from " << sIpAddress << "but not JSON is not an array of objects" << endl;
                }
            }
        }
        else
        {
            Log::Get() << "Reply from " << sIpAddress << "but not JSON is not array" << endl;
        }
    }
    else
    {
        Log::Get() << "Reply from " << sIpAddress << "but not valid JSON" << endl;
    }
}


void ClientApiImpl::RemoveResources(const string& sIpAddress)
{
    list<shared_ptr<Self> > lstSelf;
    list<shared_ptr<Device> > lstDevice;
    list<shared_ptr<Source> > lstSource;
    list<shared_ptr<Flow> > lstFlow;
    list<shared_ptr<Sender> > lstSender;
    list<shared_ptr<Receiver> > lstReceiver;

    m_nodes.RemoveResources(sIpAddress, lstSelf);
    m_devices.RemoveResources(sIpAddress, lstDevice);
    m_sources.RemoveResources(sIpAddress, lstSource);
    m_flows.RemoveResources(sIpAddress, lstFlow);
    m_senders.RemoveResources(sIpAddress, lstSender);
    m_receivers.RemoveResources(sIpAddress, lstReceiver);

    //@todo run past query subscriptions and tell the client

}

void ClientApiImpl::NodeDetailsDone()
{
    m_nCurlThreadCount--;
}


void ClientApiImpl::DeleteServiceBrowser()
{

}


void ClientApiImpl::ChangeInterest(int nFlags)
{
    lock_guard<mutex> lg(m_mutex);
    m_nFlags = nFlags;
}

int ClientApiImpl::GetInterestFlags()
{
    lock_guard<mutex> lg(m_mutex);
    return m_nFlags;
}


map<string, shared_ptr<Self> >::const_iterator ClientApiImpl::GetNodeBegin()
{
    return m_nodes.GetResourceBegin();
}

map<string, shared_ptr<Self> >::const_iterator ClientApiImpl::GetNodeEnd()
{
    return m_nodes.GetResourceEnd();
}

map<string, shared_ptr<Self> >::const_iterator ClientApiImpl::FindNode(const string& sUid)
{
    return m_nodes.FindNmosResource(sUid);
}

map<string, shared_ptr<Device> >::const_iterator ClientApiImpl::GetDeviceBegin()
{
    return m_devices.GetResourceBegin();
}

map<string, shared_ptr<Device> >::const_iterator ClientApiImpl::GetDeviceEnd()
{
    return m_devices.GetResourceEnd();
}

map<string, shared_ptr<Device> >::const_iterator ClientApiImpl::FindDevice(const string& sUid)
{
    return m_devices.FindNmosResource(sUid);
}


map<string, shared_ptr<Source> >::const_iterator ClientApiImpl::GetSourceBegin()
{
    return m_sources.GetResourceBegin();
}

map<string, shared_ptr<Source> >::const_iterator ClientApiImpl::GetSourceEnd()
{
    return m_sources.GetResourceEnd();
}

map<string, shared_ptr<Source> >::const_iterator ClientApiImpl::FindSource(const string& sUid)
{
    return m_sources.FindNmosResource(sUid);
}


map<string, shared_ptr<Flow> >::const_iterator ClientApiImpl::GetFlowBegin()
{
    return m_flows.GetResourceBegin();
}

map<string, shared_ptr<Flow> >::const_iterator ClientApiImpl::GetFlowEnd()
{
    return m_flows.GetResourceEnd();
}

map<string, shared_ptr<Flow> >::const_iterator ClientApiImpl::FindFlow(const string& sUid)
{
    return m_flows.FindNmosResource(sUid);
}



map<string, shared_ptr<Sender> >::const_iterator ClientApiImpl::GetSenderBegin()
{
    return m_senders.GetResourceBegin();
}

map<string, shared_ptr<Sender> >::const_iterator ClientApiImpl::GetSenderEnd()
{
    return m_senders.GetResourceEnd();
}

map<string, shared_ptr<Sender> >::const_iterator ClientApiImpl::FindSender(const string& sUid)
{
    return m_senders.FindNmosResource(sUid);
}



map<string, shared_ptr<Receiver> >::const_iterator ClientApiImpl::GetReceiverBegin()
{
    return m_receivers.GetResourceBegin();
}

map<string, shared_ptr<Receiver> >::const_iterator ClientApiImpl::GetReceiverEnd()
{
    return m_receivers.GetResourceEnd();
}

map<string, shared_ptr<Receiver> >::const_iterator ClientApiImpl::FindReceiver(const string& sUid)
{
    return m_receivers.FindNmosResource(sUid);
}


bool ClientApiImpl::Subscribe(const string& sSenderId, const string& sReceiverId)
{
    lock_guard<mutex> lg(m_mutex);

    shared_ptr<Sender> pSender = GetSender(sSenderId);
    shared_ptr<Receiver> pReceiver = GetReceiver(sReceiverId);
    if(!pSender || !pReceiver)
    {
        return false;
    }

    ApiVersion version(0,0);
    string sUrl(GetTargetUrl(pReceiver, version));
    if(sUrl.empty() == false)
    {
        //do a PUT to the correct place on the URL
        Json::StyledWriter sw;
        string sJson(sw.write(pSender->GetJson(version)));
        m_pCurl->PutPatch(sUrl, sJson, ClientPoster::CURLTYPE_TARGET, true, pReceiver->GetId());
        Log::Get(Log::LOG_DEBUG) << "TARGET: " << sUrl << endl;
        return true;
    }
    Log::Get(Log::LOG_ERROR) << "Couldn't create target url" << endl;
    return false;
}

bool ClientApiImpl::Unsubscribe(const string& sReceiverId)
{
    lock_guard<mutex> lg(m_mutex);
    shared_ptr<Receiver> pReceiver = GetReceiver(sReceiverId);
    if(!pReceiver)
    {
        return false;
    }

    ApiVersion version(0,0);
    string sUrl(GetTargetUrl(pReceiver, version));
    if(sUrl.empty() == false)
    {
        Log::Get(Log::LOG_DEBUG) << "TARGET: " << sUrl << endl;
        //do a PUT to the correct place on the URL
        m_pCurl->PutPatch(sUrl, "{}", ClientPoster::CURLTYPE_TARGET, true, pReceiver->GetId());

        return true;
    }
    return false;
}

string ClientApiImpl::GetTargetUrl(shared_ptr<Receiver> pReceiver, ApiVersion& version)
{
    //get the device id
    map<string, shared_ptr<Device> >::const_iterator itDevice =  m_devices.FindNmosResource(pReceiver->GetParentResourceId());
    if(itDevice == m_devices.GetResourceEnd())
    {
        Log::Get(Log::LOG_ERROR) << "Device: " << pReceiver->GetParentResourceId() << " not found" << endl;
        return string();
    }

    map<string, shared_ptr<Self> >::const_iterator itNode =  m_nodes.FindNmosResource(itDevice->second->GetParentResourceId());
    if(itNode == m_nodes.GetResourceEnd())
    {
        Log::Get(Log::LOG_ERROR) << "Node: " << itDevice->second->GetParentResourceId() << " not found" << endl;
        return string();
    }

    //decide on the version to use - for now get highest v1.x @todo deciding on version should possibly work in a user defined way
    version = ApiVersion(0,0);

    for(set<ApiVersion>::const_iterator itVersion = itNode->second->GetApiVersionBegin(); itVersion != itNode->second->GetApiVersionEnd(); ++itVersion)
    {
        Log::Get(Log::LOG_DEBUG) << itVersion->GetVersionAsString() << endl;
        if((*itVersion).GetMajor() == 1)
        {
            version = (*itVersion);
        }
        else if((*itVersion).GetMajor() > 1)
        {
            break;
        }
    }
    if(version.GetMajor() == 0)
    {
        Log::Get(Log::LOG_ERROR) << "Version 1.x not found" << endl;
        return string();
    }
    //get the endpoint to use... for now the first non https one @todo deciding on endpoint should probably work in a better way
    set<endpoint>::const_iterator itEndpoint = itNode->second->GetEndpointsBegin();
    for(; itEndpoint != itNode->second->GetEndpointsEnd(); ++itEndpoint)
    {
        if(itEndpoint->bSecure == false)
        {
            break;
        }
    }
    if(itEndpoint == itNode->second->GetEndpointsEnd())
    {
        Log::Get(Log::LOG_ERROR) << "Non-secure endpoint not found" << endl;
        return string();
    }

    //now we can build our url...
    stringstream ssurl;
    ssurl << itEndpoint->sHost << ":" << itEndpoint->nPort << "/x-nmos/node/" << version.GetVersionAsString() << "/receivers/" << pReceiver->GetId() << "/target";

    return ssurl.str();

}



string ClientApiImpl::GetConnectionUrlSingle(shared_ptr<Resource> pResource, const string& sDirection, const string& sEndpoint, ApiVersion& version)
{
    map<string, shared_ptr<Device> >::const_iterator itDevice =  m_devices.FindNmosResource(pResource->GetParentResourceId());
    if(itDevice == m_devices.GetResourceEnd())
    {
        Log::Get(Log::LOG_ERROR) << "Device: " << pResource->GetParentResourceId() << " not found" << endl;
        return string();
    }

    for(set<pair<string, string> >::const_iterator itControl = itDevice->second->GetControlsBegin(); itControl != itDevice->second->GetControlsEnd(); ++itControl)
    {
        if("urn:x-nmos:control:sr-ctrl/v1.0" == itControl->first)
        {
            version = ApiVersion(1,0);
            stringstream ssUrl;
            ssUrl << itControl->second << "/single/" << sDirection << "/" << pResource->GetId() << "/" << sEndpoint;
            return ssUrl.str();
        }
    }
    version = ApiVersion(0,0);
    return string();
}

bool ClientApiImpl::RequestSender(const string& sSenderId, ClientPoster::enumCurlType eType)
{
    lock_guard<mutex> lg(m_mutex);
    shared_ptr<Sender> pSender = GetSender(sSenderId);
    if(!pSender)
    {
        return false;
    }

    ApiVersion version(0,0);
    string sConnectionUrl(GetConnectionUrlSingle(pSender, "senders", ClientPoster::STR_TYPE[eType], version));
    if(sConnectionUrl.empty())
    {
        return false;
    }

    m_pCurl->Get(sConnectionUrl, eType);
    return true;
}

bool ClientApiImpl::RequestReceiver(const string& sReceiverId, ClientPoster::enumCurlType eType)
{
    lock_guard<mutex> lg(m_mutex);
    shared_ptr<Receiver> pReceiver = GetReceiver(sReceiverId);
    if(!pReceiver)
    {
        return false;
    }

    ApiVersion version(0,0);
    string sConnectionUrl(GetConnectionUrlSingle(pReceiver, "receivers", ClientPoster::STR_TYPE[eType], version));
    if(sConnectionUrl.empty())
    {
        return false;
    }

    m_pCurl->Get(sConnectionUrl, eType);
    return true;
}

bool ClientApiImpl::RequestSenderStaged(const string& sSenderId)
{
    return RequestSender(sSenderId, ClientPoster::CURLTYPE_SENDER_STAGED);
}

bool ClientApiImpl::RequestSenderActive(const string& sSenderId)
{
    return RequestSender(sSenderId, ClientPoster::CURLTYPE_SENDER_ACTIVE);
}

bool ClientApiImpl::RequestSenderTransportFile(const string& sSenderId)
{
    return RequestSender(sSenderId, ClientPoster::CURLTYPE_SENDER_TRANSPORTFILE);
}

bool ClientApiImpl::RequestReceiverStaged(const string& sReceiverId)
{
    return RequestReceiver(sReceiverId, ClientPoster::CURLTYPE_RECEIVER_STAGED);
}

bool ClientApiImpl::RequestReceiverActive(const string& sReceiverId)
{
    return RequestReceiver(sReceiverId, ClientPoster::CURLTYPE_RECEIVER_ACTIVE);
}


bool ClientApiImpl::PatchSenderStaged(const string& sSenderId, const connectionSender& aConnection)
{
    lock_guard<mutex> lg(m_mutex);
    shared_ptr<Sender> pSender = GetSender(sSenderId);
    if(!pSender)
    {
        return false;
    }

    ApiVersion version(0,0);
    string sConnectionUrl(GetConnectionUrlSingle(pSender, "senders", ClientPoster::STR_TYPE[ClientPoster::CURLTYPE_SENDER_STAGED], version));
    if(sConnectionUrl.empty())
    {
        return false;
    }

    Json::StyledWriter sw;
    string sJson = sw.write(aConnection.GetJson(version));
    Log::Get(Log::LOG_DEBUG) << sJson << endl;
    m_pCurl->PutPatch(sConnectionUrl, sJson, ClientPoster::CURLTYPE_SENDER_PATCH, false, sSenderId);

    return true;
}

bool ClientApiImpl::PatchReceiverStaged(const string& sReceiverId, const connectionReceiver& aConnection)
{
    lock_guard<mutex> lg(m_mutex);
    shared_ptr<Receiver> pReceiver = GetReceiver(sReceiverId);
    if(!pReceiver)
    {
        return false;
    }

    ApiVersion version(0,0);
    string sConnectionUrl(GetConnectionUrlSingle(pReceiver, "receivers", ClientPoster::STR_TYPE[ClientPoster::CURLTYPE_SENDER_STAGED], version));
    if(sConnectionUrl.empty())
    {
        return false;
    }

    Json::StyledWriter sw;
    string sJson = sw.write(aConnection.GetJson(version));
    m_pCurl->PutPatch(sConnectionUrl, sJson, ClientPoster::CURLTYPE_RECEIVER_PATCH, false, sReceiverId);

    return true;
}


shared_ptr<Sender> ClientApiImpl::GetSender(const string& sSenderId)
{
    map<string, shared_ptr<Sender> >::const_iterator itSender = m_senders.FindNmosResource(sSenderId);
    if(itSender == m_senders.GetResourceEnd())
    {
        Log::Get(Log::LOG_ERROR) << "Sender: " << sSenderId << " not found." << endl;
        return 0;
    }
    return itSender->second;
}

shared_ptr<Receiver> ClientApiImpl::GetReceiver(const string& sReceiverId)
{
    map<string, shared_ptr<Receiver> >::const_iterator itReceiver = m_receivers.FindNmosResource(sReceiverId);
    if(itReceiver == m_receivers.GetResourceEnd())
    {
        Log::Get(Log::LOG_ERROR) << "Receiver: " << sReceiverId << " not found." << endl;
        return 0;
    }
    return itReceiver->second;
}


void ClientApiImpl::StoreDevices(const string& sIpAddress)
{
    m_devices.StoreResources(sIpAddress);
}

void ClientApiImpl::StoreSources(const string& sIpAddress)
{
    m_sources.StoreResources(sIpAddress);
}

void ClientApiImpl::StoreFlows(const string& sIpAddress)
{
    m_flows.StoreResources(sIpAddress);
}

void ClientApiImpl::StoreSenders(const string& sIpAddress)
{
    m_senders.StoreResources(sIpAddress);
}

void ClientApiImpl::StoreReceivers(const string& sIpAddress)
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

void ClientApiImpl::RemoveStaleSenders(std::list<std::shared_ptr<Sender> >& lstRemoved)
{
    m_senders.RemoveStaleResources(lstRemoved);
}

void ClientApiImpl::RemoveStaleReceivers(std::list<std::shared_ptr<Receiver> >& lstRemoved)
{
    m_receivers.RemoveStaleResources(lstRemoved);
}

bool ClientApiImpl::Connect(const std::string& sSenderId, const std::string& sReceiverId)
{
    lock_guard<mutex> lg(m_mutex);
    shared_ptr<Sender> pSender = GetSender(sSenderId);
    shared_ptr<Receiver> pReceiver = GetReceiver(sReceiverId);
    if(!pSender || !pReceiver)
    {
        return false;
    }

    ApiVersion version(0,0);
    string sSenderStageUrl(GetConnectionUrlSingle(pSender, "senders", ClientPoster::STR_TYPE[ClientPoster::CURLTYPE_SENDER_STAGED], version));
    string sSenderTransportUrl(GetConnectionUrlSingle(pSender, "senders", ClientPoster::STR_TYPE[ClientPoster::CURLTYPE_SENDER_TRANSPORTFILE], version));
    string sReceiverStageUrl(GetConnectionUrlSingle(pReceiver, "receivers", ClientPoster::STR_TYPE[ClientPoster::CURLTYPE_RECEIVER_STAGED], version));


    if(sSenderStageUrl.empty() || sSenderTransportUrl.empty() || sReceiverStageUrl.empty())
    {
        return false;
    }

    thread th(ConnectThread, this, sSenderId, sReceiverId, sSenderStageUrl, sSenderTransportUrl, sReceiverStageUrl);
    th.detach();
    return true;
}



bool ClientApiImpl::Disconnect( const std::string& sReceiverId)
{
    lock_guard<mutex> lg(m_mutex);
    Log::Get(Log::LOG_DEBUG) << "ClientApiImpl::Disconnect " << sReceiverId << endl;
    shared_ptr<Receiver> pReceiver = GetReceiver(sReceiverId);
    if(!pReceiver)
    {
        Log::Get(Log::LOG_DEBUG) << "ClientApiImpl::Disconnect No Receiver" << endl;
        return false;
    }
    shared_ptr<Sender> pSender = GetSender(pReceiver->GetSender());
    if(!pSender)
    {
        Log::Get(Log::LOG_DEBUG) << "ClientApiImpl::Disconnect No Sender" << endl;
        return false;
    }

    ApiVersion version(0,0);
    string sSenderStageUrl(GetConnectionUrlSingle(pSender, "senders", ClientPoster::STR_TYPE[ClientPoster::CURLTYPE_SENDER_STAGED], version));
    string sSenderTransportUrl(GetConnectionUrlSingle(pSender, "senders", ClientPoster::STR_TYPE[ClientPoster::CURLTYPE_SENDER_TRANSPORTFILE], version));
    string sReceiverStageUrl(GetConnectionUrlSingle(pReceiver, "receivers", ClientPoster::STR_TYPE[ClientPoster::CURLTYPE_RECEIVER_STAGED], version));


    if(sSenderStageUrl.empty() || sSenderTransportUrl.empty() || sReceiverStageUrl.empty())
    {
        Log::Get(Log::LOG_DEBUG) << "ClientApiImpl::Disconnect No URLS" << endl;
        return false;
    }

    thread th(DisconnectThread, this, pSender->GetId(), sReceiverId, sSenderStageUrl, sSenderTransportUrl, sReceiverStageUrl);
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
    // @todo ClientApiImpl::AddQuerySubscriptionRegistry
}

bool ClientApiImpl::RemoveQuerySubscriptionRegistry(const std::string& sSubscriptionId)
{
    // @todo ClientApiImpl::RemoveQuerySubscriptionRegistry
}

bool ClientApiImpl::AddQuerySubscriptionP2P(int nResource, const std::string& sQuery)
{
    //store the query
    query aQuery;
    aQuery.sId = CreateGuid();
    aQuery.sQuery = sQuery;
    aQuery.nResource = nResource;
    m_mmQuery.insert(make_pair(nResource, aQuery));

    //@todo call the poster to let the client know the id
    if(m_pPoster)
    {
        m_pPoster->_QuerySubscription(aQuery.sId, nResource, sQuery);
    }

    //run the query on our stored nodes and call the poster for each found query
    switch(nResource)
    {
    case NODES:
        {
            list<shared_ptr<Self> > lstResources;
            m_nodes.GetResourcesAsList(lstResources);
            if(RunQuery(lstResources, sQuery))
            {
                m_pPoster->_NodeChanged(lstResources, list<shared_ptr<Self> >(), list<shared_ptr<Self> >());
            }
        }
        break;
    case DEVICES:
        {
            list<shared_ptr<Device> > lstResources;
            m_devices.GetResourcesAsList(lstResources);
            if(RunQuery(lstResources, sQuery))
            {
                m_pPoster->_DeviceChanged(lstResources, list<shared_ptr<Device> >(), list<shared_ptr<Device> >());
            }
        }
        break;
    case SOURCES:
        {
            list<shared_ptr<Source> > lstResources;
            m_sources.GetResourcesAsList(lstResources);
            if(RunQuery(lstResources, sQuery))
            {
                m_pPoster->_SourceChanged(lstResources, list<shared_ptr<Source> >(), list<shared_ptr<Source> >());
            }
        }
        break;
    case FLOWS:
        {
            list<shared_ptr<Flow> > lstResources;
            m_flows.GetResourcesAsList(lstResources);
            if(RunQuery(lstResources, sQuery))
            {
                m_pPoster->_FlowChanged(lstResources, list<shared_ptr<Flow> >(), list<shared_ptr<Flow> >());
            }
        }
        break;
    case SENDERS:
        {
            list<shared_ptr<Sender> > lstResources;
            m_senders.GetResourcesAsList(lstResources);
            if(RunQuery(lstResources, sQuery))
            {
                m_pPoster->_SenderChanged(lstResources, list<shared_ptr<Sender> >(), list<shared_ptr<Sender> >());
            }
        }
        break;
    case RECEIVERS:
        {
            list<shared_ptr<Receiver> > lstResources;
            m_receivers.GetResourcesAsList(lstResources);
            if(RunQuery(lstResources, sQuery))
            {
                m_pPoster->_ReceiverChanged(lstResources, list<shared_ptr<Receiver> >(), list<shared_ptr<Receiver> >());
            }
        }
        break;
    }

}

bool ClientApiImpl::RemoveQuerySubscriptionP2P(const std::string& sSubscriptionId)
{
    for(multimap<int, query>::iterator itQuery = m_mmQuery.begin(); itQuery != m_mmQuery.end(); ++itQuery)
    {
        if(itQuery->second.sId == sSubscriptionId)
        {
            if(m_pPoster)
            {
                m_pPoster->_QuerySubscriptionRemoved(sSubscriptionId);
                m_mmQuery.erase(itQuery);
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
        for(typename list<shared_ptr<T> >::iterator itResource = lstCheck.begin(); itResource != lstCheck.end(); )
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
    return false;
}

template<class T> void ClientApiImpl::RunQuery(std::list<shared_ptr<T> >& lstCheck, int nResource)
{
    for(typename list<shared_ptr<T> >::iterator itResource = lstCheck.begin(); itResource != lstCheck.end(); )
    {
        bool bKeep(false);
        for(multimap<int, query>::iterator itQuery = m_mmQuery.lower_bound(nResource); itQuery != m_mmQuery.upper_bound(nResource); ++itQuery)
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
            itResource = lstCheck.erase(itResource);
        }
    }
}

template<class T> bool ClientApiImpl::RunQuery(list<shared_ptr<T> >& lstAdded, list<shared_ptr<T> >& lstUpdated, list<shared_ptr<T> >& lstRemoved, int nResourceType)
{
    if(m_pPoster)
    {
        RunQuery(lstAdded, nResourceType);
        RunQuery(lstUpdated, nResourceType);
        RunQuery(lstRemoved, nResourceType);
        if(lstAdded.empty() == false || lstUpdated.empty() == false || lstRemoved.empty() == false)
        {
            return true;
        }
    }
    return false;
}

bool ClientApiImpl::MeetsQuery(const std::string& sQuery, shared_ptr<Resource> pResource)
{
    if(sQuery.empty())
    {
        return true;
    }
    // @todo run the query properly
}
