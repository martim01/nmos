#pragma once
#include "source.h"

class SourceAudio : public Source
{
    public:

        SourceAudio(std::string sLabel, std::string sDescription, std::string sDeviceId);

        void AddChannel(std::string sLabel, std::string sSymbol);
        void RemoveChannel(std::string sSymbol);


        Json::Value ToJson() const;




    private:
        std::map<std::string, std::string> m_mChannel;

};
