#include "flowdatasdianc.h"

FlowDataSdiAnc::FlowDataSdiAnc(std::string sLabel, std::string sDescription, std::string sSourceId, std::string sDeviceId, ) :
    FlowData(sLabel, sDescription, sSourceId, sDeviceId, "video/smpte291")
{

}

FlowDataSdiAnc::FlowDataSdiAnc() : FlowData("video/smpte291")
{

}

bool FlowDataSdiAnc::UpdateFromJson(const Json::Value& jsData)
{
    FlowData::UpdateFromJson(jsData);
    m_bIsOk &= (jsData["DID_SDID"].isArray() || jsData["DID_SDID"].isEmpty());

    if(jsData["DID_SDID"].isArray())
    {
        for(Json::ArrayIndex ai = 0; ai < jsData["DID_SDID"].size(); ai++)
        {
            if(jsData["DID_SDID"][ai]["DID"].isString() && jsData["DID_SDID"][ai]["SDID"].isString())
            {
                m_lstWords.push_back(std::make_pair(jsData["DID_SDID"][ai]["DID"].asString(), jsData["DID_SDID"][ai]["SDID"].asString()));
            }
            else
            {
                m_bIsOk = false;
                break;
            }
        }
    }
    return m_bIsOk;
}

bool FlowDataSdiAnc::Commit()
{

    if(FlowData::Commit())
    {
        if(m_lstWords.empty())
        {
            m_json["DID_SDID"] = Json::arrayValue;
            for(std::list<std::pair<std::string, std::string> >::const_iterator itWord = m_lstWords.begin(); itWord != m_lstWords.end(); ++itWord)
            {
                Json::objectValue jsWord;
                jsWord["DID"] = (*itWord).first;
                jsWord["SDID"] = (*itWord).second;
                m_json["DID_SDID"].append(jsWord);
            }
        }
        return true;
    }
    return false;

}


std::string FlowDataSdiAnc::CreateSDPLines(unsigned short nRtpPort) const
{
    // @todo create FlowDataSdiAnc SDP information
    return "";
}
