#pragma once
#include <string>
#include <vector>
#include "json/json.h"
#include <chrono>
#include <set>
#include <map>
#include <list>
#include <optional>

namespace pml
{
    namespace nmos
    {
        enum class jsondatatype {_STRING, _INTEGER, _NUMBER, _OBJECT, _ARRAY, _BOOLEAN, _NULL};

        static const std::chrono::seconds LEAP_SECONDS(37);

        std::string ConvertTimeToString(std::chrono::time_point<std::chrono::high_resolution_clock> tp, bool bIncludeNano=true);
        std::string GetCurrentTaiTime(bool bIncludeNano=true);
        std::chrono::time_point<std::chrono::high_resolution_clock> GetTaiTimeNow();
        void SplitString(std::vector<std::string>& vSplit, const std::string& str, char cSplit);
        std::vector<std::string> SplitString(const std::string& str, char cSplit);
        std::string GetIpAddress(const std::string& sInterface);
        std::string CreateGuid(const std::string& sName);
        std::string CreateGuid();
        size_t GetCurrentHeartbeatTime();
        std::optional<std::chrono::time_point<std::chrono::high_resolution_clock>> ConvertTaiStringToTimePoint(const std::string& sTai);
        Json::Value ConvertToJson(const std::string& str);
        std::string ConvertFromJson(const Json::Value& jsValue);

        bool AddTaiStringToTimePoint(const std::string& sTai, std::chrono::time_point<std::chrono::high_resolution_clock>& tp);




        void PatchJson(Json::Value& jsObject, const Json::Value& jsPatch);

        bool CheckJson(const Json::Value& jsObject, const std::map<std::string, std::set<jsondatatype>>& mKeys);
        bool CheckJsonNotAllowed(const Json::Value& jsObject, const std::set<std::string>& setNotAllowed);
        bool CheckJsonAllowed(const Json::Value& jsObject, const std::map<std::string, std::set<jsondatatype>>& mAllowed);
        bool CheckJsonOptional(const Json::Value& jsObject, const std::map<std::string, std::set<jsondatatype>>& mAllowed);
        bool CheckJsonRequired(const Json::Value& jsObject, const std::map<std::string, std::set<jsondatatype>>&mRequired);

        bool JsonMemberExistsAndIsNull(const Json::Value& jsObject, const std::string& sMember);
        bool JsonMemberExistsAndIsNotNull(const Json::Value& jsObject, const std::string& sMember);


        std::optional<bool> GetBool(const Json::Value& jsObject, const std::string& sKey);
        std::optional<std::string> GetString(const Json::Value& jsObject, const std::string& sKey);
        std::optional<uint32_t> GetUInt(const Json::Value& jsObject, const std::string& sKey) ;
        std::optional<int32_t> GetInt(const Json::Value& jsObject, const std::string& sKey) ;
        std::optional<double> GetDouble(const Json::Value& jsObject, const std::string& sKey) ;
        std::optional<int64_t> GetInt64(const Json::Value& jsObject, const std::string& sKey) ;
        std::optional<uint64_t> GetUInt64(const Json::Value& jsObject, const std::string& sKey) ;
    }
}