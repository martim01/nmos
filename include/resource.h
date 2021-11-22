#pragma once
#ifdef __GNUWIN32__
#include <windows.h>
#include <rpc.h>
#endif
#include <string>
#include "json/json.h"
#include <list>
#include <map>
#include "nmosdlldefine.h"
#include <chrono>
#include <mutex>
#include <sstream>
#include "nmosapiversion.h"

namespace pml
{
    namespace nmos
    {
        class Resource
        {
            public:
                Resource(const std::string& sType, const std::string& sLabel, const std::string& sDescription);
                Resource(const std::string& sType);


                virtual ~Resource(){}
                void AddTag(const std::string& sKey, const std::string& sValue);

                const std::string& GetId() const;
                const std::string& GetLabel() const;
                const std::string& GetDescription() const;
                const std::string& GetVersion() const;

                bool IsOk() const;

                virtual bool UpdateFromJson(const Json::Value& jsValue);

                const Json::Value& GetJson(const ApiVersion& version) const;
                virtual bool Commit(const ApiVersion& version);

                void UpdateLabel(const std::string& sLabel);
                void UpdateDescription(const std::string& sDescription);

                const std::string& GetType() const;

                virtual std::string GetParentResourceId() const
                {
                    return "";
                }
                size_t GetLastHeartbeat() const;
                void SetHeartbeat();

                std::string GetJsonParseError();

                static std::string s_sBase;

            protected:
                void UpdateVersionTime();




                Json::Value m_json;
                bool m_bIsOk;

                std::stringstream m_ssJsonError;

                mutable std::mutex m_mutex;
            private:

                std::string m_sType;
                std::string m_sId;
                std::string m_sLabel;
                std::string m_sDescription;
                std::string m_sVersion;
                std::string m_sLastVersion;
                std::multimap<std::string, std::string> m_mmTag;


                size_t m_nHeartbeat;
        };
    };
};
