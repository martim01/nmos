#pragma once
#include "resource.h"
#include <set>

class Source : public Resource
{
    public:
        enum enumFormat{AUDIO, VIDEO, DATA, MUX};


        Source(std::string sLabel, std::string sDescription, std::string sDeviceId, enumFormat eFormat);
        virtual ~Source(){}
        //Not yet defined
        void AddCap(){}
        void RemoveCap(){}


        void AddParentId(std::string sId);
        void RemoveParentId(std::string sId);

        void SetClock(std::string sClock);

        virtual Json::Value ToJson() const;

    private:
        std::string m_sDeviceId;
        std::set<std::string> m_setParent;
        std::string m_sClock;
        enumFormat m_eFormat;
};


