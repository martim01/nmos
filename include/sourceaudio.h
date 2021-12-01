#pragma once
#include "source.h"
#include "nmosdlldefine.h"

namespace pml
{
    namespace nmos
    {
        class NMOS_EXPOSE SourceAudio : public Source
        {
            public:

                SourceAudio(const std::string& sLabel, const std::string& sDescription, const std::string& sDeviceId);
                static std::shared_ptr<SourceAudio> Create(const Json::Value& jsResponse);
                SourceAudio();
                virtual bool UpdateFromJson(const Json::Value& jsData);
                void AddChannels(const std::map<std::string, std::string>& mChannels);
                void AddChannel(const std::string& sLabel, const std::string& sSymbol);
                void RemoveChannel(const std::string& sSymbol);


                bool Commit(const ApiVersion& version);

                size_t GetNumberOfChannels() const;




            private:

                std::map<std::string, std::string> m_mChannel;
                size_t m_nCommitedChannelCount;
        };
    };
};
