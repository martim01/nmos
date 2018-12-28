#include "constraint.h"
#include <sstream>

using namespace std;

constraint::constraint(const std::string& sParam, const std::string& sDescription) :
    m_sParam(sParam),
    m_pairMinimum(make_pair(false, 0)),
    m_pairMaximum(make_pair(false, 0)),
    m_pairPattern(make_pair(false, "")),
    m_sDescription(sDescription)
{

}


void constraint::SetMinimum(int nMinimum)
{
    m_pairMinimum = make_pair(true, nMinimum);
}

void constraint::SetMaximum(int nMaximum)
{
    m_pairMaximum = make_pair(true, nMaximum);
}

void constraint::SetEnum(const std::vector<constraint::pairEnum_t>& vConstraints)
{
    m_vEnum = vConstraints;
}

void constraint::SetPattern(const std::string& sPattern)
{
    m_pairPattern = make_pair(true, sPattern);
}


void constraint::RemoveMinimum()
{
    m_pairMinimum = make_pair(false,0);
}

void constraint::RemoveMaximum()
{
    m_pairMaximum = make_pair(false,0);
}

void constraint::RemoveEnum()
{
    m_vEnum.clear();
}

void constraint::RemovePattern()
{
    m_pairPattern = make_pair(false, "");
}

Json::Value constraint::GetJson() const
{
    Json::Value jsConstraint(Json::objectValue);
    if(m_pairMinimum.first)
    {
        jsConstraint["minimum"] = m_pairMinimum.second;
    }
    if(m_pairMaximum.first)
    {
        jsConstraint["maximum"] = m_pairMaximum.second;
    }
    if(m_pairPattern.first)
    {
        jsConstraint["pattern"] = m_pairPattern.second;
    }
    if(m_vEnum.empty() == false)
    {
        jsConstraint["enum"] = Json::arrayValue;
        for(size_t i = 0; i < m_vEnum.size(); i++)
        {
            switch(m_vEnum[i].first)
            {
                case CON_BOOL:
                    if(m_vEnum[i].second == "true")
                    {
                        jsConstraint["enum"].append(true);
                    }
                    else
                    {
                        jsConstraint["enum"].append(false);
                    }
                    break;
                case CON_INTEGER:
                    jsConstraint["enum"] = stoi(m_vEnum[i].second);
                    break;
                case CON_NULL:
                    jsConstraint["enum"] = Json::nullValue;
                    break;
                case CON_STRING:
                    jsConstraint["enum"] = m_vEnum[i].second;
                    break;
                case CON_NUMBER:
                    jsConstraint["enum"] = stod(m_vEnum[i].second);
                    break;
            }
        }
    }
    return jsConstraint;
}


bool constraint::MeetsConstraint(const std::string& sValue)
{
    bool bMeets = true;
    //is the min set
    if(m_pairMinimum.first)
    {
        int nMin;
        try
        {
            nMin = stoi(sValue);
            bMeets &= (nMin >= m_pairMinimum.second);
        }
        catch(const invalid_argument& ia)
        {
            bMeets = false;
        }
    }
    if(m_pairMaximum.first)
    {
        int nMax;
        try
        {
            nMax = stoi(sValue);
            bMeets &= (nMax <= m_pairMaximum.second);
        }
        catch(const invalid_argument& ia)
        {
            bMeets = false;
        }
    }

    if(m_vEnum.empty() == false)
    {
        for(size_t i = 0; i < m_vEnum.size(); i++)
        {
            bMeets &= (m_vEnum[i].second == sValue);
        }
    }

    if(m_pairPattern.first)
    {
        // @todo constraint pattern checking
    }
    return bMeets;
}

bool constraint::MeetsConstraint(int nValue)
{
    bool bMeets = true;
    //is the min set
    if(m_pairMinimum.first)
    {
        bMeets &= (nValue >= m_pairMinimum.second);
    }
    if(m_pairMaximum.first)
    {
        bMeets &= (nValue <= m_pairMaximum.second);
    }

    if(m_vEnum.empty() == false)
    {
        stringstream ss;
        ss << nValue;

        for(size_t i = 0; i < m_vEnum.size(); i++)
        {
            bMeets &= (m_vEnum[i].second == ss.str());
        }
    }

    if(m_pairPattern.first)
    {
        // @todo constraint pattern checking
    }
    return bMeets;
}

bool constraint::MeetsConstraint(double dValue)
{
    bool bMeets = true;
    //is the min set
    if(m_pairMinimum.first)
    {
        bMeets &= (dValue >= m_pairMinimum.second);
    }
    if(m_pairMaximum.first)
    {
        bMeets &= (dValue <= m_pairMaximum.second);
    }

    if(m_vEnum.empty() == false)
    {
        stringstream ss;
        ss << dValue;

        for(size_t i = 0; i < m_vEnum.size(); i++)
        {
            bMeets &= (m_vEnum[i].second == ss.str());
        }
    }

    if(m_pairPattern.first)
    {
        // @todo constraint pattern checking
    }
    return bMeets;
}

bool constraint::MeetsConstraint(bool bValue)
{
    bool bMeets = true;
    //is the min set
    if(m_pairMinimum.first)
    {
        bMeets &= (bValue >= m_pairMinimum.second);
    }
    if(m_pairMaximum.first)
    {
        bMeets &= (bValue <= m_pairMaximum.second);
    }

    if(m_vEnum.empty() == false)
    {
        string sValue("false");
        if(bValue)
        {
            sValue = "true";
        }

        for(size_t i = 0; i < m_vEnum.size(); i++)
        {
            bMeets &= (m_vEnum[i].second == sValue);
        }
    }

    if(m_pairPattern.first)
    {
        // @todo constraint pattern checking
    }
    return bMeets;
}




constraints::constraints() :
    destination_port("destination_port"),
    fec_destination_ip("fec_destination_ip"),
    fec_enabled("fec_enabled"),
    fec_mode("fec_mode"),
    fec1D_destination_port("fec1D_destination_port"),
    fec2D_destination_port("fec2D_destination_port"),
    rtcp_destination_ip("rtcp_destination_ip"),
    rtcp_destination_port("rtcp_destination_port")
{

}

Json::Value constraints::GetJson() const
{
    Json::Value jsConstraints(Json::objectValue);
    jsConstraints[destination_port.GetParam()] = destination_port.GetJson();
    jsConstraints[fec_destination_ip.GetParam()] = fec_destination_ip.GetJson();
    jsConstraints[fec_enabled.GetParam()] = fec_enabled.GetJson();
    jsConstraints[fec_mode.GetParam()] = fec_mode.GetJson();
    jsConstraints[fec1D_destination_port.GetParam()] = fec1D_destination_port.GetJson();
    jsConstraints[fec2D_destination_port.GetParam()] = fec2D_destination_port.GetJson();
    jsConstraints[rtcp_destination_ip.GetParam()] = rtcp_destination_ip.GetJson();
    jsConstraints[rtcp_destination_port.GetParam()] = rtcp_destination_port.GetJson();
    return jsConstraints;
}

constraintsSender::constraintsSender() : constraints(),
    destination_ip("destination_ip"),
    source_ip("source_ip"),
    source_port("source_port"),
    fec_type("fec_type"),
    fec_block_width("fec_block_width"),
    fec_block_height("fec_block_height"),
    fec1D_source_port("fec1D_source_port"),
    fec2D_source_port("fec2D_source_port"),
    rtcp_enabled("rtcp_enabled"),
    rtcp_source_port("rtcp_source_port"),
    rtp_enabled("rtp_enabled")
{

}

Json::Value constraintsSender::GetJson() const
{
    Json::Value jsConstraints(constraints::GetJson());
    jsConstraints[destination_ip.GetParam()] = destination_ip.GetJson();
    jsConstraints[source_ip.GetParam()] = source_ip.GetJson();
    jsConstraints[source_port.GetParam()] = source_port.GetJson();
    jsConstraints[fec_type.GetParam()] = fec_type.GetJson();
    jsConstraints[fec_block_width.GetParam()] = fec_block_width.GetJson();
    jsConstraints[fec_block_height.GetParam()] = fec_block_height.GetJson();
    jsConstraints[fec1D_source_port.GetParam()] = fec1D_source_port.GetJson();
    jsConstraints[fec2D_source_port.GetParam()] = fec2D_source_port.GetJson();
    jsConstraints[rtcp_enabled.GetParam()] = rtcp_enabled.GetJson();
    jsConstraints[rtcp_source_port.GetParam()] = rtcp_source_port.GetJson();
    jsConstraints[rtp_enabled.GetParam()] = rtp_enabled.GetJson();
    return jsConstraints;
}

constraintsReceiver::constraintsReceiver() : constraints(),
    interface_ip("interface_ip")
{

}

Json::Value constraintsReceiver::GetJson() const
{
    Json::Value jsConstraints(constraints::GetJson());
    jsConstraints[interface_ip.GetParam()] = interface_ip.GetJson();
    return jsConstraints;
}
