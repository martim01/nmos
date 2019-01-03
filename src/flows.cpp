#include "flows.h"

Flows::Flows()
{

}

Flows::~Flows()
{
    for(std::map<std::string, Flow*>::const_iterator itFlow = m_mFlow.begin(); itFlow != m_mFlow.end(); ++itFlow)
    {
        delete itFlow->second;
    }
}

void Flows::AddFlow(Flow* pFlow)
{
    m_mFlow.insert(make_pair(pFlow->GetId(), pFlow));
}

void Flows::RemoveFlow(Flow* pFlow)
{
    m_mFlow.erase(pFlow->GetId());
}

Json::Value Flows::Commit() const
{
    Json::Value json(Json::arrayValue);
    for(std::map<std::string, Flow*>::const_iterator itFlow = m_mFlow.begin(); itFlow != m_mFlow.end(); ++itFlow)
    {
        json.append(itFlow->second->Commit());
    }
    return json;
}
