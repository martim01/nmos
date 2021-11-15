#include "flowsdpcreatornode.h"
#include "flowaudiocoded.h"
#include "flowaudioraw.h"
#include "flowvideocoded.h"
#include "flowvideoraw.h"
#include "flowdatasdianc.h"
#include "flowmux.h"
#include "nodeapiprivate.h"
#include <sstream>
#include "sourceaudio.h"

using namespace pml::nmos;

std::string CreateFlowSdpLines(NodeApiPrivate& api, std::shared_ptr<Flow> pFlow, unsigned short nRtpPort)
{
    //@todo this feels a bodge
    auto pfar = std::dynamic_pointer_cast<FlowAudioRaw>(pFlow);
    if(pfar)
    {
        return CreateFlowSdpLines(api, pfar, nRtpPort);
    }

    auto pfac = std::dynamic_pointer_cast<FlowAudioCoded>(pFlow);
    if(pfac)
    {
        return CreateFlowSdpLines(api, pfac, nRtpPort);
    }

    return std::string();
}

std::string CreateFlowSdpLines(NodeApiPrivate& api, std::shared_ptr<FlowAudioRaw> pFlow, unsigned short nRtpPort)
{
    if(!pFlow)
        return "";

    std::stringstream sstr;

    sstr << "m=audio " << nRtpPort << " RTP/AVP 96\r\n";

    sstr << "a=rtpmap:96 L";
    switch(pFlow->GetFormat())
    {
    case pml::nmos::FlowAudioRaw::L24:
        sstr << "24/";
        break;
    case pml::nmos::FlowAudioRaw::L20:
        sstr << "20/";
        break;
    case pml::nmos::FlowAudioRaw::L16:
        sstr << "16/";
        break;
    case pml::nmos::FlowAudioRaw::L8:
        sstr << "8/";
        break;
    }

    sstr << pFlow->GetSampleRateNumerator() << "/";

    //Get the number of channels from the associated source
    auto itResource = api.GetSources().FindNmosResource(pFlow->GetSourceId());
    if(itResource != api.GetSources().GetResourceEnd())
    {
        auto pSource = std::dynamic_pointer_cast<pml::nmos::SourceAudio>(itResource->second);
        if(pSource)
        {
            sstr << pSource->GetNumberOfChannels();
        }
        else
        {
            sstr << "0";
        }
    }
    else
    {
        sstr << "0";
    }
    sstr << "\r\n";

    // @todo channel order - needed in SMPTE2110. Not needed in AES67 but it shouldnt matter to include it


    //packet time
    switch(pFlow->GetPacketTime())
    {
        case pml::nmos::FlowAudioRaw::US_125:
            sstr << "a=ptime:0.125\r\n";
            break;
        case pml::nmos::FlowAudioRaw::US_250:
            sstr << "a=ptime:0.250\r\n";
            break;
        case pml::nmos::FlowAudioRaw::US_333:
            sstr << "a=ptime:0.333\r\n";
            break;
        case pml::nmos::FlowAudioRaw::US_1000:
            sstr << "a=ptime:1\r\n";
            break;
        case pml::nmos::FlowAudioRaw::US_4000:
            sstr << "a=ptime:4\r\n";
            break;
    }

    //mediaclk:direct=
    sstr << "a=mediaclk:direct=" << pFlow->GetMediaClkOffset() << "\r\n";

    return sstr.str();
}


std::string CreateFlowSdpLines(NodeApiPrivate& api, std::shared_ptr<FlowAudioCoded> pFlow, unsigned short nRtpPort)
{
    if(!pFlow)
        return "";

    std::stringstream sstr;
/*
    if(m_json["media_type"].isString())
    {
        std::string sMedia = m_json["media_type"].asString();
        std::map<std::string, unsigned short>::const_iterator itType = m_mRtpTypes.find(sMedia);
        if(itType != m_mRtpTypes.end())
        {
            sstr << "m=audio " << nRtpPort << " RTP/AVP " << itType->second << "\r\n"; //this is not 96 its the actual number
        }
        else
        {
            sstr << "m=audio " << nRtpPort << " RTP/AVP 103\r\n";
            sstr << "a=rtpmap:103 " << sMedia << "\r\n";    // @todo check what more is needed here ofr different audio codings
        }

    }
    */
    return sstr.str();
}
