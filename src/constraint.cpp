#include "constraint.h"
#include <sstream>
#include "transportparams.h"

using namespace std;
using namespace pml::nmos;

const std::string constraint::MAXIMUM = "maximum";
const std::string constraint::MINIMUM = "minimum";
const std::string constraint::PATTERN = "pattern";
const std::string constraint::ENUM = "enum";

constraint::constraint(const std::string& sDescription) :
    m_sDescription(sDescription)
{

}


void constraint::SetMinimum(int nMinimum)
{
    m_minimum = nMinimum;
}

void constraint::SetMaximum(int nMaximum)
{
    m_maximum = nMaximum;
}

void constraint::SetEnum(const std::vector<pairEnum_t>& vConstraints)
{
    m_vEnum = vConstraints;
}

void constraint::SetPattern(const std::string& sPattern)
{
    m_pattern = sPattern;
}


void constraint::RemoveMinimum()
{
    m_minimum.reset();
}

void constraint::RemoveMaximum()
{
    m_maximum.reset();
}

void constraint::RemoveEnum()
{
    m_vEnum.clear();
}

void constraint::RemovePattern()
{
    m_pattern.reset();
}

Json::Value constraint::GetJson() const
{
    Json::Value jsConstraint(Json::objectValue);
    if(m_minimum)
    {
        jsConstraint[MINIMUM] = *m_minimum;
    }
    if(m_maximum)
    {
        jsConstraint[MAXIMUM] = *m_maximum;
    }
    if(m_pattern)
    {
        jsConstraint[PATTERN] = *m_pattern;
    }
    if(m_vEnum.empty() == false)
    {
        jsConstraint[ENUM] = Json::arrayValue;
        for(size_t i = 0; i < m_vEnum.size(); i++)
        {
            switch(m_vEnum[i].first)
            {
                case jsondatatype::_BOOLEAN:
                    if(m_vEnum[i].second == "true")
                    {
                        jsConstraint[ENUM].append(true);
                    }
                    else
                    {
                        jsConstraint[ENUM].append(false);
                    }
                    break;
                case jsondatatype::_INTEGER:
                    jsConstraint[ENUM].append(stoi(m_vEnum[i].second));
                    break;
                case jsondatatype::_NULL:
                    jsConstraint[ENUM].append(Json::nullValue);
                    break;
                case jsondatatype::_STRING:
                    jsConstraint[ENUM].append(m_vEnum[i].second);
                    break;
                case jsondatatype::_NUMBER:
                    jsConstraint[ENUM].append(stod(m_vEnum[i].second));
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
    if(m_minimum)
    {
        int nMin;
        try
        {
            nMin = stoi(sValue);
            bMeets &= (nMin >= *m_minimum);
        }
        catch(const invalid_argument& ia)
        {
            bMeets = false;
        }
    }
    if(m_maximum)
    {
        int nMax;
        try
        {
            nMax = stoi(sValue);
            bMeets &= (nMax <= *m_maximum);
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

    if(m_pattern)
    {
        // @todo constraint pattern checking
    }
    return bMeets;
}

bool constraint::MeetsConstraint(int nValue)
{
    bool bMeets = true;
    //is the min set
    if(m_minimum)
    {
        bMeets &= (nValue >= *m_minimum);
    }
    if(m_maximum)
    {
        bMeets &= (nValue <= *m_maximum);
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

    if(m_pattern)
    {
        // @todo constraint pattern checking
    }
    return bMeets;
}

bool constraint::MeetsConstraint(unsigned int nValue)
{
    bool bMeets = true;
    //is the min set
    if(m_minimum)
    {
        bMeets &= (nValue >= *m_minimum);
    }
    if(m_maximum)
    {
        bMeets &= (nValue <= *m_maximum);
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

    if(m_pattern)
    {
        // @todo constraint pattern checking
    }
    return bMeets;
}

bool constraint::MeetsConstraint(double dValue)
{
    bool bMeets = true;
    //is the min set
    if(m_minimum)
    {
        bMeets &= (dValue >= *m_minimum);
    }
    if(m_maximum)
    {
        bMeets &= (dValue <= *m_maximum);
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

    if(m_pattern)
    {
        // @todo constraint pattern checking
    }
    return bMeets;
}

bool constraint::MeetsConstraint(bool bValue)
{
    bool bMeets = true;
    //is the min set
    if(m_minimum)
    {
        bMeets &= (bValue >= *m_minimum);
    }
    if(m_maximum)
    {
        bMeets &= (bValue <= *m_maximum);
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

    if(m_pattern)
    {
        // @todo constraint pattern checking
    }
    return bMeets;
}

bool constraint::UpdateFromJson(const Json::Value& jsConstraint)
{
    if(CheckJsonAllowed(jsConstraint, { {MAXIMUM, {jsondatatype::_NUMBER, jsondatatype::_INTEGER}},
                          {MINIMUM, {jsondatatype::_NUMBER, jsondatatype::_INTEGER}},
                          {ENUM, {jsondatatype::_ARRAY}},
                          {PATTERN, {jsondatatype::_STRING}}}) == false)
    {
        return false;
    }

    if(jsConstraint.isMember(MAXIMUM))
    {
        m_minimum = jsConstraint.asInt();   //@todo this could be a double
    }
    if(jsConstraint.isMember(MINIMUM))
    {
        m_minimum = jsConstraint.asInt();   //@todo this could be a double
    }
    if(jsConstraint.isMember(ENUM))
    {
        m_vEnum.clear();
        m_vEnum.resize(jsConstraint[ENUM].size());
        for(Json::ArrayIndex ai = 0; ai < jsConstraint[ENUM].size(); ai++)
        {
            switch(jsConstraint[ENUM][ai].type())
            {
                case Json::ValueType::booleanValue:
                    m_vEnum[ai] = {jsondatatype::_BOOLEAN, jsConstraint[ENUM][ai].asBool() ? "true" : "false"};
                    break;
                case Json::ValueType::intValue:
                case Json::ValueType::uintValue:
                    m_vEnum[ai] = {jsondatatype::_INTEGER, jsConstraint[ENUM][ai].asString()};
                    break;
                case Json::ValueType::realValue:
                    m_vEnum[ai] = {jsondatatype::_NUMBER, jsConstraint[ENUM][ai].asString()};
                    break;
                case Json::ValueType::nullValue:
                    m_vEnum[ai] = {jsondatatype::_NULL, "null"};
                    break;
                case Json::ValueType::stringValue:
                    m_vEnum[ai] = {jsondatatype::_STRING, jsConstraint[ENUM][ai].asString()};
                    break;
                default:
                    return false;
            }

        }
    }
    if(jsConstraint.isMember(PATTERN))
    {
        m_pattern = jsConstraint.asString();
    }
    return true;
}



Constraints::Constraints()
{

}

Json::Value Constraints::GetJson() const
{
    Json::Value jsConstraints(Json::objectValue);

    for(const auto& pairConstraint : m_mConstraints)
    {
        jsConstraints[pairConstraint.first] = pairConstraint.second.GetJson();
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

bool Constraints::MeetsConstraint(const std::string& sKey, const Json::Value& jsCheck)
{
    auto itConstraint = m_mConstraints.find(sKey);
    if(itConstraint != m_mConstraints.end())
    {
        switch(jsCheck.type())
        {
            case Json::ValueType::arrayValue:
            case Json::ValueType::nullValue:
            case Json::ValueType::objectValue:
                return false;
            case Json::ValueType::booleanValue:
                return itConstraint->second.MeetsConstraint(jsCheck.asBool());
            case Json::ValueType::intValue:
            case Json::ValueType::uintValue:
                return itConstraint->second.MeetsConstraint(jsCheck.asInt());
            case Json::ValueType::realValue:
               return itConstraint->second.MeetsConstraint(jsCheck.asDouble());
            case Json::ValueType::stringValue:
                return itConstraint->second.MeetsConstraint(jsCheck.asString());
        }
    }
    return true;
}

bool Constraints::AddConstraint(const std::string& sKey, const std::experimental::optional<int>& minValue, const std::experimental::optional<int>& maxValue, const std::experimental::optional<std::string>& pattern,
                                const std::vector<pairEnum_t>& vEnum)
{
    auto itConstraint = m_mConstraints.find(sKey);
    if(itConstraint != m_mConstraints.end())
    {
        if(minValue)
        {
            itConstraint->second.SetMinimum(*minValue);
        }
        if(maxValue)
        {
            itConstraint->second.SetMaximum(*maxValue);
        }
        if(pattern)
        {
            itConstraint->second.SetPattern(*pattern);
        }
        if(vEnum.size() != 0)
        {
            itConstraint->second.SetEnum(vEnum);
        }
        return true;
    }
    return false;
}

bool Constraints::ClearConstraint(const std::string& sKey)
{
    auto itConstraint = m_mConstraints.find(sKey);
    if(itConstraint != m_mConstraints.end())
    {
        itConstraint->second.RemoveMinimum();
        itConstraint->second.RemoveMaximum();
        itConstraint->second.RemoveEnum();
        itConstraint->second.RemovePattern();
        return true;
    }
    else
    {
        return false;
    }
}

void Constraints::CreateEmptyConstraint(const std::string& sKey)
{
    m_mConstraints.insert({sKey, constraint()});
}
