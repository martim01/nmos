#include "ioresource.h"

using namespace pml::nmos;

const std::array<std::string,6> IOResource::STR_TRANSPORT = {"urn:x-nmos:transport:rtp", "urn:x-nmos:transport:rtp.ucast", "urn:x-nmos:transport:rtp.mcast","urn:x-nmos:transport:dash", "urn:x-nmos:transport:mqtt", "urn:x-nmos:transport:websocket"};


void IOResource::AddInterfaceBinding(const std::string& sInterface)
{
    m_setInterfaces.insert(sInterface);
    UpdateVersionTime();
}

void IOResource::RemoveInterfaceBinding(const std::string& sInterface)
{
    m_setInterfaces.erase(sInterface);
    UpdateVersionTime();
}


void IOResource::SetTransport(IOResource::enumTransport eTransport)
{
     m_eTransport = eTransport;
     UpdateVersionTime();
}

bool IOResource::AddConstraint(const std::string& sKey, const std::experimental::optional<int>& minValue, const std::experimental::optional<int>& maxValue, const std::experimental::optional<std::string>& pattern,
                                    const std::vector<pairEnum_t>& vEnum, const std::experimental::optional<size_t>& tp)
{
    if(tp)
    {
        if(*tp < m_vConstraints.size())
        {
            return m_vConstraints[*tp].AddConstraint(sKey, minValue, maxValue, pattern, vEnum);
        }
        else
        {
            return false;
        }
    }
    else
    {
        for(auto& con : m_vConstraints)
        {
            if(con.AddConstraint(sKey, minValue, maxValue, pattern, vEnum) == false)
            {
                return false;
            }
        }
        return true;
    }
}

bool IOResource::ClearConstraint(const std::string& sKey, const std::experimental::optional<size_t>& tp)
{
    if(tp)
    {
        if(*tp < m_vConstraints.size())
        {
            return m_vConstraints[*tp].ClearConstraint(sKey);
        }
        else
        {
            return false;
        }
    }
    else
    {
        for(auto& con : m_vConstraints)
        {
            if(con.ClearConstraint(sKey) == false)
            {
                return false;
            }
        }
        return true;
    }
}

bool IOResource::CheckConstraints(const Json::Value& jsonRequest)
{
    if(jsonRequest.isMember("transport_params") && jsonRequest["transport_params"].isArray())
    {
        if(jsonRequest["transport_params"].size() == m_vConstraints.size())
        {
            for(Json::ArrayIndex ai = 0; ai < jsonRequest["transport_params"].size(); ai++)
            {
                if(CheckConstraints(jsonRequest["transport_params"][ai], m_vConstraints[ai]) == false)
                {
                    return false;
                }
            }
        }
        else
        {
            return false;
        }
    }
    return true;
}

bool IOResource::CheckConstraints(const Json::Value& jsonRequest, const Constraints& con)
{
    for(auto itObject = jsonRequest.begin(); itObject != jsonRequest.end(); ++itObject)
    {
        if(con.MeetsConstraint(itObject.key().asString(), (*itObject)) == false)
        {
            return false;
        }
    }
    return true;
}

void IOResource::CreateConstraints(const Json::Value& jsonStaged)
{
    if(jsonStaged.isMember("transport_params") && jsonStaged["transport_params"].isArray())
    {
        for(Json::ArrayIndex ai = 0; ai < jsonStaged["transport_params"].size(); ai++)
        {
            m_vConstraints.push_back(Constraints(jsonStaged["transport_params"][ai]));
        }
    }
}

Json::Value IOResource::GetConnectionConstraintsJson(const ApiVersion& version) const
{
    Json::Value jsArray(Json::arrayValue);
    for(const auto& con : m_vConstraints)
    {
        jsArray.append(con.GetJson());
    }
    return jsArray;
}
