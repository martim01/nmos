#include "flowdatasdianc.h"

FlowDataSdiAnc::FlowDataSdiAnc(std::string sLabel, std::string sDescription, std::string sSourceId, std::string sDeviceId, std::string sMediaType) :
    FlowData(sLabel, sDescription, sSourceId, sDeviceId, sMediaType)
{

}

bool FlowDataSdiAnc::Commit()
{
    return FlowData::Commit();

}
