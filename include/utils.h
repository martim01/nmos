#pragma once
#include <string>
#include <vector>
#include "json/json.h"
#include <chrono>
#include <set>
#include <map>
#include <list>
#include "optional.hpp"
enum class jsondatatype {_STRING, _INTEGER, _NUMBER, _OBJECT, _ARRAY, _BOOLEAN, _NULL};

static const std::chrono::seconds LEAP_SECONDS(37);

extern std::string ConvertTimeToString(std::chrono::time_point<std::chrono::high_resolution_clock> tp, bool bIncludeNano=true);
extern std::string GetCurrentTaiTime(bool bIncludeNano=true);
extern std::chrono::time_point<std::chrono::high_resolution_clock> GetTaiTimeNow();
extern void SplitString(std::vector<std::string>& vSplit, const std::string& str, char cSplit);
extern std::vector<std::string> SplitString(const std::string& str, char cSplit);
extern std::string GetIpAddress(const std::string& sInterface);
extern std::string CreateGuid(const std::string& sName);
extern std::string CreateGuid();
extern size_t GetCurrentHeartbeatTime();
extern std::experimental::optional<std::chrono::time_point<std::chrono::high_resolution_clock>> ConvertTaiStringToTimePoint(const std::string& sTai);
extern Json::Value ConvertToJson(const std::string& str);
extern std::string ConvertFromJson(const Json::Value& jsValue);

extern bool AddTaiStringToTimePoint(const std::string& sTai, std::chrono::time_point<std::chrono::high_resolution_clock>& tp);




extern void PatchJson(Json::Value& jsObject, const Json::Value& jsPatch);

extern bool CheckJson(const Json::Value& jsObject, const std::map<std::string, std::set<jsondatatype>>& mKeys);
extern bool CheckJsonNotAllowed(const Json::Value& jsObject, const std::set<std::string>& setNotAllowed);
extern bool CheckJsonAllowed(const Json::Value& jsObject, const std::map<std::string, std::set<jsondatatype>>& mAllowed);
extern bool CheckJsonOptional(const Json::Value& jsObject, const std::map<std::string, std::set<jsondatatype>>& mAllowed);
extern bool CheckJsonRequired(const Json::Value& jsObject, const std::map<std::string, std::set<jsondatatype>>&mRequired);

extern bool JsonMemberExistsAndIsNull(const Json::Value& jsObject, const std::string& sMember);
extern bool JsonMemberExistsAndIsNotNull(const Json::Value& jsObject, const std::string& sMember);


extern std::experimental::optional<bool> GetBool(const Json::Value& jsObject, const std::string& sKey);
extern std::experimental::optional<std::string> GetString(const Json::Value& jsObject, const std::string& sKey);
extern std::experimental::optional<uint32_t> GetUInt(const Json::Value& jsObject, const std::string& sKey) ;
extern std::experimental::optional<int32_t> GetInt(const Json::Value& jsObject, const std::string& sKey) ;
extern std::experimental::optional<double> GetDouble(const Json::Value& jsObject, const std::string& sKey) ;
extern std::experimental::optional<int64_t> GetInt64(const Json::Value& jsObject, const std::string& sKey) ;
extern std::experimental::optional<uint64_t> GetUInt64(const Json::Value& jsObject, const std::string& sKey) ;
