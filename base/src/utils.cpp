#include "utils.h"
#include <istream>
#include <sstream>
#ifdef __GNU__
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
//#include <uuid/uuid.h>
#include <sys/time.h>
#endif // __GNU__
#include <chrono>
#include <sstream>
#include "log.h"
#include "guid.h"
#include <array>


using namespace std;


static uuid_t NameSpace_OID = { /* 6ba7b812-9dad-11d1-80b4-00c04fd430c8 */
       0x6ba7b812,
       0x9dad,
       0x11d1,
       0x80, 0xb4, 0x00, 0xc0, 0x4f, 0xd4, 0x30, 0xc8
   };

void SplitString(vector<string>& vSplit, const std::string& str, char cSplit)
{
    vSplit.clear();
    istringstream f(str);
    string s;
    while (getline(f, s, cSplit))
    {
        if(s.empty() == false)
        {
            vSplit.push_back(s);
        }
    }
}

vector<string> SplitString(const std::string& str, char cSplit)
{
    std::vector<std::string> vSplit;
    SplitString(vSplit, str, cSplit);
    return vSplit;
}



string GetIpAddress(const string& sInterface)
{
    #ifdef __GNU__
    int fd = socket(AF_INET, SOCK_DGRAM,0);
    ifreq ifr;
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy((char*)ifr.ifr_ifrn.ifrn_name, sInterface.c_str(), IFNAMSIZ-1);
    ioctl(fd, SIOCGIFADDR, &ifr);
    close(fd);


    return inet_ntoa((((sockaddr_in*)&ifr.ifr_addr)->sin_addr));
    #else
    return "";
    #endif
}


size_t GetCurrentHeartbeatTime()
{
    std::chrono::time_point<std::chrono::high_resolution_clock> tp(std::chrono::high_resolution_clock::now());
    tp += LEAP_SECONDS;
    return std::chrono::duration_cast<std::chrono::seconds>(tp.time_since_epoch()).count();

}

std::string GetCurrentTaiTime(bool bIncludeNano)
            {
    return ConvertTimeToString(GetTaiTimeNow(), bIncludeNano);
}

std::chrono::time_point<std::chrono::high_resolution_clock> GetTaiTimeNow()
{
    auto tp = std::chrono::high_resolution_clock::now();
    tp += LEAP_SECONDS;
    return tp;
}

std::string ConvertTimeToString(std::chrono::time_point<std::chrono::high_resolution_clock> tp, bool bIncludeNano)
{
    std::stringstream sstr;
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(tp.time_since_epoch());
    sstr << seconds.count();
    if(bIncludeNano)
    {
        sstr << ":" << (std::chrono::duration_cast<std::chrono::nanoseconds>(tp.time_since_epoch()).count()%1000000000);
    }
    return sstr.str();
}

std::string CreateGuid(const std::string& sName)
{
    uuid_t guid;
    uuid_create_md5_from_name(&guid, NameSpace_OID, sName.c_str(), sName.length());


    std::array<char,40> output;
    snprintf(output.data(), output.size(), "%08x-%04hx-%04hx-%02x%02x-%02x%02x%02x%02x%02x%02x",
             guid.time_low, guid.time_mid, guid.time_hi_and_version, guid.clock_seq_hi_and_reserved, guid.clock_seq_low,
             guid.node[0], guid.node[1], guid.node[2], guid.node[3], guid.node[4], guid.node[5]);
    return std::string(output.data());
}


std::string CreateGuid()
{
    return CreateGuid(GetCurrentTaiTime(true));
}

bool CheckJsonType(const Json::Value& jsValue, const std::set<jsondatatype>& setType)
{
    switch(jsValue.type())
    {
        case Json::ValueType::arrayValue:
            if(setType.find(jsondatatype::_ARRAY) == setType.end())
            {
                return false;
            }
            break;
        case Json::ValueType::booleanValue:
            if(setType.find(jsondatatype::_BOOLEAN) == setType.end())
            {
                return false;
            }
            break;
        case Json::ValueType::intValue:
        case Json::ValueType::uintValue:
            if(setType.find(jsondatatype::_INTEGER) == setType.end())
            {
                return false;
            }
            break;
        case Json::ValueType::realValue:
            if(setType.find(jsondatatype::_NUMBER) == setType.end())
            {
                return false;
            }
            break;
        case Json::ValueType::nullValue:
            if(setType.find(jsondatatype::_NULL) == setType.end())
            {
                return false;
            }
            break;
        case Json::ValueType::objectValue:
            if(setType.find(jsondatatype::_OBJECT) == setType.end())
            {
                return false;
            }
            break;
        case Json::ValueType::stringValue:
            if(setType.find(jsondatatype::_STRING) == setType.end())
            {
                return false;
            }
            break;
    }
    return true;
}

void PatchJson(Json::Value& jsObject, const Json::Value& jsPatch)
{
    for(auto itObject = jsPatch.begin(); itObject != jsPatch.end(); ++itObject)
    {
        if(jsObject.isMember(itObject.key().asString()))
        {
            jsObject[itObject.key().asString()] = (*itObject);
        }
    }
}

bool CheckJson(const Json::Value& jsObject, const std::map<std::string, std::set<jsondatatype>>& mKeys)
{
    return CheckJsonAllowed(jsObject, mKeys) && CheckJsonRequired(jsObject, mKeys);
}

bool CheckJsonAllowed(const Json::Value& jsObject, const std::map<std::string, std::set<jsondatatype>>& mAllowed)
{
    for(auto itObject = jsObject.begin(); itObject != jsObject.end(); ++itObject)
    {
        auto itAllowed = mAllowed.find(itObject.key().asString()) ;
        if(itAllowed == mAllowed.end())
        {
            pmlLog(pml::LOG_DEBUG, "pml::nmos") << "NMOS: CheckJsonAllowed: " << itObject.key().asString() << " not allowed";
            return false;
        }
        else if(CheckJsonType(*itObject, itAllowed->second) == false)
        {   //key allowed but not if this type
            pmlLog(pml::LOG_DEBUG, "pml::nmos") << "NMOS: CheckJsonAllowed: " << itObject.key().asString() << " allowed but wrong type";
            return false;
        }
    }
    return true;
}


bool CheckJsonRequired(const Json::Value& jsObject, const std::map<std::string, std::set<jsondatatype>>& mRequired)
{
    for(auto required : mRequired)
    {
        if(jsObject.isMember(required.first) == false)
        {
            return false;
        }
        if(!CheckJsonType(jsObject[required.first], required.second))
        {
            return false;
        }
    }
    return true;
}

bool CheckJsonOptional(const Json::Value& jsObject, const std::map<std::string, std::set<jsondatatype>>& mOptional)
{
    //if the key exists then it must be of the given type
    for(auto opt : mOptional)
    {
        if(jsObject.isMember(opt.first))
        {
            if(!CheckJsonType(jsObject[opt.first], opt.second))
            {
                return false;
            }
        }
    }
    return true;
}

bool CheckJsonNotAllowed(const Json::Value& jsObject, const std::set<std::string>& setNotAllowed)
{
    for(auto sKey : setNotAllowed)
    {
        if(jsObject.isMember(sKey))
        {
            return false;
        }
    }
    return true;
}


bool JsonMemberExistsAndIsNull(const Json::Value& jsObject, const std::string& sMember)
{
    return (jsObject.isMember(sMember) && jsObject[sMember].isNull());
}

bool JsonMemberExistsAndIsNotNull(const Json::Value& jsObject, const std::string& sMember)
{
    return (jsObject.isMember(sMember) && (jsObject[sMember].isNull()==false));
}


Json::Value ConvertToJson(const std::string& str)
{
    Json::Value jsData;
    try
    {
        std::stringstream ss;
        ss.str(str);

        ss >> jsData;

    }
    catch(const Json::RuntimeError& e)
    {
        pmlLog(pml::LOG_ERROR, "pml::nmos") << "NMOS: " << "Unable to convert '" << str << "' to JSON: " << e.what();
    }

    return jsData;
}

std::string ConvertFromJson(const Json::Value& jsValue)
{
    Json::StreamWriterBuilder builder;
    builder["commentStyle"] = "None";
    builder["indentation"] = "";
    return Json::writeString(builder, jsValue);
}

std::optional<bool> GetBool(const Json::Value& jsObject, const std::string& sKey)
{
    if(jsObject.isMember(sKey) && jsObject[sKey].isConvertibleTo(Json::booleanValue))
    {
        return jsObject[sKey].asBool();
    }
    return {};
}

std::optional<std::string> GetString(const Json::Value& jsObject, const std::string& sKey)
{
    if(jsObject.isMember(sKey) && jsObject[sKey].isConvertibleTo(Json::stringValue))
    {
        return jsObject[sKey].asString();
    }
    return {};
}

std::optional<uint32_t> GetUInt(const Json::Value& jsObject, const std::string& sKey)
{
    if(jsObject.isMember(sKey) && jsObject[sKey].isConvertibleTo(Json::uintValue))
    {
        return jsObject[sKey].asUInt();
    }
    return {};
}

std::optional<int32_t> GetInt(const Json::Value& jsObject, const std::string& sKey)
{
    if(jsObject.isMember(sKey) && jsObject[sKey].isConvertibleTo(Json::intValue))
    {
        return jsObject[sKey].asInt();
    }
    return {};
}
std::optional<uint64_t> GetUInt64(const Json::Value& jsObject, const std::string& sKey)
{
    if(jsObject.isMember(sKey) && jsObject[sKey].isConvertibleTo(Json::uintValue))
    {
        return jsObject[sKey].asUInt64();
    }
    return {};
}

std::optional<int64_t> GetInt64(const Json::Value& jsObject, const std::string& sKey)
{
    if(jsObject.isMember(sKey) && jsObject[sKey].isConvertibleTo(Json::intValue))
    {
        return jsObject[sKey].asInt64();
    }
    return {};
}

std::optional<double> GetDouble(const Json::Value& jsObject, const std::string& sKey)
{
    if(jsObject.isMember(sKey) && jsObject[sKey].isConvertibleTo(Json::realValue))
    {
        return jsObject[sKey].asDouble();
    }
    return {};
}



std::optional<std::chrono::time_point<std::chrono::high_resolution_clock>> ConvertTaiStringToTimePoint(const std::string& sTai)
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
            return std::chrono::time_point<std::chrono::high_resolution_clock>(nano);
        }
        catch(std::invalid_argument& e)
        {
            return {};
        }
    }
    return {};
}

bool AddTaiStringToTimePoint(const std::string& sTai,std::chrono::time_point<std::chrono::high_resolution_clock>& tp)
{
    try
    {
        std::vector<std::string> vTime;
        SplitString(vTime, sTai, ':');
        if(vTime.size() != 2)
        {
            throw std::invalid_argument("invalid time");
        }
        std::chrono::nanoseconds nano((static_cast<long long int>(std::stoul(vTime[0]))*1000000000)+stoul(vTime[1]));
        tp+=nano;
        tp-=LEAP_SECONDS;   //remove the leap seconds to get back to the system time?>?>
        return true;
    }
    catch(std::invalid_argument& e)
    {
        return false;
    }
}


