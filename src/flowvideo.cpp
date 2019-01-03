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


bool FlowVideo::Commit()
{
    if(Flow::Commit())
    {
        m_json["frame_width"] = m_nFrameWidth;
        m_json["frame_height"] = m_nFrameHeight;
        m_json["interlace_mode"] = STR_INTERLACE[m_eInterlace];
        m_json["colorspace"] = STR_COLOUR[m_eColour];
        m_json["transfer_characteristic"] = STR_TRANSFER[m_eTransfer];

        return true;
    }
    return false;
}


void FlowVideo::SetFrameWidth(unsigned int nWidth)
{
    m_nFrameWidth = nWidth;
    UpdateVersionTime();
}

void FlowVideo::SetFrameHeight(unsigned int nHeight)
{
    m_nFrameHeight = nHeight;
    UpdateVersionTime();
}

void FlowVideo::SetColour(enumColour eColour)
{
    m_eColour = eColour;
    UpdateVersionTime();
}

void FlowVideo::SetInterlace(enumInterlace eInterlace)
{
    m_eInterlace = eInterlace;
    UpdateVersionTime();
}

void FlowVideo::SetTransfer(enumTransfer eTransfer)
{
    m_eTransfer = eTransfer;
    UpdateVersionTime();
}

