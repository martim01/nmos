#pragma once
#include "source.h"
#include "nmosdlldefine.h"


class NMOS_EXPOSE SourceAudio : public Source
{
    public:

        SourceAudio(std::string sLabel, std::string sDescription, std::string sDeviceId);
        SourceAudio();
        virtual bool UpdateFromJson(const Json::Value& jsData);
        void AddChannel(std::string sLabel, std::string sSymbol);
        void RemoveChannel(std::string sSymbol);


        bool Commit(const ApiVersion& version);

        size_t GetNumberOfChannels() const;




    private:
        std::map<std::string, std::string> m_mChannel;
        size_t m_nCommitedChannelCount;
};
