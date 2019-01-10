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
    m_bIsOk &= (jsData["components"].isArray() && jsData["components"].size() >=1);
    if(m_bIsOk)
    {
        for(Json::ArrayIndex ai = 0; ai < jsData["components"].size(); ++ai)
        {
            if(jsData["components"][ai].isOject() == false || jsData["components"][ai]["name"].isString() == false ||jsData["components"][ai]["width"].isInt() == false || jsData["components"][ai]["height"].isInt() == false || jsData["components"][ai]["bit_depth"].isInt() == false)
            {
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
                        m_mComponent.insert(make_pair(i, component aComp(jsData["components"][ai]["width"].asInt(), jsData["components"][ai]["height"].asInt(), jsData["components"][ai]["bit_depth"].asInt())));
                        break;
                    }
                }
                m_bIsOk &= bFound;

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
