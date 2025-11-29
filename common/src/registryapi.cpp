#ifdef __NMOS_REGISTRY__
#include "registryapi.h"
#ifdef __GNU__
#include "avahipublisher.h"
#else
#include "bonjourpublisher.h"
#endif // __GNU__
#include "registryserver.h"
#include "self.h"
#include "device.h"
#include "sourceaudio.h"
#include "sourcegeneric.h"
#include "flowaudiocoded.h"
#include "flowaudioraw.h"
#include "flowvideoraw.h"
#include "flowvideocoded.h"
#include "flowdatasdianc.h"
#include "flowmux.h"
#include "sender.h"
#include "receiver.h"
#include "log.h"
#include "utils.h"
#include "registry.h"
#include <chrono>

using namespace std;

const string RegistryApi::STR_RESOURCE[6] = {"node", "device", "source", "flow", "sender", "receiver"};

void GarbageThread()
{
    while(RegistryApi::Get().Running())
    {
        std::this_thread::sleep_for(std::chrono::seconds(12));
        RegistryApi::Get().GarbageCollection();
    }
}


RegistryApi::RegistryApi() :
    m_pRegistryApiPublisher(0),
    m_pRegistryServer(0),
    m_pRegistry(0)
{

}

RegistryApi::~RegistryApi()
{
    Stop();
}

RegistryApi& RegistryApi::Get()
{
    static RegistryApi ra;
    return ra;
}

bool RegistryApi::Start(shared_ptr<Registry> pRegistry, unsigned short nPriority, const std::string& sInterface, unsigned short nPort, bool bSecure)
{
    m_pRegistry = pRegistry;
    pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "RegistryApi: Start" ;
    if(StartPublisher(nPriority, sInterface, nPort, bSecure) && StartServer(nPort))
    {
        m_bRunning = true;
        //Start the garbage collector thread
        thread th(GarbageThread);
        th.detach();
        return true;
    }
    return false;
}

void RegistryApi::Stop()
{
    lock_guard<mutex> lg(m_mutex);
    StopPublisher();
    StopServer();
    m_bRunning = false;
}

bool RegistryApi::StartPublisher(unsigned short nPriority, const std::string& sInterface, unsigned short nPort, bool bSecure)
{
    StopPublisher();
    m_pRegistryApiPublisher = new ServicePublisher("registryapi", "_nmos-registration._tcp", nPort, GetIpAddress(sInterface));

    if(bSecure)
    {
        m_pRegistryApiPublisher->AddTxt("api_proto", "https", false);
    }
    else
    {
        m_pRegistryApiPublisher->AddTxt("api_proto", "http", false);
    }
    m_pRegistryApiPublisher->AddTxt("api_ver", "v1.2", false);

    m_pRegistryApiPublisher->AddTxt("pri", to_string(nPriority),false);
    m_pRegistryApiPublisher->Modify();

    return m_pRegistryApiPublisher->Start();
}

void RegistryApi::StopPublisher()
{
    if(m_pRegistryApiPublisher)
    {
        m_pRegistryApiPublisher->Stop();
        delete m_pRegistryApiPublisher;
        m_pRegistryApiPublisher = 0;
    }
}

bool RegistryApi::StartServer(unsigned short nPort)
{
    StopServer();
    m_pRegistryServer = new RegistryServer();
    return m_pRegistryServer->Init(nPort);
}

void RegistryApi::StopServer()
{

    if(m_pRegistryServer)
    {
        m_pRegistryServer->Stop();
        delete m_pRegistryServer;
        m_pRegistryServer = 0;
    }
}


const shared_ptr<Resource> RegistryApi::FindNmosResource(const std::string& sType, const std::string& sId)
{
    lock_guard<mutex> lg(m_mutex);
    if(m_pRegistry)
    {
        return m_pRegistry->FindNmosResource(sType, sId);
    }
    return shared_ptr<Resource>(0);
}

bool RegistryApi::AddResource(const std::string& sType, shared_ptr<Resource> pResource)
{
    lock_guard<mutex> lg(m_mutex);
    if(m_pRegistry)
    {
        return m_pRegistry->AddResource(sType, pResource);
    }
    return false;
}

unsigned short RegistryApi::AddUpdateResource(const string& sType, const Json::Value& jsData, std::string& sError)
{
    //lock_guard<mutex> lg(m_mutex);
    if(m_pRegistry)
    {
        pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "FindNmosResource: " << jsData["id"].asString() ;
        shared_ptr<Resource> pResource = m_pRegistry->FindNmosResource(sType, jsData["id"].asString());
        if(pResource)
        {
            pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "Resource already registered!" ;
            if(UpdateNmosResource(jsData, pResource,sError))
            {
                return 200;
            }
            return 404;
        }
        else
        {
            pml::log::log(pml::log::Level::kInfo, "pml::nmos") << "Add Resource" ;
            return AddResource(sType, jsData,sError);
        }
    }
    return 404;
}

unsigned short RegistryApi::AddResource(const string& sType, const Json::Value& jsData, std::string& sError)
{
    bool bOk(false);
    if(sType == STR_RESOURCE[NODE])
    {
        shared_ptr<Self> pResource = make_shared<Self>();
        if(pResource->UpdateFromJson(jsData))
        {
            bOk = AddResource(sType, pResource);
            if(bOk)
            {   //mark the registration as a heartbeat so we don't garbage collect
                m_pRegistry->Heartbeat(pResource->GetId());
            }
        }
        else
        {
            sError = pResource->GetJsonParseError();
        }
    }
    else if(sType == STR_RESOURCE[DEVICE])
    {
        shared_ptr<Device> pResource = make_shared<Device>();
        if(pResource->UpdateFromJson(jsData))
        {
            if(!FindNmosResource(STR_RESOURCE[NODE], pResource->GetParentResourceId()))
            {
               return 400;
            }
            bOk = AddResource(sType, pResource);
        }
        else
        {
            sError = pResource->GetJsonParseError();
        }
    }
    else if(sType == STR_RESOURCE[SENDER])
    {
        shared_ptr<Sender> pResource = make_shared<Sender>();
        if(pResource->UpdateFromJson(jsData))
        {
            if(!FindNmosResource(STR_RESOURCE[DEVICE], pResource->GetParentResourceId()))
            {
                return 400;
            }
            bOk = AddResource(sType, pResource);
        }
        else
        {
            sError = pResource->GetJsonParseError();
        }
    }
    else if(sType == STR_RESOURCE[RECEIVER])
    {
        shared_ptr<Receiver> pResource = make_shared<Receiver>();
        if(pResource->UpdateFromJson(jsData))
        {
            if(!FindNmosResource(STR_RESOURCE[DEVICE], pResource->GetParentResourceId()))
            {
                return 400;
            }
            bOk = AddResource(sType, pResource);
        }
        else
        {
            sError = pResource->GetJsonParseError();
        }
    }
    else if(sType == STR_RESOURCE[SOURCE])
    {
        //audio or generic?
        if(jsData["format"].isString())
        {
            if(jsData["format"].asString().find("urn:x-nmos:format:audio") != string::npos)
            {   //Audio
                shared_ptr<SourceAudio> pResource = make_shared<SourceAudio>();
                if(pResource->UpdateFromJson(jsData))
                {
                    if(!FindNmosResource(STR_RESOURCE[DEVICE], pResource->GetParentResourceId()))
                    {
                        return 400;
                    }
                    bOk = AddResource(sType, pResource);
                }
                else
                {
                    sError = pResource->GetJsonParseError();
                }
            }
            else
            {   //Generic
                shared_ptr<SourceGeneric> pResource = make_shared<SourceGeneric>();
                if(pResource->UpdateFromJson(jsData))
                {
                    if(!FindNmosResource(STR_RESOURCE[DEVICE], pResource->GetParentResourceId()))
                    {
                        return 400;
                    }
                    bOk = AddResource(sType, pResource);
                }
                else
                {
                    sError = pResource->GetJsonParseError();
                }
            }
        }
    }
    else if(sType == STR_RESOURCE[FLOW])
    {
        if(jsData["format"].isString())
        {
            if(jsData["format"].asString().find("urn:x-nmos:format:audio") != string::npos)
            {
                if(jsData["media_type"].isString())
                {
                    if(jsData["media_type"].asString() == "audio/L24" || jsData["media_type"].asString() == "audio/L20" || jsData["media_type"].asString() == "audio/L16" || jsData["media_type"].asString() == "audio/L8")
                    {   //Raw Audio
                        shared_ptr<FlowAudioRaw> pResource = make_shared<FlowAudioRaw>();
                        if(pResource->UpdateFromJson(jsData))
                        {
                            if(!FindNmosResource(STR_RESOURCE[DEVICE], pResource->GetParentResourceId()))
                            {
                                return 400;
                            }
                            bOk = AddResource(sType, pResource);
                        }
                        else
                        {
                            sError = pResource->GetJsonParseError();
                        }
                    }
                    else
                    {   //Code audio
                        shared_ptr<FlowAudioCoded> pResource = make_shared<FlowAudioCoded>();
                        if(pResource->UpdateFromJson(jsData))
                        {
                            if(!FindNmosResource(STR_RESOURCE[DEVICE], pResource->GetParentResourceId()))
                            {
                                return 400;
                            }
                            bOk = AddResource(sType, pResource);
                        }
                        else
                        {
                            sError = pResource->GetJsonParseError();
                        }
                    }
                }
            }
            else if(jsData["format"].asString().find("urn:x-nmos:format:video") != string::npos)
            {
                if(jsData["media_type"].isString())
                {//Coded vidoe
                    if(jsData["media_type"].asString() == "vidoe/raw")
                    {
                        shared_ptr<FlowVideoRaw> pResource = make_shared<FlowVideoRaw>();
                        if(pResource->UpdateFromJson(jsData))
                        {
                            if(!FindNmosResource(STR_RESOURCE[DEVICE], pResource->GetParentResourceId()))
                            {
                                return 400;
                            }
                            bOk = AddResource(sType, pResource);
                        }
                        else
                        {
                            sError = pResource->GetJsonParseError();
                        }
                    }
                    else
                    {
                        shared_ptr<FlowVideoCoded> pResource = make_shared<FlowVideoCoded>(jsData["media_type"].asString());
                        if(pResource->UpdateFromJson(jsData))
                        {
                            if(!FindNmosResource(STR_RESOURCE[DEVICE], pResource->GetParentResourceId()))
                            {
                                return 400;
                            }
                            bOk = AddResource(sType, pResource);
                        }
                        else
                        {
                            sError = pResource->GetJsonParseError();
                        }
                    }
                }
            }
            else if(jsData["format"].asString().find("urn:x-nmos:format:data") != string::npos)
            {
                if(jsData["media_type"] == "video/smpte291")
                {
                    shared_ptr<FlowDataSdiAnc> pResource = make_shared<FlowDataSdiAnc>();
                    if(pResource->UpdateFromJson(jsData))
                    {
                        if(!FindNmosResource(STR_RESOURCE[DEVICE], pResource->GetParentResourceId()))
                        {
                            return 400;
                        }
                        bOk = AddResource(sType, pResource);
                    }
                    else
                    {
                        sError = pResource->GetJsonParseError();
                    }
                }
                //SDIAncData only at the moment
            }
            else if(jsData["format"].asString().find("urn:x-nmos:format:mux") != string::npos)
            {
                //Mux only at the momemnt
                shared_ptr<FlowMux> pResource = make_shared<FlowMux>();
                if(pResource->UpdateFromJson(jsData))
                {
                    if(!FindNmosResource(STR_RESOURCE[DEVICE], pResource->GetParentResourceId()))
                    {
                        return 400;
                    }
                    bOk = AddResource(sType, pResource);
                }
                else
                {
                    sError = pResource->GetJsonParseError();
                }
            }
        }
    }
    else
    {
        sError = "Resource type not valid";
    }
    if(bOk)
    {
        return 201;
    }
    return 404;
}

bool RegistryApi::UpdateNmosResource(const Json::Value& jsData, std::shared_ptr<Resource> pResource, std::string& sError)
{
    // @todo update resource  we should make a copy of the resource in case updating it corrupts it
    if(m_pRegistry)
    {
        if(pResource->UpdateFromJson(jsData))
        {
            return m_pRegistry->ResourceUpdated(pResource);
        }
        else
        {
            sError = pResource->GetJsonParseError();
        }
    }
    return false;
}

bool RegistryApi::DeleteResource(const string& sType, const std::string& sId)
{
    lock_guard<mutex> lg(m_mutex);
    if(m_pRegistry)
    {
        return m_pRegistry->DeleteResource(sType, sId);
    }
    return false;
}


size_t RegistryApi::Heartbeat(const std::string& sId)
{
    lock_guard<mutex> lg(m_mutex);
    if(m_pRegistry)
    {
        return m_pRegistry->Heartbeat(sId);
    }
    return 0;
}

bool RegistryApi::Running()
{
    lock_guard<mutex> lg(m_mutex);
    return m_bRunning;
}

void RegistryApi::GarbageCollection()
{
    lock_guard<mutex> lg(m_mutex);
    if(m_pRegistry)
    {
        m_pRegistry->GarbageCollection();
    }
}
#endif
