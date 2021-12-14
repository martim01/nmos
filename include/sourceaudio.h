#pragma once
#include "source.h"
#include "nmosdlldefine.h"
#include "namedtype.h"

using channelLabel = NamedType<std::string,  struct channelLabelParameter>;
using channelSymbol = NamedType<std::string,  struct channelSymbolParameter>;


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
                void AddChannels(const std::map<channelSymbol, channelLabel>& mChannels);
                void AddChannel(const channelSymbol& symbol, const channelLabel& label);
                void RemoveChannel(const channelSymbol& symbol);


                bool Commit(const ApiVersion& version);

                size_t GetNumberOfChannels() const;




            private:

                std::map<channelSymbol, channelLabel> m_mChannel;
                size_t m_nCommitedChannelCount;
        };
    };
};
