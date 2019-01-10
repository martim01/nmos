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

using namespace std;

RegistryApi::RegistryApi() :
    m_pRegistryApiPublisher(0),
    m_pRegistryServer(0),
    m_resources("")
{

}

RegistryApi::~RegistryApi()
{
    Stop();
}

bool RegistryApi::Start(unsigned short nPriority, unsigned short nPort, bool bSecure)
{
    return (StartPublisher(nPriority, nPort, bSecure) && StartServer(nPort));
}

void RegistryApi::Stop()
{
    StopPublisher();
    StopServer();
}

bool RegistryApi::StartPublisher(unsigned short nPriority, unsigned short nPort, bool bSecure)
{
    StopPublisher();
    m_pRegistryApiPublisher = new ServicePublisher("registryapi", "_nmos-registry._tcp", nPort);

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
const ResourceHolder& RegistryApi::GetResources()
{
    return m_resources;
}

shared_ptr<Resource> RegistryApi::GetResource(const std::string& sId)
{
    map<string, shared_ptr<Resource> >::iterator itResource = m_resources.GetResource(sId);
    if(itResource != m_resources.GetResourceEnd())
    {
        return itResource->second;
    }
    return shared_ptr<Resource>(0);
}


bool RegistryApi::AddResource(const string& sType, const Json::Value& jsData)
{
    bool bOk(false);
    if(sType == "node")
    {

    }
    else if(sType == "device")
    {
        shared_ptr<Device> pResource = make_shared<Device>();
        if(pResource->UpdateFromJson(jsData))
        {
            m_resources.AddResource(pResource);
            bOk = true;
        }
    }
    else if(sType == "sender")
    {
        shared_ptr<Sender> pResource = make_shared<Sender>();
        if(pResource->UpdateFromJson(jsData))
        {
            m_resources.AddResource(pResource);
            bOk = true;
        }
    }
    else if(sType == "receiver")
    {
        hared_ptr<Receiver> pResource = make_shared<Receiver>();
        if(pResource->UpdateFromJson(jsData))
        {
            m_resources.AddResource(pResource);
            bOk = true;
        }
    }
    else if(sType == "source")
    {
        //audio or generic?
        if(jsData["format"].isString())
        {
            if(jsData["format"].asString() == "urn:x-nmos:format:audio")
            {   //Audio
                shared_ptr<SourceAudio> pResource = make_shared<SourceAudio>();
                if(pResource->UpdateFromJson(jsData))
                {
                    m_resources.AddResource(pResource);
                    bOk = true;
                }
            }
            else
            {   //Generic
                shared_ptr<SourceGeneric> pResource = make_shared<SourceGeneric>();
                if(pResource->UpdateFromJson(jsData))
                {
                    m_resources.AddResource(pResource);
                    bOk = true;
                }
            }
        }
    }
    else if(sType == "flow")
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
                            m_resources.AddResource(pResource);
                            bOk = true;
                        }
                    }
                    else
                    {   //Code audio
                        shared_ptr<FlowAudioCoded> pResource = make_shared<FlowAudioCoded>();
                        if(pResource->UpdateFromJson(jsData))
                        {
                            m_resources.AddResource(pResource);
                            bOk = true;
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
                            m_resources.AddResource(pResource);
                            bOk = true;
                        }
                    }
                    else
                    {
                        shared_ptr<FlowVideoCoded> pResource = make_shared<FlowVideoCoded>(sData["media_type"].asString());
                        if(pResource->UpdateFromJson(jsData))
                        {
                            m_resources.AddResource(pResource);
                            bOk = true;
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
                        m_resources.AddResource(pResource);
                        bOk = true;
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
                    m_resources.AddResource(pResource);
                    bOk = true;
                }
            }
        }
    }
    return bOk;
}

bool RegistryApi::UpdateResource(const Json::Value& jsData, std::shared_ptr<Resource> pResource)
{

}
void RegistryApi::DeleteResource(const string& sId)
{
    m_resources.RemoveResource(sId);
    m_resources.Commit();
}
