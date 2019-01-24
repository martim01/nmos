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
        m_pBrowser = new ServiceBrowser(make_shared<ClientPoster>(), false);
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



void ClientApiPrivate::SetInstanceResolved(std::shared_ptr<dnsInstance> pInstance)
{
    m_mutex.lock();
    m_pInstance = pInstance;
    m_eSignal = CLIENT_SIG_INSTANCE_RESOLVED;
    m_mutex.unlock();

    m_cvBrowse.notify_one();
}

void ClientApiPrivate::SetInstanceRemoved(std::shared_ptr<dnsInstance> pInstance)
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

bool ClientApiPrivate::UpdateResource(ClientHolder& holder, const Json::Value& jsData)
{
    map<string, shared_ptr<Resource> >::iterator itResource = holder.GetNmosResource(jsData["id"].asString());
    if(itResource != holder.GetResourceEnd())
    {
        Log::Get() <<  itResource->first << " found already " << endl;
        if(itResource->second->UpdateFromJson(jsData))
        {
            Log::Get() <<  itResource->first << " updated " << endl;
        }
        else
        {
            Log::Get() << "Found node but json data incorrect: " << itResource->second->GetJsonParseError() << endl;
        }
        return true;
    }
    return false;
}

void ClientApiPrivate::AddNode(const std::string& sIpAddress, const std::string& sData)
{
    lock_guard<mutex> lg(m_mutex);

    Json::Value jsData;
    Json::Reader jsReader;
    if(jsReader.parse(sData, jsData))
    {
        if(jsData["id"].isString())
        {
            if(!UpdateResource(m_nodes, jsData))
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

void ClientApiPrivate::AddDevices(const std::string& sIpAddress, const std::string& sData)
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
                    if(!UpdateResource(m_devices, jsData[ai]))
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

void ClientApiPrivate::AddSources(const std::string& sIpAddress, const std::string& sData)
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
                    if(!UpdateResource(m_sources, jsData[ai]))
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

void ClientApiPrivate::AddFlows(const std::string& sIpAddress, const std::string& sData)
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
                    if(!UpdateResource(m_flows, jsData[ai]))
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

void ClientApiPrivate::AddSenders(const std::string& sIpAddress, const std::string& sData)
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
                    if(!UpdateResource(m_senders, jsData[ai]))
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

void ClientApiPrivate::AddReceivers(const std::string& sIpAddress, const std::string& sData)
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
                    if(!UpdateResource(m_receivers, jsData[ai]))
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


void ClientApiPrivate::RemoveResources(const std::string& sIpAddress)
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
