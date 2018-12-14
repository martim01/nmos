#pragma once
#include <string>
#include "json/json.h"
#include <list>


class Resource
{
    public:
        Resource(std::string sLabel, std::string sDescription);
        Resource(){}
        void AddTag(std::string sTag);

        std::string GetId() const
        {   return m_sId;   }

        const Json::Value& GetJson() const;
        virtual bool Commit();

        void UpdateLabel(std::string sLabel);
        void UpdateDescription(std::string sDescription);

    protected:
        void UpdateVersionTime();
        Json::Value m_json;

    private:
        void CreateGuid();
        std::string m_sId;
        std::string m_sLabel;
        std::string m_sDescription;
        std::string m_sVersion;
        std::string m_sLastVersion;
        std::list<std::string> m_lstTag;
};
