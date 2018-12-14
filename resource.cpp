#include "resource.h"
#ifdef __WIN__
#include <objbase.h>
#include "timeofday.h"
#endif
#ifdef __GNU__
#include <uuid/uuid.h>
#include <sys/time.h>
#endif // __GNU__

#include <sstream>

Resource::Resource(std::string sLabel, std::string sDescription) :
    m_sLabel(sLabel),
    m_sDescription(sDescription)
{
    CreateGuid();
    UpdateVersionTime();
}

void Resource::AddTag(std::string sTag)
{
    m_lstTag.push_back(sTag);
}


Json::Value Resource::ToJson() const
{
    Json::Value json;
    json["label"] = m_sLabel;
    json["description"] = m_sDescription;
    json["id"] = m_sId;

    //@todo should be time of last change
    json["version"] = m_sVersion;

    return json;
}

void Resource::CreateGuid()
{


#ifdef __WIN__
    UUID guid;
	CoCreateGuid(&guid);

    std::stringstream os;
	os << std::lowercase;
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
    os << std::nolowercase;
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
    m_sDescription;
    UpdateVersionTime();
}

void Resource::UpdateVersionTime()
{
    timeval tvNow;

    gettimeofday(&tvNow, NULL);
    std::stringstream strs;
    unsigned long nNano(tvNow.tv_usec);
    nNano*=1000;

    strs << tvNow.tv_sec << ":" << nNano;
    m_sVersion = strs.str();
}
