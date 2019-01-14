#include "registryapi.h"
#include "avahipublisher.h"
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

using namespace std;

const string RegistryApi::STR_RESOURCE[6] = {"node", "device", "source", "flow", "sender", "receiver"};

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
    Log::Get() << "RegistryApi: Start" << endl;
    return (StartPublisher(nPriority, sInterface, nPort, bSecure) && StartServer(nPort));
}

void RegistryApi::Stop()
{
    StopPublisher();
    StopServer();
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


const shared_ptr<Resource> RegistryApi::FindResource(const std::string& sType, const std::string& sId)
{
    if(m_pRegistry)
    {
        return m_pRegistry->FindResource(sType, sId);
    }
    return shared_ptr<Resource>(0);
}

bool RegistryApi::AddResource(const std::string& sType, shared_ptr<Resource> pResource)
{
    if(m_pRegistry)
    {
        return m_pRegistry->AddResource(sType, pResource);
    }
    return false;
}

unsigned short RegistryApi::AddUpdateResource(const string& sType, const Json::Value& jsData)
{
    if(m_pRegistry)
    {
        shared_ptr<Resource> pResource = m_pRegistry->FindResource(sType, jsData["id"].asString());
        if(pResource)
        {
            if(UpdateResource(jsData, pResource))
            {
                return 200;
            }
            return 404;
        }
        else if(AddResource(sType, jsData))
        {
            return 201;
        }
    }
    return 404;
}

bool RegistryApi::AddResource(const string& sType, const Json::Value& jsData)
{
    bool bOk(false);
    if(sType == STR_RESOURCE[NODE])
    {
        shared_ptr<Self> pResource = make_shared<Self>();
        if(pResource->UpdateFromJson(jsData))
        {
            bOk = AddResource(sType, pResource);
        }
    }
    else if(sType == STR_RESOURCE[DEVICE])
    {
        shared_ptr<Device> pResource = make_shared<Device>();
        if(pResource->UpdateFromJson(jsData))
        {
            bOk = AddResource(sType, pResource);
        }
    }
    else if(sType == STR_RESOURCE[SENDER])
    {
        shared_ptr<Sender> pResource = make_shared<Sender>();
        if(pResource->UpdateFromJson(jsData))
        {
            bOk = AddResource(sType, pResource);
        }
    }
    else if(sType == STR_RESOURCE[RECEIVER])
    {
        shared_ptr<Receiver> pResource = make_shared<Receiver>();
        if(pResource->UpdateFromJson(jsData))
        {
            bOk = AddResource(sType, pResource);
        }
    }
    else if(sType == STR_RESOURCE[SOURCE])
    {
        //audio or generic?
        if(jsData["format"].isString())
        {
            if(jsData["format"].asString() == "urn:x-nmos:format:audio")
            {   //Audio
                shared_ptr<SourceAudio> pResource = make_shared<SourceAudio>();
                if(pResource->UpdateFromJson(jsData))
                {
                    bOk = AddResource(sType, pResource);
                }
            }
            else
            {   //Generic
                shared_ptr<SourceGeneric> pResource = make_shared<SourceGeneric>();
                if(pResource->UpdateFromJson(jsData))
                {
                    bOk = AddResource(sType, pResource);
                }
            }
        }
    }
    else if(sType == STR_RESOURCE[FLOW])
    {
        if(jsData["format"].isString())
        {
            if(jsData["format"].asString() == "urn:x-nmos:format:audio")
            {
                if(jsData["media_type"].isString())
                {
                    if(jsData["media_type"].asString() == "audio/L24" || jsData["media_type"].asString() == "audio/L20" || jsData["media_type"].asString() == "audio/L16" || jsData["media_type"].asString() == "audio/L8")
                    {   //Raw Audio
                        shared_ptr<FlowAudioRaw> pResource = make_shared<FlowAudioRaw>();
                        if(pResource->UpdateFromJson(jsData))
                        {
                            bOk = AddResource(sType, pResource);
                        }
                    }
                    else
                    {   //Code audio
                        shared_ptr<FlowAudioCoded> pResource = make_shared<FlowAudioCoded>();
                        if(pResource->UpdateFromJson(jsData))
                        {
                            bOk = AddResource(sType, pResource);
                        }
                    }
                }
            }
            else if(jsData["format"].asString() == "urn:x-nmos:format:video")
            {
                if(jsData["media_type"].isString())
                {//Coded vidoe
                    if(jsData["media_type"].asString() == "vidoe/raw")
                    {
                        shared_ptr<FlowVideoRaw> pResource = make_shared<FlowVideoRaw>();
                        if(pResource->UpdateFromJson(jsData))
                        {
                            bOk = AddResource(sType, pResource);
                        }
                    }
                    else
                    {
                        shared_ptr<FlowVideoCoded> pResource = make_shared<FlowVideoCoded>(jsData["media_type"].asString());
                        if(pResource->UpdateFromJson(jsData))
                        {
                            bOk = AddResource(sType, pResource);
                        }
                    }
                }
            }
            else if(jsData["format"].asString() == "urn:x-nmos:format:data")
            {
                if(jsData["media_type"] == "video/smpte291")
                {
                    shared_ptr<FlowDataSdiAnc> pResource = make_shared<FlowDataSdiAnc>();
                    if(pResource->UpdateFromJson(jsData))
                    {
                        bOk = AddResource(sType, pResource);
                    }
                }
                //SDIAncData only at the moment
            }
            else if(jsData["format"].asString() == "urn:x-nmos:format:mux")
            {
                //Mux only at the momemnt
                shared_ptr<FlowMux> pResource = make_shared<FlowMux>();
                if(pResource->UpdateFromJson(jsData))
                {
                    bOk = AddResource(sType, pResource);
                }
            }
        }
    }
    return bOk;
}

bool RegistryApi::UpdateResource(const Json::Value& jsData, std::shared_ptr<Resource> pResource)
{
    // @todo update resource  we should make a copy of the resource in case updating it corrupts it
    if(m_pRegistry)
    {
        if(pResource->UpdateFromJson(jsData))
        {
            return m_pRegistry->ResourceUpdated(pResource);
        }
    }
    return false;
}

bool RegistryApi::DeleteResource(const string& sType, const std::string& sId)
{
    if(m_pRegistry)
    {
        return m_pRegistry->DeleteResource(sType, sId);
    }
    return false;
}


size_t RegistryApi::Heartbeat(const std::string& sId)
{
    if(m_pRegistry)
    {
        return m_pRegistry->Heartbeat(sId);
    }
    return 0;
}
