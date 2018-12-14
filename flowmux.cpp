#include "flowmux.h"

FlowMux::FlowMux(std::string sLabel, std::string sDescription, std::string sSourceId, std::string sDeviceId, std::string sMediaType) :
    Flow(sLabel, sDescription, "urn:x-nmos:format:mux", sSourceId, sDeviceId),
    m_sMediaType(sMediaType)
{

}

Json::Value FlowMux::ToJson() const
{
    Json::Value jsFlow(Flow::ToJson());


    jsFlow["media_type"] = m_sMediaType;

    return jsFlow;
}
