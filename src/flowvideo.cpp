#include "flowvideo.h"

using namespace std;

const string FlowVideo::STR_INTERLACE[4] = {"progressive", "interlaced_tff", "interlaced_bff", "interlaced_psf" };
const string FlowVideo::STR_COLOUR[4] = {"BT601", "BT709", "BT2020", "BT2100" };
const string FlowVideo::STR_TRANSFER[3] = {"SDR", "HLG", "PQ" };


FlowVideo::FlowVideo(const std::string& sLabel, const std::string& sDescription, const std::string& sSourceId, const std::string& sDeviceId, const std::string& sMediaType, unsigned int nFrameWidth, unsigned int nFrameHeight, enumColour eColour, enumInterlace eInterlace, enumTransfer eTransfer) :
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
    if(jsData["frame_width"].isInt() == false)
    {
        m_bIsOk = false;
        m_ssJsonError << "'frame_width' is not an int" << std::endl;
    }
    if(jsData["frame_height"].isInt() == false)
    {
        m_bIsOk = false;
        m_ssJsonError << "'frame_height' is not an int" << std::endl;
    }
    if(jsData["colorspace"].isString() == false)
    {
        m_bIsOk = false;
        m_ssJsonError << "'colorspace' is not a string" << std::endl;
    }
    if(jsData["interlace_mode"].isString() == false && jsData["interlace_mode"].empty() == false)
    {
        m_bIsOk = false;
        m_ssJsonError << "'interlace_mode' is not a string and not empty" << std::endl;
    }
    if(jsData["transfer_characteristic"].isString() == false && jsData["transfer_characteristic"].empty()== false)
    {
        m_bIsOk = false;
        m_ssJsonError << "'transfer_characteristic' is not a string and not empty" << std::endl;
    }

    if(m_bIsOk)
    {
        m_nFrameWidth = jsData["frame_width"].asInt();
        m_nFrameHeight = jsData["frame_height"].asInt();

        bool bFound(false);
        for(int i = 0; i < 4; i++)
        {
            if(STR_COLOUR[i] == jsData["colorspace"].asString())
            {
                m_eColour = enumColour(i);
                bFound = true;
                break;
            }
        }
        if(!bFound)
        {
            m_ssJsonError << "'colorspace' is not valid" << std::endl;
            m_bIsOk = false;
        }

        if(jsData["interlace_mode"].isString())
        {
            bFound = false;
            for(int i = 0; i < 4; i++)
            {
                if(STR_INTERLACE[i] == jsData["interlace_mode"].asString())
                {
                    m_eInterlace = enumInterlace(i);
                    bFound = true;
                    break;
                }
            }
            if(!bFound)
            {
                m_ssJsonError << "'interlace_mode' is not valid" << std::endl;
                m_bIsOk = false;
            }

        }

        if(jsData["transfer_characteristic"].isString())
        {
            bFound = false;
            for(int i = 0; i < 3; i++)
            {
                if(STR_TRANSFER[i] == jsData["transfer_characteristic"].asString())
                {
                    m_eTransfer = enumTransfer(i);
                    bFound = true;
                    break;
                }
            }
            if(!bFound)
            {
                m_ssJsonError << "'transfer_characteristic' is not valid" << std::endl;
                m_bIsOk = false;
            }

        }
    }
    return m_bIsOk;
}

bool FlowVideo::Commit(const ApiVersion& version)
{
    if(Flow::Commit(version))
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

