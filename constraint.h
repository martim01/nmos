#pragma once
#include <string>
#include "json/json.h"
#include <vector>


class constraint
{
    public:

        enum enumConstraint {CON_BOOL, CON_INTEGER, CON_NULL, CON_NUMBER, CON_STRING};
        typedef std::pair<enumConstraint, std::string> pairEnum_t;

        constraint(const std::string& sParam, const std::string& sDescription="");

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

        std::string GetParam() const
        {
            return m_sParam;
        }

        Json::Value GetJson() const;

    private:
        std::string m_sParam;

        std::pair<bool, int> m_pairMinimum;     // @todo min constraint can be a double as well
        std::pair<bool, int> m_pairMaximum;     // @todo max constraint can be a double as well
        std::vector<pairEnum_t> m_vEnum;
        std::pair<bool, std::string> m_pairPattern;

        std::string m_sDescription;
};


struct constraints
{
    constraints();

    virtual Json::Value GetJson() const;


    constraint destination_port;
    constraint fec_destination_ip;
    constraint fec_enabled;
    constraint fec_mode;
    constraint fec1D_destination_port;
    constraint fec2D_destination_port;
    constraint rtcp_destination_ip;
    constraint rtcp_destination_port;
};

struct constraintsSender : public constraints
{
    constraintsSender();
    virtual Json::Value GetJson() const;


    constraint destination_ip;
    constraint source_ip;
    constraint source_port;
    constraint fec_type;
    constraint fec_block_width;
    constraint fec_block_height;
    constraint fec1D_source_port;
    constraint fec2D_source_port;
    constraint rtcp_enabled;
    constraint rtcp_source_port;
    constraint rtp_enabled;
};


struct constraintsReceiver : public constraints
{
    constraintsReceiver();

    virtual Json::Value GetJson() const;

    constraint interface_ip;
};
