#include "sources.h"

Sources::Sources()
{

}

Sources::~Sources()
{
    for(std::map<std::string, Source*>::iterator itSource = m_mSource.begin(); itSource != m_mSource.end(); ++itSource)
    {
        delete itSource->second;
    }
    m_mSource.clear();
}

void Sources::AddSource(Source* pSource)
{
    m_mSource.insert(make_pair(pSource->GetId(), pSource));
}

void Sources::RemoveSource(Source* pSource)
{
    m_mSource.erase(pSource->GetId());
    delete pSource;
}

Json::Value Sources::Commit() const
{
    Json::Value json(Json::arrayValue);
    for(std::map<std::string, Source*>::const_iterator itSource = m_mSource.begin(); itSource != m_mSource.end(); ++itSource)
    {
        json.append(itSource->second->Commit());
    }
    return json;
}
