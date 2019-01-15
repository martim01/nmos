#include "flowvideoraw.h"

using namespace std;


const string FlowVideoRaw::STR_COMPONENT[11] = { "Y", "Cb", "Cr", "I", "Ct", "Cp", "A", "R", "G", "B", "DepthMap"};



FlowVideoRaw::FlowVideoRaw(string sLabel, string sDescription, string sSourceId, string sDeviceId, unsigned int nFrameWidth, unsigned int nFrameHeight, enumColour eColour, enumInterlace eInterlace, enumTransfer eTransfer) :
    FlowVideo(sLabel, sDescription, sSourceId, sDeviceId, "vidoe/raw", nFrameWidth, nFrameHeight, eColour, eInterlace, eTransfer)
{

}

FlowVideoRaw::FlowVideoRaw() : FlowVideo("vidoe/raw")
{

}

bool FlowVideoRaw::UpdateFromJson(const Json::Value& jsData)
{
    FlowVideo::UpdateFromJson(jsData);
    if(jsData["components"].isArray() == false || jsData["components"].size() == 0)
    {
        m_ssJsonError << "'components' is not an array or is an empty array" << std::endl;
        m_bIsOk = false;
    }
    if(m_bIsOk)
    {
        for(Json::ArrayIndex ai = 0; ai < jsData["components"].size(); ++ai)
        {
            if(jsData["components"][ai].isObject() == false)
            {
                m_ssJsonError << "'components' #" << ai << " is not an object" << std::endl;
                m_bIsOk = false;
                break;
            }
            if(jsData["components"][ai]["name"].isString() == false)
            {
                m_ssJsonError << "'components' #" << ai << " 'name' is not a string" << std::endl;
                m_bIsOk = false;
                break;
            }
            if(jsData["components"][ai]["width"].isInt() == false)
            {
                m_ssJsonError << "'components' #" << ai << " 'width' is not an int" << std::endl;
                m_bIsOk = false;
                break;
            }
            if(jsData["components"][ai]["height"].isInt() == false)
            {
                m_ssJsonError << "'components' #" << ai << " 'height' is not an int" << std::endl;
                m_bIsOk = false;
                break;
            }
            if(jsData["components"][ai]["bit_depth"].isInt() == false)
            {
                m_ssJsonError << "'components' #" << ai << " 'bit_depth' is not an int" << std::endl;
                m_bIsOk = false;
                break;
            }
            else
            {
                bool bFound(false);
                int i = 0;
                for(; i < 11; i++)
                {
                    if(jsData["components"][ai]["name"] == STR_COMPONENT[i])
                    {
                        bFound = true;
                        m_mComponent.insert(make_pair(enumComponent(i), component(jsData["components"][ai]["width"].asInt(), jsData["components"][ai]["height"].asInt(), jsData["components"][ai]["bit_depth"].asInt())));
                        break;
                    }
                }
                if(!bFound)
                {
                    m_ssJsonError << "'components' #" << ai << " 'name' is not valid" << std::endl;
                    m_bIsOk = false;
                }

            }
        }
    }
    return m_bIsOk;
}

void FlowVideoRaw::AddComponent(enumComponent eComponent, unsigned int nWidth, unsigned int nHeight, unsigned int nBitDepth)
{
    m_mComponent.insert(make_pair(eComponent, component(nWidth, nHeight, nBitDepth)));

}

void FlowVideoRaw::RemoveComponent(enumComponent eComponent)
{
    m_mComponent.erase(eComponent);

}

bool FlowVideoRaw::Commit()
{
    if(FlowVideo::Commit())
    {
        m_json["components"] = Json::Value(Json::arrayValue);
        for(map<enumComponent, component>::const_iterator itComponent = m_mComponent.begin(); itComponent != m_mComponent.end(); ++itComponent)
        {
            Json::Value jsComponent(Json::objectValue);
            jsComponent["name"] = STR_COMPONENT[itComponent->first];
            jsComponent["width"] = itComponent->second.nWidth;
            jsComponent["height"] = itComponent->second.nHeight;
            jsComponent["bit_depth"] = itComponent->second.nBitDepth;

            m_json["components"].append(jsComponent);
        }
        return true;
    }
    return false;
}


std::string FlowVideoRaw::CreateSDPLines(unsigned short nRtpPort) const
{
    // @todo create VideoRaw SDP information
    return "";
}
