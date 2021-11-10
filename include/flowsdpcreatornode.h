#pragma once
#include "flowsdpcreator.h"
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

        class FlowAudioCodedSdpCreator : public FlowSdpCreator
        {
            public:
                FlowAudioCodedSdpCreator(std::shared_ptr<FlowAudioCoded> pFlow) : FlowSdpCreator(), m_pFlow(pFlow){}
                std::string CreateLines(unsigned short nRtpPort) override;

            private:
                std::shared_ptr<FlowAudioCoded> m_pFlow;

        };

        class FlowAudioRawSdpCreator : public FlowSdpCreator
        {
            public:
                FlowAudioRawSdpCreator(std::shared_ptr<FlowAudioRaw> pFlow) : FlowSdpCreator(), m_pFlow(pFlow){}

                std::string CreateLines(unsigned short nRtpPort) override;
            private:
                std::shared_ptr<FlowAudioRaw> m_pFlow;

        };

        class FlowVideoCodedSdpCreator : public FlowSdpCreator
        {
            public:
                FlowVideoCodedSdpCreator() : FlowSdpCreator(){}

        };

        class FlowVideoRawSdpCreator : public FlowSdpCreator
        {
            public:
                FlowVideoRawSdpCreator() : FlowSdpCreator(){}

        };

        class FlowDataSdiAncSdpCreator : public FlowSdpCreator
        {
            public:
                FlowDataSdiAncSdpCreator() : FlowSdpCreator(){}

        };


        class FlowMuxSdpCreator : public FlowSdpCreator
        {
            public:
                FlowMuxSdpCreator() : FlowSdpCreator(){}

        };
    };
};
