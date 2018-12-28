#pragma once
#include <string>
#include "json/json.h"
#include <list>
#include "dlldefine.h"

class NMOS_EXPOSE Resource
{
    public:
        Resource(std::string sLabel, std::string sDescription);
        Resource();
        Resource(const Json::Value& jsValue);

        virtual ~Resource(){}
        void AddTag(std::string sTag);

        const std::string& GetId() const;
        const std::string& GetLabel() const;
        const std::string& GetDescription() const;
        const std::string& GetVersion() const;

        bool IsOk() const;


        const Json::Value& GetJson() const;
        virtual bool Commit();

        void UpdateLabel(std::string sLabel);
        void UpdateDescription(std::string sDescription);

    protected:
        void UpdateVersionTime();

        std::string GetCurrentTime();

        Json::Value m_json;
        bool m_bIsOk;


    private:
        void CreateGuid();
        std::string m_sId;
        std::string m_sLabel;
        std::string m_sDescription;
        std::string m_sVersion;
        std::string m_sLastVersion;
        std::list<std::string> m_lstTag;


};
