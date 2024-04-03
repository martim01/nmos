#include "resource.h"
#include <chrono>
#include <sstream>
#include <string>
#include "utils.h"
#include <iostream>

using namespace pml::nmos;

std::string Resource::s_sBase("");


Resource::Resource(const std::string& sType, const std::string& sLabel, const std::string& sDescription) :
    m_bIsOk(true),
    m_sType(sType),
    m_sId(CreateGuid(s_sBase+sType+sLabel)),
    m_sLabel(sLabel),
    m_sDescription(sDescription),
    m_nHeartbeat(0)

{
     UpdateVersionTime();
}

Resource::Resource(const std::string& sType) :
    m_bIsOk(true),
    m_sType(sType),
    m_sId(CreateGuid(s_sBase+sType))
{
    UpdateVersionTime();

}

bool Resource::UpdateFromJson(const Json::Value& jsValue)
{
    m_json = jsValue;
    m_ssJsonError.str(std::string());

    if(m_json["id"].isString() == false)
    {
        m_bIsOk = false;
        m_ssJsonError << "'id' not a string" ;
    }
    if(m_json["label"].isString()==false)
    {
        m_bIsOk = false;
        m_ssJsonError << "'label' not a string" ;
    }
    if(m_json["description"].isString()==false)
    {
        m_bIsOk = false;
        m_ssJsonError << "'description' not a string" ;
    }
    if(m_json["version"].isString() ==false)
    {
        m_bIsOk = false;
        m_ssJsonError << "'version' not a string" ;
    }
    if(m_json["tags"].isObject() == false)
    {
        m_bIsOk = false;
        m_ssJsonError << "'tags' not an object" ;
    }
    if(m_bIsOk)
    {
        m_sId = m_json["id"].asString();
        m_sLabel = m_json["label"].asString();
        m_sDescription = m_json["description"].asString();
        m_sVersion = m_json["version"].asString();

        Json::Value::Members vMembers = m_json["tags"].getMemberNames();
        for(size_t i = 0; i < vMembers.size(); i++)
        {
            if(m_json["tags"][vMembers[i]].isArray())
            {
                for(Json::ArrayIndex ai = 0; ai < m_json["tags"][vMembers[i]].size(); ++ai)
                {
                    if(m_json["tags"][vMembers[i]][ai].isString())
                    {
                        m_mmTag.insert(make_pair(vMembers[i], m_json["tags"][vMembers[i]][ai].asString()));
                    }
                }
            }
        }
    }
    return m_bIsOk;
}


bool Resource::IsOk() const
{
    return m_bIsOk;
}

void Resource::AddTag(const std::string& sKey, const std::string& sValue)
{
    m_mmTag.insert(std::make_pair(sKey, sValue));
    UpdateVersionTime();
}

void Resource::AddTags(const std::list<std::pair<std::string, std::string>>& lstTags)
{
    for(const auto& tag : lstTags)
    {
        m_mmTag.insert(tag);
    }
}


bool Resource::Commit(const ApiVersion& version)
{
    if(m_sVersion == m_sLastVersion)
    {   //no changes to resource
        return false;
    }

    m_json.clear();

    m_json["label"] = m_sLabel;
    m_json["description"] = m_sDescription;
    m_json["id"] = m_sId;

    m_json["tags"] = Json::Value(Json::objectValue);
    for(std::multimap<std::string, std::string>::iterator itTag = m_mmTag.begin(); itTag != m_mmTag.end(); ++itTag)
    {
        if(m_json["tags"][itTag->first].empty())
        {
            m_json["tags"][itTag->first] = Json::Value(Json::arrayValue);
        }
        m_json["tags"][itTag->first].append(itTag->second);
    }

    m_json["version"] = m_sVersion;

    m_sLastVersion = m_sVersion;
    return true;
}


void Resource::UpdateLabel(const std::string& sLabel)
{
    m_sLabel = sLabel;
    UpdateVersionTime();
}

void Resource::UpdateDescription(const std::string& sDescription)
{
    m_sDescription = sDescription;
    UpdateVersionTime();
}

void Resource::UpdateVersionTime()
{
    m_sLastVersion = m_sVersion;

    m_sVersion = GetCurrentTaiTime();
}



const Json::Value& Resource::GetJson(const ApiVersion& version) const
{
    return m_json;
}


const std::string& Resource::GetId() const
{
    return m_sId;
}

const std::string& Resource::GetLabel() const
{
    return m_sLabel;
}

const std::string& Resource::GetDescription() const
{
    return m_sDescription;
}

const std::string& Resource::GetVersion() const
{
    return m_sVersion;
}



const std::string& Resource::GetType() const
{
    return m_sType;
}


size_t Resource::GetLastHeartbeat() const
{
    return m_nHeartbeat;
}

void Resource::SetHeartbeat()
{
    m_nHeartbeat =GetCurrentHeartbeatTime();

}


std::string Resource::GetJsonParseError()
{
    return m_ssJsonError.str();
}

