#pragma once
#include "flow.h"


class FlowAudio : public Flow
{
    public:
        FlowAudio(std::string sLabel, std::string sDescription, std::string sSourceId, std::string sDeviceId, unsigned int nSampleRate);
        virtual bool Commit();

        void SetSampleRate(unsigned int nSampleRate);

    private:
        unsigned int m_nSampleRate;
};


