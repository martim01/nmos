#pragma once
#include <string>
#include <vector>
#include "json/json.h"
#include <chrono>
#include <set>


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
extern bool CheckJson(const Json::Value& jsObject, std::set<std::string> setAllowed);
extern bool CheckJsonRequired(const Json::Value& jsObject, std::set<std::string> setRequired);
extern bool JsonMemberExistsAndIsNull(const Json::Value& jsObject, const std::string& sMember);
extern bool JsonMemberExistsAndIsNotNull(const Json::Value& jsObject, const std::string& sMember);
extern bool ConvertTaiStringToTimePoint(const std::string& sTai,  std::chrono::time_point<std::chrono::high_resolution_clock>& tp);
extern Json::Value ConvertToJson(const std::string& str);
extern std::string ConvertFromJson(const Json::Value& jsValue);
//template<typename T, typename... Args> std::unique_ptr<T> make_unique(Args&&... args){return std::unique_ptr<T>(new T(std::forward<Args>(args)...));}
