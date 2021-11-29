#pragma once
#include <string>
#include "json/json.h"
#include <vector>
#include "nmosapiversion.h"
#include "utils.h"
#include "optional.hpp"

namespace pml
{
    namespace nmos
    {
        typedef std::pair<jsondatatype, std::string> pairEnum_t;

        class constraint
        {
            public:


                constraint(const std::string& sDescription="");

                bool MeetsConstraint(const std::string& value) const;
                bool MeetsConstraint(const unsigned int value) const;
                bool MeetsConstraint(const int value) const;
                bool MeetsConstraint(const double value) const;
                bool MeetsConstraint(const bool value) const;

                void SetMinimum(int nMinimum);
                void SetMaximum(int nMaximum);
                void SetEnum(const std::vector<pairEnum_t>& vConstraints);
                void SetPattern(const std::string& sPattern);

                void RemoveMinimum();
                void RemoveMaximum();
                void RemoveEnum();
                void RemovePattern();


                Json::Value GetJson() const;

                bool UpdateFromJson(const Json::Value& jsConstraint);

                static const std::string MAXIMUM;
                static const std::string MINIMUM;
                static const std::string PATTERN;
                static const std::string ENUM;

            private:

                std::string m_sDescription;
                std::experimental::optional<int> m_minimum; // @todo min constraint can be a double as well
                std::experimental::optional<int> m_maximum; // @todo min constraint can be a double as well
                std::experimental::optional<std::string> m_pattern;
                std::vector<pairEnum_t> m_vEnum;


        };


        class Constraints
        {
            public:
                Constraints();
                Constraints(const Json::Value& jsTransport);

                virtual Json::Value GetJson() const;

                bool UpdateFromJson(const Json::Value& jsData);

                bool MeetsConstraint(const std::string& sKey, const Json::Value& jsCheck) const;

                bool AddConstraint(const std::string& sKey, const std::experimental::optional<int>& minValue, const std::experimental::optional<int>& maxValue, const std::experimental::optional<std::string>& pattern,
                                   const std::vector<pairEnum_t>& vEnum);
                bool ClearConstraint(const std::string& sKey);

                void CreateEmptyConstraint(const std::string& sKey);

            protected:
                std::map<std::string, constraint> m_mConstraints;


        };


    };
};
