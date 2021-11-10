#pragma once
#include "flowsdpcreator.h"
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
                FlowAudioCodedSdpCreator(FlowAudioCoded* pFlow) : FlowSdpCreator(), m_pFlow(pFlow){}
                std::string CreateLines(unsigned short nRtpPort) override;

            private:
                FlowAudioCoded* m_pFlow;

        };

        class FlowAudioRawSdpCreator : public FlowSdpCreator
        {
            public:
                FlowAudioRawSdpCreator(FlowAudioRaw* pFlow) : FlowSdpCreator(), m_pFlow(pFlow){}

                std::string CreateLines(unsigned short nRtpPort) override;
            private:
                FlowAudioRaw* m_pFlow;

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
