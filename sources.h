#pragma once
#include "source.h"
#include "resourceholder.h"

class Sources : public ResourceHolder
{
    public:
        Sources();
        ~Sources();
        void AddSource(Source* pSource);
        void RemoveSource(Source* pSource);

        std::map<std::string, Source*>::const_iterator GetSourceBegin()
        {
            return m_mSource.begin();
        }
        std::map<std::string, Source*>::const_iterator GetSourceEnd()
        {
            return m_mSource.end();
        }
        std::map<std::string, Source*>::const_iterator FindSource(std::string sSourceId)
        {
            return m_mSource.find(sSourceId);
        }


        Json::Value Commit() const;
    private:
        std::map<std::string, Source*> m_mSource;

};
