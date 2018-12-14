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

        virtual Json::Value ToJson() const;

        void UpdateLabel(std::string sLabel);
        void UpdateDescription(std::string sDescription);

    private:
        void CreateGuid();
        void UpdateVersionTime();

        std::string m_sId;
        std::string m_sLabel;
        std::string m_sDescription;
        std::string m_sVersion;
        std::list<std::string> m_lstTag;
};
