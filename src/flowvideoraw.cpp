#include "flowvideoraw.h"

using namespace std;


const string FlowVideoRaw::STR_COMPONENT[11] = { "Y", "Cb", "Cr", "I", "Ct", "Cp", "A", "R", "G", "B", "DepthMap"};



FlowVideoRaw::FlowVideoRaw(string sLabel, string sDescription, string sSourceId, string sDeviceId, unsigned int nFrameWidth, unsigned int nFrameHeight, enumColour eColour, enumInterlace eInterlace, enumTransfer eTransfer) :
    FlowVideo(sLabel, sDescription, sSourceId, sDeviceId, nFrameWidth, nFrameHeight, eColour, eInterlace, eTransfer)
{

}


void FlowVideoRaw::AddComponent(enumComponent eComponent, unsigned int nWidth, unsigned int nHeight, unsigned int nBitDepth)
{
    m_mComponent.insert(make_pair(eComponent, component(nWidth, nHeight, nBitDepth)));
    UpdateVersionTime();
}

void FlowVideoRaw::RemoveComponent(enumComponent eComponent)
{
    m_mComponent.erase(eComponent);
    UpdateVersionTime();
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
