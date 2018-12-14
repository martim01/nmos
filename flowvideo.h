#pragma once
#include "flow.h"


class FlowVideo : public Flow
{
    public:
        enum enumInterlace{PROGRESSIVE, INTERLACED_TFF, INTERLACED_BFF, INTERLACED_PSF};
        enum enumColour{BT601, BT709, BT2020, BT2100};
        enum enumTransfer{SDR, HLG,PQ};

        FlowVideo(std::string sLabel, std::string sDescription, std::string sSourceId, std::string sDeviceId, unsigned int nFrameWidth, unsigned int nFrameHeight, enumColour eColour, enumInterlace eInterlace=PROGRESSIVE, enumTransfer eTransfer=SDR);

        virtual bool Commit();

        void SetFrameWidth(unsigned int nWidth);
        void SetFrameHeight(unsigned int nHeight);
        void SetColour(enumColour eColour);
        void SetInterlace(enumInterlace eInterlace);
        void SetTransfer(enumTransfer eTransfer);

    private:
        unsigned int m_nFrameWidth;
        unsigned int m_nFrameHeight;

        enumColour m_eColour;
        enumInterlace m_eInterlace;

        enumTransfer m_eTransfer;

        static const std::string STR_INTERLACE[4];
        static const std::string STR_COLOUR[4];
        static const std::string STR_TRANSFER[3];

};


