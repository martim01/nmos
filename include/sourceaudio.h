#pragma once
#include "source.h"
#include "dlldefine.h"


class NMOS_EXPOSE SourceAudio : public Source
{
    public:

        SourceAudio(std::string sLabel, std::string sDescription, std::string sDeviceId);

        void AddChannel(std::string sLabel, std::string sSymbol);
        void RemoveChannel(std::string sSymbol);


        bool Commit();

        size_t GetNumberOfChannels() const;




    private:
        std::map<std::string, std::string> m_mChannel;
        size_t m_nCommitedChannelCount;
};
