#include "flowvideo.h"

using namespace std;

const string FlowVideo::STR_INTERLACE[4] = {"progressive", "interlaced_tff", "interlaced_bff", "interlaced_psf" };
const string FlowVideo::STR_COLOUR[4] = {"BT601", "BT709", "BT2020", "BT2100" };
const string FlowVideo::STR_TRANSFER[3] = {"SDR", "HLG", "PQ" };


FlowVideo::FlowVideo(std::string sLabel, std::string sDescription, std::string sSourceId, std::string sDeviceId, unsigned int nFrameWidth, unsigned int nFrameHeight, enumColour eColour, enumInterlace eInterlace, enumTransfer eTransfer) :
    Flow(sLabel, sDescription, "urn:x-nmos:format:video", sSourceId, sDeviceId),
    m_nFrameWidth(nFrameWidth),
    m_nFrameHeight(nFrameHeight),
    m_eColour(eColour),
    m_eInterlace(eInterlace),
    m_eTransfer(eTransfer)
{
}


Json::Value FlowVideo::ToJson() const
{
    Json::Value jsFlow(Flow::ToJson());
    jsFlow["frame_width"] = m_nFrameWidth;
    jsFlow["frame_height"] = m_nFrameHeight;
    jsFlow["interlace_mode"] = STR_INTERLACE[m_eInterlace];
    jsFlow["colorspace"] = STR_COLOUR[m_eColour];
    jsFlow["transfer_characteristic"] = STR_TRANSFER[m_eTransfer];

    return jsFlow;
}

