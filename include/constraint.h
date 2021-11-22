#pragma once
#include <string>
#include "json/json.h"
#include <vector>
#include "nmosapiversion.h"
#include "utils.h"

namespace pml
{
    namespace nmos
    {
        class constraint
        {
            public:

                typedef std::pair<jsondatatype, std::string> pairEnum_t;

                constraint(const std::string& sDescription="");

                bool MeetsConstraint(const std::string& sValue);
                bool MeetsConstraint(int nValue);
                bool MeetsConstraint(double dValue);
                bool MeetsConstraint(bool bValue);

                void SetMinimum(int nMinimum);
                void SetMaximum(int nMaximum);
                void SetEnum(const std::vector<pairEnum_t>& vConstraints);
                void SetPattern(const std::string& sPattern);

                void RemoveMinimum();
                void RemoveMaximum();
                void RemoveEnum();
                void RemovePattern();


                Json::Value GetJson(const ApiVersion& version) const;

                bool UpdateFromJson(const Json::Value& jsConstraint);

            private:

                std::string m_sDescription;
                std::pair<bool, int> m_pairMinimum;     // @todo min constraint can be a double as well
                std::pair<bool, int> m_pairMaximum;     // @todo max constraint can be a double as well
                std::vector<pairEnum_t> m_vEnum;
                std::pair<bool, std::string> m_pairPattern;


        };


        class Constraints
        {
            public:
                Constraints(int nSupported);

                virtual Json::Value GetJson(const ApiVersion& version) const;

                bool UpdateFromJson(const Json::Value& jsData);

                bool MeetsConstraint(const std::string& sConstraint, const std::string& sValue);
                bool MeetsConstraint(const std::string& sConstraint, int nValue);
                bool MeetsConstraint(const std::string& sConstraint, double dValue);
                bool MeetsConstraint(const std::string& sConstraint, bool bValue);


            protected:
                int m_nParamsSupported;


            std::map<std::string, constraint> m_mConstraints;


        };

        class ConstraintsSender : public Constraints
        {
            public:
                ConstraintsSender(int nSupported);


        };


        struct ConstraintsReceiver : public Constraints
        {
            ConstraintsReceiver(int nSupported);
        };

        std::vector<Constraints> CreateConstraints(const Json::Value& jsData);
    };
};
