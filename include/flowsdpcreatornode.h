#pragma once
#include <memory>



namespace pml
{
    namespace nmos
    {
        class FlowAudioCoded;
        class FlowAudioRaw;
        class FlowVideoCoded;
        class FlowVideoRaw;
        class FlowDataSdiAnc;
        class FlowMux;
        class Flow;
        class NodeApiPrivate;
    };
};

std::string CreateFlowSdpLines(pml::nmos::NodeApiPrivate& api, std::shared_ptr<pml::nmos::Flow> pFlow, unsigned short nRtpPort);
std::string CreateFlowSdpLines(pml::nmos::NodeApiPrivate& api, std::shared_ptr<pml::nmos::FlowAudioCoded> pFlow, unsigned short nRtpPort);
std::string CreateFlowSdpLines(pml::nmos::NodeApiPrivate& api, std::shared_ptr<pml::nmos::FlowAudioRaw> pFlow, unsigned short nRtpPort);
/* @todo

extern std::string CreateFlowSdpLines(NodeApiPrivate& api, std::shared_ptr<FlowVideoCoded> pFlow, unsigned short nRtpPort);
extern std::string CreateFlowSdpLines(NodeApiPrivate& api, std::shared_ptr<FlowVideoRaw> pFlow, unsigned short nRtpPort);
extern std::string CreateFlowSdpLines(NodeApiPrivate& api, std::shared_ptr<FlowDataSdiAnc> pFlow, unsigned short nRtpPort);
extern std::string CreateFlowSdpLines(NodeApiPrivate& api, std::shared_ptr<FlowMux> pFlow, unsigned short nRtpPort);
        */
