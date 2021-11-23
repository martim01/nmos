#include "constraint.h"
#include <sstream>
#include "transportparams.h"

using namespace std;
using namespace pml::nmos;

constraint::constraint(const std::string& sDescription) :
    m_sDescription(sDescription),
    m_pairMinimum(make_pair(false, 0)),
    m_pairMaximum(make_pair(false, 0)),
    m_pairPattern(make_pair(false, ""))

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

Json::Value constraint::GetJson(const ApiVersion& version) const
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
                case jsondatatype::_BOOLEAN:
                    if(m_vEnum[i].second == "true")
                    {
                        jsConstraint["enum"].append(true);
                    }
                    else
                    {
                        jsConstraint["enum"].append(false);
                    }
                    break;
                case jsondatatype::_INTEGER:
                    jsConstraint["enum"] = stoi(m_vEnum[i].second);
                    break;
                case jsondatatype::_NULL:
                    jsConstraint["enum"] = Json::nullValue;
                    break;
                case jsondatatype::_STRING:
                    jsConstraint["enum"] = m_vEnum[i].second;
                    break;
                case jsondatatype::_NUMBER:
                    jsConstraint["enum"] = stod(m_vEnum[i].second);
                    break;
                default:
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

bool constraint::UpdateFromJson(const Json::Value& jsConstraint)
{
    if(CheckJsonAllowed(jsConstraint, { {"maximum", {jsondatatype::_NUMBER, jsondatatype::_INTEGER}},
                          {"minumum", {jsondatatype::_NUMBER, jsondatatype::_INTEGER}},
                          {"enum", {jsondatatype::_ARRAY}},
                          {"pattern", {jsondatatype::_STRING}}}) == false)
    {
        return false;
    }

    if(jsConstraint.isMember("maximum"))
    {
        m_pairMaximum = {true, jsConstraint.asInt()};   //@todo this could be a double
    }
    if(jsConstraint.isMember("minumum"))
    {
        m_pairMinimum = {true, jsConstraint.asInt()};   //@todo this could be a double
    }
    if(jsConstraint.isMember("enum"))
    {
        m_vEnum.clear();
        m_vEnum.resize(jsConstraint["enum"].size());
        for(Json::ArrayIndex ai = 0; ai < jsConstraint["enum"].size(); ai++)
        {
            switch(jsConstraint["enum"][ai].type())
            {
                case Json::ValueType::booleanValue:
                    m_vEnum[ai] = {jsondatatype::_BOOLEAN, jsConstraint["enum"][ai].asBool() ? "true" : "false"};
                    break;
                case Json::ValueType::intValue:
                case Json::ValueType::uintValue:
                    m_vEnum[ai] = {jsondatatype::_INTEGER, jsConstraint["enum"][ai].asString()};
                    break;
                case Json::ValueType::realValue:
                    m_vEnum[ai] = {jsondatatype::_NUMBER, jsConstraint["enum"][ai].asString()};
                    break;
                case Json::ValueType::nullValue:
                    m_vEnum[ai] = {jsondatatype::_NULL, "null"};
                    break;
                case Json::ValueType::stringValue:
                    m_vEnum[ai] = {jsondatatype::_STRING, jsConstraint["enum"][ai].asString()};
                    break;
                default:
                    return false;
            }

        }
    }
    if(jsConstraint.isMember("pattern"))
    {
        m_pairPattern = {true, jsConstraint.asString()};   //@todo this could be a double
    }
    return true;
}



Constraints::Constraints(int nSupported) :
    m_nParamsSupported(nSupported),
    m_mConstraints({ {"source_ip", constraint()},
                   {"destination_port",constraint()},
                   {"rtp_enabled",constraint()}})
{
    if(m_nParamsSupported & TransportParamsRTP::FEC)
    {
        m_mConstraints.insert({"fec_enabled",constraint()});
        m_mConstraints.insert({"fec_destination_ip",constraint()});
        m_mConstraints.insert({"fec_mode",constraint()});
        m_mConstraints.insert({"fec1D_destination_port",constraint()});
        m_mConstraints.insert({"fec2D_destination_port",constraint()});
    }
    if(m_nParamsSupported & TransportParamsRTP::RTCP)
    {
        m_mConstraints.insert({"rtcp_enabled",constraint()});
        m_mConstraints.insert({"rtcp_destination_ip",constraint()});
        m_mConstraints.insert({"rtcp_destination_port",constraint()});
    }
}

Json::Value Constraints::GetJson(const ApiVersion& version) const
{
    Json::Value jsConstraints(Json::objectValue);

    for(const auto& pairConstraint : m_mConstraints)
    {
        jsConstraints[pairConstraint.first] = pairConstraint.second.GetJson(version);
    }
    return jsConstraints;
}


bool Constraints::UpdateFromJson(const Json::Value& jsData)
{
    for(auto itObject = jsData.begin(); itObject != jsData.end(); ++itObject)
    {
        m_mConstraints[itObject.key().asString()].UpdateFromJson(*itObject);
    }
    return true;
}

bool Constraints::MeetsConstraint(const std::string& sConstraint, const std::string& sValue)
{
    auto itConstraint = m_mConstraints.find(sConstraint);
    if(itConstraint != m_mConstraints.end())
    {
        return itConstraint->second.MeetsConstraint(sValue);
    }
    return true;
}

bool Constraints::MeetsConstraint(const std::string& sConstraint, int nValue)
{
    auto itConstraint = m_mConstraints.find(sConstraint);
    if(itConstraint != m_mConstraints.end())
    {
        return itConstraint->second.MeetsConstraint(nValue);
    }
    return true;
}

bool Constraints::MeetsConstraint(const std::string& sConstraint, double dValue)
{
    auto itConstraint = m_mConstraints.find(sConstraint);
    if(itConstraint != m_mConstraints.end())
    {
        return itConstraint->second.MeetsConstraint(dValue);
    }
    return true;
}

bool Constraints::MeetsConstraint(const std::string& sConstraint, bool bValue)
{
    auto itConstraint = m_mConstraints.find(sConstraint);
    if(itConstraint != m_mConstraints.end())
    {
        return itConstraint->second.MeetsConstraint(bValue);
    }
    return true;
}

ConstraintsSender::ConstraintsSender(int nSupported) : Constraints(nSupported)
{
    m_mConstraints.insert({"destination_ip", constraint()});
    m_mConstraints.insert({"source_port", constraint()});

    if(m_nParamsSupported & TransportParamsRTP::FEC)
    {
        m_mConstraints.insert({"fec_type", constraint()});
        m_mConstraints.insert({"fec_block_width", constraint()});
        m_mConstraints.insert({"fec_block_height", constraint()});
        m_mConstraints.insert({"fec1D_source_port", constraint()});
        m_mConstraints.insert({"fec2D_source_port", constraint()});
    }
    if(m_nParamsSupported & TransportParamsRTP::RTCP)
    {
        m_mConstraints.insert({"rtcp_source_port", constraint()});
    }
}

ConstraintsReceiver::ConstraintsReceiver(int nSupported) : Constraints(nSupported)
{
    m_mConstraints.insert({"interface_ip", constraint()});
    if(m_nParamsSupported & TransportParamsRTP::MULTICAST)
    {
        m_mConstraints.insert({"multicast_ip", constraint()});
    }
}



