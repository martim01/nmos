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
#include "sourceaudio.h"
#include "sourcegeneric.h"
#include "flowaudiocoded.h"
#include "flowvideocoded.h"
#include "flowaudioraw.h"
#include "flowvideoraw.h"
#include "flowdatasdianc.h"
#include "flowmux.h"
#include "sender.h"
#include "receiver.h"
#include "receiver.h"

using namespace std;

const string ClientApiPrivate::STR_RESOURCE[6] = {"node", "device", "source", "flow", "sender", "receiver"};



void ClientThread(ClientApiPrivate* pApi)
{
    //start the browser
    set<string> setServices;
    setServices.insert("_nmos-node._tcp");
    setServices.insert("_nmos-query._tcp");
    pApi->GetBrowser()->StartBrowser(setServices);

    while(pApi->IsRunning())
    {
        if(pApi->Wait(5000))
        {
            switch(pApi->GetSignal())
            {
                case ClientApiPrivate::CLIENT_SIG_INSTANCE_RESOLVED:
                    pApi->HandleInstanceResolved();
                    break;
                case ClientApiPrivate::CLIENT_SIG_INSTANCE_REMOVED:
                    pApi->HandleInstanceRemoved();
                    break;
                case ClientApiPrivate::CLIENT_SIG_NODE_BROWSED:
                    pApi->NodeDetailsDone();
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

static void NodeBrowser(ClientApiPrivate* pApi, shared_ptr<dnsInstance> pInstance)
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
                pApi->AddNode(pInstance->sHostIP, sResponse);
            }
            if(VersionChanged(pInstance, "ver_dvc") && (nFlags&ClientApi::DEVICES))
            {
                CurlRegister::Get(string(ss.str()+"devices"), sResponse, true);
                pApi->AddDevices(pInstance->sHostIP, sResponse);
            }
            if(VersionChanged(pInstance, "ver_src") && (nFlags&ClientApi::SOURCES))
            {
                CurlRegister::Get(string(ss.str()+"sources"), sResponse, true);
                pApi->AddSources(pInstance->sHostIP, sResponse);
            }
            if(VersionChanged(pInstance, "ver_flw") && (nFlags&ClientApi::FLOWS))
            {
                CurlRegister::Get(string(ss.str()+"flows"), sResponse, true);
                pApi->AddFlows(pInstance->sHostIP, sResponse);
            }
            if(VersionChanged(pInstance, "ver_snd") && (nFlags&ClientApi::SENDERS))
            {
                CurlRegister::Get(string(ss.str()+"senders"), sResponse, true);
                pApi->AddSenders(pInstance->sHostIP, sResponse);
            }
            if(VersionChanged(pInstance, "ver_rcv") && (nFlags&ClientApi::RECEIVERS))
            {
                CurlRegister::Get(string(ss.str()+"receivers"), sResponse, true);
                pApi->AddReceivers(pInstance->sHostIP, sResponse);
            }

        }
    }
    pApi->Signal(ClientApiPrivate::CLIENT_SIG_NODE_BROWSED);
}



void ClientApiPrivate::Start(int nFlags)
{
    m_nFlags = nFlags;
    if(!m_pBrowser)
    {
        shared_ptr<ClientPoster> pPoster(make_shared<ClientPoster>());
        m_pBrowser = new ServiceBrowser(pPoster, false);
        m_pCurl = new CurlRegister(pPoster);
        thread th(ClientThread, this);
        th.detach();
    }
}

void ClientApiPrivate::Stop()
{
    StopRun();
}

ClientApiPrivate::ClientApiPrivate() :
    m_eMode(MODE_P2P),
    m_pBrowser(0),
    m_bRun(true),
    m_pInstance(0),
    m_nCurlThreadCount(0)
{

}

ClientApiPrivate::~ClientApiPrivate()
{
    Stop();
}


bool ClientApiPrivate::Wait(unsigned long nMilliseconds)
{
    m_mutex.lock();
    m_eSignal = CLIENT_SIG_NONE;
    m_mutex.unlock();

    unique_lock<mutex> ul(m_mutex);
    return (m_cvBrowse.wait_for(ul, chrono::milliseconds(nMilliseconds)) == cv_status::no_timeout);
}

bool ClientApiPrivate::IsRunning()
{
    lock_guard<mutex> lg(m_mutex);
    return m_bRun;
}

void ClientApiPrivate::StopRun()
{
    lock_guard<mutex> lg(m_mutex);
    m_bRun = false;

}


ClientApiPrivate::enumSignal ClientApiPrivate::GetSignal()
{
    lock_guard<mutex> lg(m_mutex);
    return m_eSignal;
}



void ClientApiPrivate::SetInstanceResolved(shared_ptr<dnsInstance> pInstance)
{
    m_mutex.lock();
    m_pInstance = pInstance;
    m_eSignal = CLIENT_SIG_INSTANCE_RESOLVED;
    m_mutex.unlock();

    m_cvBrowse.notify_one();
}

void ClientApiPrivate::SetInstanceRemoved(shared_ptr<dnsInstance> pInstance)
{
    m_mutex.lock();
    m_pInstance = pInstance;
    m_eSignal = CLIENT_SIG_INSTANCE_REMOVED;
    m_mutex.unlock();

    m_cvBrowse.notify_one();
}

void ClientApiPrivate::Signal(enumSignal eSignal)
{
    m_mutex.lock();
    m_eSignal = eSignal;
    m_mutex.unlock();

    m_cvBrowse.notify_one();
}

void ClientApiPrivate::HandleInstanceResolved()
{
    lock_guard<mutex> lg(m_mutex);
    if(m_pInstance)
    {
        if(m_pInstance->sService == "_nmos-query._tcp")
        {
            Log::Get(Log::LOG_INFO) << "Query node found" << endl;
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

void ClientApiPrivate::HandleInstanceRemoved()
{
    if(m_pInstance)
    {
        if(m_pInstance->sService == "_nmos_query._tcp")
        {
            Log::Get(Log::LOG_INFO) << "Query node: " << m_pInstance->sName << "removed" << endl;
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


void ClientApiPrivate::ConnectToQueryServer()
{
    // @todo ConnectToQueryServer
}

void ClientApiPrivate::GetNodeDetails()
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

ClientApiPrivate::enumMode ClientApiPrivate::GetMode()
{
    lock_guard<mutex> lg(m_mutex);
    return m_eMode;
}



void ClientApiPrivate::AddNode(const string& sIpAddress, const string& sData)
{
    lock_guard<mutex> lg(m_mutex);

    Json::Value jsData;
    Json::Reader jsReader;
    if(jsReader.parse(sData, jsData))
    {
        if(jsData["id"].isString())
        {
            if(!m_nodes.UpdateResource(jsData))
            {
                shared_ptr<Self> pSelf = (make_shared<Self>());
                if(pSelf->UpdateFromJson(jsData))
                {
                    m_nodes.AddResource(sIpAddress, pSelf);
                    Log::Get() << "Node: " << pSelf->GetId() << " found at " << sIpAddress << endl;
                }
                else
                {
                    Log::Get() << "Found node but json data incorrect: " << pSelf->GetJsonParseError() << endl;
                }
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

void ClientApiPrivate::AddDevices(const string& sIpAddress, const string& sData)
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
                    if(!m_devices.UpdateResource(jsData[ai]))
                    {
                        shared_ptr<Device> pResource(make_shared<Device>());
                        if(pResource->UpdateFromJson(jsData[ai]))
                        {
                            m_devices.AddResource(sIpAddress, pResource);
                            Log::Get() << "Device: " << pResource->GetId() << " found at " << sIpAddress << endl;
                        }
                        else
                        {
                            Log::Get() << "Found device but json data incorrect: " << pResource->GetJsonParseError() << endl;
                        }
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

void ClientApiPrivate::AddSources(const string& sIpAddress, const string& sData)
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
                    if(!m_sources.UpdateResource(jsData[ai]))
                    {
                        if(jsData[ai]["format"].asString().find("urn:x-nmos:format:audio") != string::npos)
                        {   //Audio
                            shared_ptr<SourceAudio> pResource = make_shared<SourceAudio>();
                            if(pResource->UpdateFromJson(jsData[ai]))
                            {
                                m_sources.AddResource(sIpAddress, pResource);
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
                                Log::Get() << "SourceGeneric: " << pResource->GetId() << " found at " << sIpAddress << endl;
                            }
                            else
                            {
                                Log::Get() << "Found Source but json data incorrect: " << pResource->GetJsonParseError() << endl;
                            }
                        }
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

void ClientApiPrivate::AddFlows(const string& sIpAddress, const string& sData)
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
                    if(!m_flows.UpdateResource(jsData[ai]))
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
                                Log::Get() << "FlowMux: " << pResource->GetId() << " found at " << sIpAddress << endl;
                            }
                            else
                            {
                                Log::Get() << "Found Flow but json data incorrect: " << pResource->GetJsonParseError() << endl;
                            }
                        }
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

void ClientApiPrivate::AddSenders(const string& sIpAddress, const string& sData)
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
                    if(!m_senders.UpdateResource(jsData[ai]))
                    {
                        shared_ptr<Sender> pResource(make_shared<Sender>());
                        if(pResource->UpdateFromJson(jsData[ai]))
                        {
                            m_senders.AddResource(sIpAddress, pResource);
                            Log::Get() << "Sender: " << pResource->GetId() << " found at " << sIpAddress << endl;
                        }
                        else
                        {
                            Log::Get() << "Found Sender but json data incorrect: " << pResource->GetJsonParseError() << endl;
                        }
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

void ClientApiPrivate::AddReceivers(const string& sIpAddress, const string& sData)
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
                    if(!m_receivers.UpdateResource(jsData[ai]))
                    {
                        shared_ptr<Receiver> pResource(make_shared<Receiver>());
                        if(pResource->UpdateFromJson(jsData[ai]))
                        {
                            m_receivers.AddResource(sIpAddress, pResource);
                            Log::Get() << "Receiver: " << pResource->GetId() << " found at " << sIpAddress << endl;
                        }
                        else
                        {
                            Log::Get() << "Found Receiver but json data incorrect: " << pResource->GetJsonParseError() << endl;
                        }
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


void ClientApiPrivate::RemoveResources(const string& sIpAddress)
{

    size_t nRemoved = m_nodes.RemoveResources(sIpAddress);
    Log::Get() << nRemoved << " node(s) removed" << endl;

    nRemoved = m_devices.RemoveResources(sIpAddress);
    Log::Get() << nRemoved << " device(s) removed" << endl;
    nRemoved = m_sources.RemoveResources(sIpAddress);
    Log::Get() << nRemoved << " source(s) removed" << endl;
    nRemoved = m_flows.RemoveResources(sIpAddress);
    Log::Get() << nRemoved << " flow(s) removed" << endl;
    nRemoved = m_senders.RemoveResources(sIpAddress);
    Log::Get() << nRemoved << " sender(s) removed" << endl;
    nRemoved = m_receivers.RemoveResources(sIpAddress);
    Log::Get() << nRemoved << " receiver(s) removed" << endl;
}

void ClientApiPrivate::NodeDetailsDone()
{
    m_nCurlThreadCount--;
}

ServiceBrowser* ClientApiPrivate::GetBrowser()
{
    return m_pBrowser;
}

void ClientApiPrivate::DeleteServiceBrowser()
{
    delete m_pBrowser;
    m_pBrowser = 0;
    delete m_pCurl;
    m_pCurl = 0;
}


void ClientApiPrivate::ChangeInterest(int nFlags)
{
    lock_guard<mutex> lg(m_mutex);
    m_nFlags = nFlags;
}

int ClientApiPrivate::GetInterestFlags()
{
    lock_guard<mutex> lg(m_mutex);
    return m_nFlags;
}


map<string, shared_ptr<Self> >::const_iterator ClientApiPrivate::GetNodeBegin()
{
    return m_nodes.GetResourceBegin();
}

map<string, shared_ptr<Self> >::const_iterator ClientApiPrivate::GetNodeEnd()
{
    return m_nodes.GetResourceEnd();
}

map<string, shared_ptr<Self> >::const_iterator ClientApiPrivate::FindNode(const string& sUid)
{
    return m_nodes.FindNmosResource(sUid);
}

map<string, shared_ptr<Device> >::const_iterator ClientApiPrivate::GetDeviceBegin()
{
    return m_devices.GetResourceBegin();
}

map<string, shared_ptr<Device> >::const_iterator ClientApiPrivate::GetDeviceEnd()
{
    return m_devices.GetResourceEnd();
}

map<string, shared_ptr<Device> >::const_iterator ClientApiPrivate::FindDevice(const string& sUid)
{
    return m_devices.FindNmosResource(sUid);
}


map<string, shared_ptr<Source> >::const_iterator ClientApiPrivate::GetSourceBegin()
{
    return m_sources.GetResourceBegin();
}

map<string, shared_ptr<Source> >::const_iterator ClientApiPrivate::GetSourceEnd()
{
    return m_sources.GetResourceEnd();
}

map<string, shared_ptr<Source> >::const_iterator ClientApiPrivate::FindSource(const string& sUid)
{
    return m_sources.FindNmosResource(sUid);
}


map<string, shared_ptr<Flow> >::const_iterator ClientApiPrivate::GetFlowBegin()
{
    return m_flows.GetResourceBegin();
}

map<string, shared_ptr<Flow> >::const_iterator ClientApiPrivate::GetFlowEnd()
{
    return m_flows.GetResourceEnd();
}

map<string, shared_ptr<Flow> >::const_iterator ClientApiPrivate::FindFlow(const string& sUid)
{
    return m_flows.FindNmosResource(sUid);
}



map<string, shared_ptr<Sender> >::const_iterator ClientApiPrivate::GetSenderBegin()
{
    return m_senders.GetResourceBegin();
}

map<string, shared_ptr<Sender> >::const_iterator ClientApiPrivate::GetSenderEnd()
{
    return m_senders.GetResourceEnd();
}

map<string, shared_ptr<Sender> >::const_iterator ClientApiPrivate::FindSender(const string& sUid)
{
    return m_senders.FindNmosResource(sUid);
}



map<string, shared_ptr<Receiver> >::const_iterator ClientApiPrivate::GetReceiverBegin()
{
    return m_receivers.GetResourceBegin();
}

map<string, shared_ptr<Receiver> >::const_iterator ClientApiPrivate::GetReceiverEnd()
{
    return m_receivers.GetResourceEnd();
}

map<string, shared_ptr<Receiver> >::const_iterator ClientApiPrivate::FindReceiver(const string& sUid)
{
    return m_receivers.FindNmosResource(sUid);
}


bool ClientApiPrivate::Subscribe(const std::string& sSenderId, const std::string& sReceiverId)
{
    lock_guard<mutex> lg(m_mutex);
    map<string, shared_ptr<Sender> >::const_iterator itSender=  m_senders.FindNmosResource(sSenderId);
    map<string, shared_ptr<Receiver>  >::const_iterator itReceiver =  m_receivers.FindNmosResource(sReceiverId);
    if(itSender == m_senders.GetResourceEnd() || itReceiver == m_receivers.GetResourceEnd())
    {
        Log::Get(Log::LOG_ERROR) << "Sender: " << sSenderId << " or Receiver: " << sReceiverId << " not found" << endl;
        return false;
    }
    ApiVersion version(0,0);
    string sUrl(GetTargetUrl(itReceiver->second, version));
    if(sUrl.empty() == false)
    {
        //do a PUT to the correct place on the URL
        Json::StyledWriter sw;
        string sJson(sw.write(itSender->second->GetJson(version)));
        m_pCurl->Put(sUrl, sJson, ClientPoster::CURLTYPE_TARGET);

        return true;
    }
    return false;
}

bool ClientApiPrivate::Unsubscribe(const std::string& sReceiverId)
{
    lock_guard<mutex> lg(m_mutex);
    map<string, shared_ptr<Receiver> >::const_iterator itReceiver =  m_receivers.FindNmosResource(sReceiverId);
    if(itReceiver == m_receivers.GetResourceEnd())
    {
        Log::Get(Log::LOG_ERROR) << "Receiver: " << sReceiverId << " not found" << endl;
        return false;
    }

    ApiVersion version(0,0);
    string sUrl(GetTargetUrl(itReceiver->second, version));
    if(sUrl.empty() == false)
    {
        //do a PUT to the correct place on the URL
        m_pCurl->Put(sUrl, "{}", ClientPoster::CURLTYPE_TARGET);

        return true;
    }
    return false;
}

std::string ClientApiPrivate::GetTargetUrl(shared_ptr<Receiver> pReceiver, ApiVersion& version)
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
