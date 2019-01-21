#include "resource.h"
#include <chrono>
#include <sstream>
#include <string>



#ifdef __GNU__
#include <uuid/uuid.h>
#include <sys/time.h>
#endif // __GNU__


Resource::Resource(const std::string& sType, const std::string& sLabel, const std::string& sDescription) :
    m_bIsOk(true),
    m_sType(sType),
    m_sLabel(sLabel),
    m_sDescription(sDescription),
    m_nHeartbeat(0)

{
    CreateGuid();
    UpdateVersionTime();
}

Resource::Resource(const std::string& sType) :
    m_bIsOk(true),
    m_sType(sType)
{
    CreateGuid();
    UpdateVersionTime();
}

bool Resource::UpdateFromJson(const Json::Value& jsValue)
{
    m_json = jsValue;
    m_ssJsonError.str(std::string());

    if(m_json["id"].isString() == false)
    {
        m_bIsOk = false;
        m_ssJsonError << "'id' not a string" << std::endl;
    }
    if(m_json["label"].isString()==false)
    {
        m_bIsOk = false;
        m_ssJsonError << "'label' not a string" << std::endl;
    }
    if(m_json["description"].isString()==false)
    {
        m_bIsOk = false;
        m_ssJsonError << "'description' not a string" << std::endl;
    }
    if(m_json["version"].isString() ==false)
    {
        m_bIsOk = false;
        m_ssJsonError << "'version' not a string" << std::endl;
    }
    if(m_json["tags"].isObject() == false)
    {
        m_bIsOk = false;
        m_ssJsonError << "'tags' not an object" << std::endl;
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


bool Resource::Commit(const ApiVersion& version)
{
    if(m_sVersion == m_sLastVersion)
    {   //no changes to resource
 //       return false;
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

void Resource::CreateGuid()
{

#ifdef __GNUWIN32__
    UUID guid;
	CoCreateGuid(&guid);

    std::stringstream os;
//	os << std::lowercase;
    os.width(8);
    os << std::hex << guid.Data1 << '-';

    os.width(4);
    os << std::hex << guid.Data2 << '-';

    os.width(4);
    os << std::hex << guid.Data3 << '-';

    os.width(2);
    os << std::hex
        << static_cast<short>(guid.Data4[0])
        << static_cast<short>(guid.Data4[1])
        << '-'
        << static_cast<short>(guid.Data4[2])
        << static_cast<short>(guid.Data4[3])
        << static_cast<short>(guid.Data4[4])
        << static_cast<short>(guid.Data4[5])
        << static_cast<short>(guid.Data4[6])
        << static_cast<short>(guid.Data4[7]);
  //  os << std::nolowercase;
    m_sId = os.str();
#endif // __WIN__

#ifdef __GNU__
    uuid_t guid;
    uuid_generate(guid);

    char uuid_str[37];      // ex. "1b4e28ba-2fa1-11d2-883f-0016d3cca427" + "\0"
    uuid_unparse_lower(guid, uuid_str);
    m_sId = uuid_str;
#endif // __GNU__
}

void Resource::UpdateLabel(std::string sLabel)
{
    m_sLabel = sLabel;
    UpdateVersionTime();
}

void Resource::UpdateDescription(std::string sDescription)
{
    m_sDescription = sDescription;
    UpdateVersionTime();
}

void Resource::UpdateVersionTime()
{
    m_sLastVersion = m_sVersion;

    m_sVersion = GetCurrentTime();
}

std::string Resource::GetCurrentTime(bool bIncludeNano)
{
    auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    std::stringstream sstr;

    sstr << (nanos/1000000000);
    if(bIncludeNano)
    {
        sstr << ":" << (nanos%1000000000);
    }
    return sstr.str();
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


bool Resource::ConvertTaiStringToTimePoint(const std::string& sTai, std::chrono::time_point<std::chrono::high_resolution_clock>& tp)
{
    std::istringstream f(sTai);
    std::string sSeconds;
    std::string sNano;

    if(getline(f, sSeconds, ':') && getline(f, sNano, ':'))
    {
        try
        {
            std::chrono::seconds sec(std::stoul(sSeconds));
            std::chrono::nanoseconds nano(std::stoul(sNano));
            nano += std::chrono::duration_cast<std::chrono::nanoseconds>(sec);
            tp = std::chrono::time_point<std::chrono::high_resolution_clock>(nano);

        }
        catch(std::invalid_argument& e)
        {
            return false;
        }
    }
    return false;
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
    auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    m_nHeartbeat =(nanos/1000000000);
}


std::string Resource::GetJsonParseError()
{
    return m_ssJsonError.str();
}
