#pragma once
#include <string>
#include "json/json.h"
#include <list>
#include "dlldefine.h"
#include <chrono>
#include <mutex>

class NMOS_EXPOSE Resource
{
    public:
        Resource(const std::string& sType, const std::string& sLabel, const std::string& sDescription);
        Resource(const std::string& sType);


        virtual ~Resource(){}
        void AddTag(std::string sTag);

        const std::string& GetId() const;
        const std::string& GetLabel() const;
        const std::string& GetDescription() const;
        const std::string& GetVersion() const;

        bool IsOk() const;

        virtual bool UpdateFromJson(const Json::Value& jsValue);

        const Json::Value& GetJson() const;
        virtual bool Commit();

        void UpdateLabel(std::string sLabel);
        void UpdateDescription(std::string sDescription);

        const std::string& GetType() const;

        size_t GetLastHeartbeat() const;
        void SetHeartbeat();

    protected:
        void UpdateVersionTime();

        std::string GetCurrentTime(bool bIncludeNano=true);
        bool ConvertTaiStringToTimePoint(const std::string& sTai,  std::chrono::time_point<std::chrono::high_resolution_clock>& tp);

        Json::Value m_json;
        bool m_bIsOk;

        mutable std::mutex m_mutex;
    private:
        void CreateGuid();
        std::string m_sType;
        std::string m_sId;
        std::string m_sLabel;
        std::string m_sDescription;
        std::string m_sVersion;
        std::string m_sLastVersion;
        std::list<std::string> m_lstTag;


        size_t m_nHeartbeat;
};
