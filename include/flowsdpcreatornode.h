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
        class TransportParamsRTPSender;
    };
};

std::string CreateFlowSdpLines(pml::nmos::NodeApiPrivate& api, std::shared_ptr<pml::nmos::Flow> pFlow, unsigned short nRtpPort, const std::string& sConnectionLine);
std::string CreateFlowSdpLines(pml::nmos::NodeApiPrivate& api, std::shared_ptr<pml::nmos::FlowAudioCoded> pFlow, unsigned short nRtpPort, const std::string& sConnectionLine);
std::string CreateFlowSdpLines(pml::nmos::NodeApiPrivate& api, std::shared_ptr<pml::nmos::FlowAudioRaw> pFlow, unsigned short nRtpPort, const std::string& sConnectionLine);
std::string CreateConnectionLine(const pml::nmos::TransportParamsRTPSender& tpSender);
/* @todo

extern std::string CreateFlowSdpLines(NodeApiPrivate& api, std::shared_ptr<FlowVideoCoded> pFlow, unsigned short nRtpPort);
extern std::string CreateFlowSdpLines(NodeApiPrivate& api, std::shared_ptr<FlowVideoRaw> pFlow, unsigned short nRtpPort);
extern std::string CreateFlowSdpLines(NodeApiPrivate& api, std::shared_ptr<FlowDataSdiAnc> pFlow, unsigned short nRtpPort);
extern std::string CreateFlowSdpLines(NodeApiPrivate& api, std::shared_ptr<FlowMux> pFlow, unsigned short nRtpPort);
        */
