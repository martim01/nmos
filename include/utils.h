#pragma once
#include <string>
#include <vector>
#include "json/json.h"
#include <chrono>

static const std::chrono::seconds LEAP_SECONDS(37);

extern std::string ConvertTimeToString(std::chrono::time_point<std::chrono::high_resolution_clock> tp, bool bIncludeNano=true);
extern std::string GetCurrentTaiTime(bool bIncludeNano=true);
extern void SplitString(std::vector<std::string>& vSplit, const std::string& str, char cSplit);
extern std::string GetIpAddress(const std::string& sInterface);
extern std::string CreateGuid(const std::string& sName);
extern std::string CreateGuid();
extern size_t GetCurrentHeartbeatTime();
extern bool CheckJson(const Json::Value& jsObject, std::initializer_list<std::string> lstAllowed);
extern bool JsonMemberExistsAndIsNull(const Json::Value& jsObject, const std::string& sMember);
extern bool JsonMemberExistsAndIsNotNull(const Json::Value& jsObject, const std::string& sMember);

