#pragma once
#include "flowvideo.h"
#include "nmosdlldefine.h"

class NMOS_EXPOSE FlowVideoRaw : public FlowVideo
{
    public:
        FlowVideoRaw(const std::string& sLabel, const std::string& sDescription, const std::string& sSourceId, const std::string& sDeviceId, unsigned int nFrameWidth, unsigned int nFrameHeight, enumColour eColour, enumInterlace eInterlace=PROGRESSIVE, enumTransfer eTransfer=SDR);
        FlowVideoRaw();
        virtual bool UpdateFromJson(const Json::Value& jsData);
        enum enumComponent{Y, CB, CR, I, CT, CP, A,R, G,B, DEPTHMAP};

        void AddComponent(enumComponent eComponent, unsigned int nWidth, unsigned int nHeight, unsigned int nBitDepth);
        void RemoveComponent(enumComponent eComponent);

        virtual bool Commit(const ApiVersion& version);

    private:
        struct component
        {
            component(unsigned int nW, unsigned int nH, unsigned int nB) : nWidth(nW), nHeight(nH), nBitDepth(nB){}
            unsigned int nWidth;
            unsigned int nHeight;
            unsigned int nBitDepth;
        };

        std::map<enumComponent, component> m_mComponent;

        static const std::string STR_COMPONENT[11];
};
