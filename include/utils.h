#pragma once
#include <string>
#include <vector>

extern void SplitString(std::vector<std::string>& vSplit, std::string str, char cSplit);
extern std::string GetIpAddress(const std::string& sInterface);
extern std::string CreateGuid();
