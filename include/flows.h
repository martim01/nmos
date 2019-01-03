#pragma once
#include "flow.h"
#include "resourceholder.h"

class Flows : public ResourceHolder
{
    public:
        Flows();
        ~Flows();
        void AddFlow(Flow* pFlow);
        void RemoveFlow(Flow* pFlow);

        Json::Value Commit() const;

        std::map<std::string, Flow*>::const_iterator GetFlowBegin()
        {
            return m_mFlow.begin();
        }
        std::map<std::string, Flow*>::const_iterator GetFlowEnd()
        {
            return m_mFlow.end();
        }
        std::map<std::string, Flow*>::const_iterator FindFlow(std::string sFlowId)
        {
            return m_mFlow.find(sFlowId);
        }
    private:
        std::map<std::string, Flow*> m_mFlow;

};

