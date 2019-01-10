#include "flowvideo.h"

using namespace std;

const string FlowVideo::STR_INTERLACE[4] = {"progressive", "interlaced_tff", "interlaced_bff", "interlaced_psf" };
const string FlowVideo::STR_COLOUR[4] = {"BT601", "BT709", "BT2020", "BT2100" };
const string FlowVideo::STR_TRANSFER[3] = {"SDR", "HLG", "PQ" };


FlowVideo::FlowVideo(std::string sLabel, std::string sDescription, std::string sSourceId, std::string sDeviceId, const std::string& sMediaType, unsigned int nFrameWidth, unsigned int nFrameHeight, enumColour eColour, enumInterlace eInterlace, enumTransfer eTransfer) :
    Flow(sLabel, sDescription, "urn:x-nmos:format:video", sSourceId, sDeviceId),
    m_sMediaType(sMediaType),
    m_nFrameWidth(nFrameWidth),
    m_nFrameHeight(nFrameHeight),
    m_eColour(eColour),
    m_eInterlace(eInterlace),
    m_eTransfer(eTransfer)
{
}

FlowVideo::FlowVideo(const std::string& sMediaType) : Flow("urn:x-nmos:format:video"),
    m_sMediaType(sMediaType)
{

}

bool FlowVideo::UpdateFromJson(const Json::Value& jsData)
{
    Flow::UpdateFromJson(jsData);
    m_bIsOk &= (jsData["frame_width"].isInt() && jsData["frame_height"].isInt() &&  jsData["colorspace"].isString() &&
               (jsData["interlace_mode"].isString() || jsData["interlace_mode"].isEmpty()) &&
               (jsData["transfer_characteristic"].isString() || jsData["transfer_characteristic"].isEmpty()));

    if(m_bIsOk)
    {
        m_nFrameWidth = jsData["frame_width"].asInt();
        m_nFrameHeight = jsData["frame_height"].asInt();

        bool bFound(false);
        for(int i = 0; i < 4; i++)
        {
            if(STR_COLOUR[i] == jsData["colorspace"])
            {
                m_eColour = i;
                bFound = true;
                break;
            }
        }
        m_bIsOk &= bFound;

        if(jsData["interlace_mode"].isString())
        {
            bFound = false;
            for(int i = 0; i < 4; i++)
            {
                if(STR_INTERLACE[i] == jsData["interlace_mode"])
                {
                    m_eInterlace = i;
                    bFound = true;
                    break;
                }
            }
            m_bIsOk &= bFound;
        }

        if(jsData["transfer_characteristic"].isString())
        {
            bFound = false;
            for(int i = 0; i < 3; i++)
            {
                if(STR_TRANSFER[i] == jsData["transfer_characteristic"])
                {
                    m_eTransfer = i;
                    bFound = true;
                    break;
                }
            }
            m_bIsOk &= bFound;
        }
    }
    return m_bIsOk;
}

bool FlowVideo::Commit()
{
    if(Flow::Commit())
    {
        m_json["media_type"] = m_sMediaType;
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

