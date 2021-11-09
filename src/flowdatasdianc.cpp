#include "flowdatasdianc.h"

FlowDataSdiAnc::FlowDataSdiAnc(const std::string& sLabel, const std::string& sDescription, const std::string& sSourceId, const std::string& sDeviceId) :
    FlowData(sLabel, sDescription, sSourceId, sDeviceId, "video/smpte291")
{

}

FlowDataSdiAnc::FlowDataSdiAnc() : FlowData("video/smpte291")
{

}

bool FlowDataSdiAnc::UpdateFromJson(const Json::Value& jsData)
{
    FlowData::UpdateFromJson(jsData);
    if(jsData["DID_SDID"].isArray() == false)
    {
        m_bIsOk = false;
        m_ssJsonError << "'DID_SDID' is not an array" ;
    }
    else if(jsData["DID_SDID"].empty())
    {
        m_bIsOk = false;
        m_ssJsonError << "'DID_SDID' cannot be an empty array" ;
    }

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
                m_ssJsonError << "'DID_SDID' #" << ai << "'DID' or 'SDID' is not a string" ;
                break;
            }
        }
    }
    return m_bIsOk;
}

bool FlowDataSdiAnc::Commit(const ApiVersion& version)
{

    if(FlowData::Commit(version))
    {
        if(m_lstWords.empty())
        {
            m_json["DID_SDID"] = Json::arrayValue;
            for(std::list<std::pair<std::string, std::string> >::const_iterator itWord = m_lstWords.begin(); itWord != m_lstWords.end(); ++itWord)
            {
                Json::Value jsWord(Json::objectValue);
                jsWord["DID"] = (*itWord).first;
                jsWord["SDID"] = (*itWord).second;
                m_json["DID_SDID"].append(jsWord);
            }
        }
        return true;
    }
    return false;

}


