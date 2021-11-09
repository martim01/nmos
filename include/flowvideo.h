#pragma once
#include "flow.h"
#include "nmosdlldefine.h"

class NMOS_EXPOSE FlowVideo : public Flow
{
    public:
        enum enumInterlace{PROGRESSIVE, INTERLACED_TFF, INTERLACED_BFF, INTERLACED_PSF};
        enum enumColour{BT601, BT709, BT2020, BT2100};
        enum enumTransfer{SDR, HLG,PQ};

        FlowVideo(const std::string& sLabel, const std::string& sDescription, const std::string& sSourceId, const std::string& sDeviceId, const std::string& sMediaType, unsigned int nFrameWidth, unsigned int nFrameHeight, enumColour eColour, enumInterlace eInterlace=PROGRESSIVE, enumTransfer eTransfer=SDR);
        FlowVideo(const std::string& sMediaType);
        virtual bool UpdateFromJson(const Json::Value& jsData);
        virtual bool Commit(const ApiVersion& version);

        void SetFrameWidth(unsigned int nWidth);
        void SetFrameHeight(unsigned int nHeight);
        void SetColour(enumColour eColour);
        void SetInterlace(enumInterlace eInterlace);
        void SetTransfer(enumTransfer eTransfer);

    private:
        std::string m_sMediaType;
        unsigned int m_nFrameWidth;
        unsigned int m_nFrameHeight;

        enumColour m_eColour;
        enumInterlace m_eInterlace;

        enumTransfer m_eTransfer;

        static const std::string STR_INTERLACE[4];
        static const std::string STR_COLOUR[4];
        static const std::string STR_TRANSFER[3];

};


