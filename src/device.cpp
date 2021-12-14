#include "device.h"
#include "utils.h"
#include "log.h"

using namespace pml::nmos;

const std::string Device::STR_TYPE[2] = {"urn:x-nmos:device:generic", "urn:x-nmos:device:pipeline"};

Device::Device(const std::string& sLabel, const std::string& sDescription, enumType eType, const std::string& sNodeId) :
    Resource("device", sLabel, sDescription),
    m_eType(eType),
    m_sNodeId(sNodeId)
{

}


Device::Device() : Resource("device")
{

}

std::shared_ptr<Device> Device::Create(const Json::Value& jsResponse)
{
    auto pResource  = std::make_shared<Device>();
    if(pResource->UpdateFromJson(jsResponse))
    {
        return pResource;
    }
    return nullptr;
}

bool Device::UpdateFromJson(const Json::Value& jsData)
{
    Resource::UpdateFromJson(jsData);
    if(CheckJsonRequired(jsData, {{"type",{jsondatatype::_STRING}},
                       {"node_id",{jsondatatype::_STRING}},
                       {"senders",{jsondatatype::_ARRAY}},
                       {"receivers",{jsondatatype::_ARRAY}},
                       {"controls",{jsondatatype::_ARRAY}}}) == false)
    {
        m_bIsOk = false;
        m_ssJsonError << "Device json incorrect";
        pmlLog(pml::LOG_WARN) << "NMOS: Device - json incorrect: " << jsData;
    }

    if(m_bIsOk)
    {
        if(jsData["type"].asString() == STR_TYPE[GENERIC])
        {
            m_eType = GENERIC;
        }
        else
        {
            m_eType = PIPELINE;
        }
        m_sNodeId = jsData["node_id"].asString();

        for(Json::ArrayIndex ai = 0; ai < jsData["senders"].size(); ++ai)
        {
            if(jsData["senders"][ai].isString() == false)
            {
                m_bIsOk = false;
                m_ssJsonError << "'senders' entry #" << ai << " not a string" ;
                break;
            }
            else
            {
                m_setSenders.insert(jsData["senders"][ai].asString());
            }
        }
        for(Json::ArrayIndex ai = 0; ai < jsData["receivers"].size(); ++ai)
        {
            if(jsData["receivers"][ai].isString() == false)
            {
                m_bIsOk = false;
                m_ssJsonError << "'receivers' entry #" << ai << " not a string" ;
                break;
            }
            else
            {
                m_setReceivers.insert(jsData["receivers"][ai].asString());
            }
        }
        for(Json::ArrayIndex ai = 0; ai < jsData["controls"].size(); ai++)
        {
            if(jsData["controls"][ai].isObject() == false)
            {
                m_ssJsonError << "'controls' entry #" << ai << " not an object" ;
                m_bIsOk = false;
                break;
            }
            if(jsData["controls"][ai]["href"].isString() == false)
            {
                m_ssJsonError << "'controls' entry #" << ai << " 'href' not a string" ;
                m_bIsOk = false;
                break;
            }
            if(jsData["controls"][ai]["type"].isString() == false)
            {
                m_bIsOk = false;
                m_ssJsonError << "'controls' entry #" << ai << " type not a string" ;
                break;
            }
            else
            {
                m_mmControls.insert({control(jsData["controls"][ai]["type"].asString()), endpoint(jsData["controls"][ai]["href"].asString())});
            }
        }
    }
    return m_bIsOk;
}

bool Device::Commit(const ApiVersion& version)
{
    if(Resource::Commit(version))
    {   //something has changed
        m_json["type"] = STR_TYPE[m_eType];
        m_json["node_id"] = m_sNodeId;
        m_json["senders"] = Json::Value(Json::arrayValue);
        for(std::set<std::string>::const_iterator itSender = m_setSenders.begin(); itSender != m_setSenders.end(); ++itSender)
        {
            m_json["senders"].append((*itSender));
        }

        m_json["receivers"] = Json::Value(Json::arrayValue);
        for(std::set<std::string>::const_iterator itReceiver = m_setReceivers.begin(); itReceiver != m_setReceivers.end(); ++itReceiver)
        {
            m_json["receivers"].append((*itReceiver));
        }

        m_json["controls"] = Json::Value(Json::arrayValue);
        for(const auto& pairControl : m_mmControls)
        {
            Json::Value jsControl = Json::Value(Json::objectValue);
            jsControl["type"] = pairControl.first.Get();
            jsControl["href"] = pairControl.second.Get();
            m_json["controls"].append(jsControl);
        }
        return true;
    }
    return false;
}

void Device::AddControl(const control& type, const endpoint& theEndpoint)
{
    m_mmControls.insert({type, theEndpoint});

    UpdateVersionTime();
}
void Device::RemoveControl(const control& type, const endpoint& theEndpoint)
{
    for(auto itControl = m_mmControls.lower_bound(type); itControl != m_mmControls.upper_bound(type); ++itControl)
    {
        if(itControl->second == theEndpoint)
        {
            m_mmControls.erase(itControl);
            break;
        }
    }
    UpdateVersionTime();
}

void Device::ChangeType(enumType eType)
{
    m_eType = eType;
    UpdateVersionTime();
}



endpoint Device::GetPreferredUrl(const control& type) const
{
    auto itPref = m_mPreferred.find(type);
    if(itPref != m_mPreferred.end())
    {
        return itPref->second;
    }
    return endpoint("");
}

void Device::SetPreferredUrl(const control& type, const endpoint& theEndpoint)
{
    m_mPreferred[type] = theEndpoint;
}

void Device::RemovePreferredUrl(const control& type)
{
    m_mPreferred.erase(type);
}

void Device::AddSender(const std::string& sId)
{
    m_setSenders.insert(sId);
    UpdateVersionTime();
}

void Device::AddReceiver(const std::string& sId)
{
    m_setReceivers.insert(sId);
    UpdateVersionTime();
}

void Device::RemoveSender(const std::string& sId)
{
    m_setSenders.erase(sId);
    UpdateVersionTime();
}

void Device::RemoveReceiver(const std::string& sId)
{
    m_setReceivers.erase(sId);
    UpdateVersionTime();
}
