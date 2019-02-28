#pragma once
#include <string>
#include <vector>

extern std::string GetCurrentTime(bool bIncludeNano=true);
extern void SplitString(std::vector<std::string>& vSplit, std::string str, char cSplit);
extern std::string GetIpAddress(const std::string& sInterface);
extern std::string CreateGuid(std::string sName);
extern std::string CreateGuid();

