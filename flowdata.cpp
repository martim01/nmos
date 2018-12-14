#include "flowdata.h"

FlowData::FlowData(std::string sLabel, std::string sDescription, std::string sSourceId, std::string sDeviceId, std::string sMediaType) :
    Flow(sLabel, sDescription, "urn:x-nmos:format:data", sSourceId, sDeviceId),
    m_sMediaType(sMediaType)
{

}

bool FlowData::Commit()
{
    if(Flow::Commit())
    {
        m_json["media_type"] = m_sMediaType;
        return true;
    }
    return false;
}


void FlowData::SetMediaType(std::string sMediaType)
{
    m_sMediaType = sMediaType;
    UpdateVersionTime();
}
