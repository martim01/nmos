#pragma once
#include "flowaudio.h"
#include "nmosdlldefine.h"

namespace pml
{
    namespace nmos
    {
        class NMOS_EXPOSE FlowAudioCoded : public FlowAudio
        {
            public:

                FlowAudioCoded(const std::string& sLabel, const std::string& sDescription, const std::string& sSourceId, const std::string& sDeviceId, unsigned int nSampleRate, const std::string& sMediaType);
                FlowAudioCoded();
                static std::shared_ptr<FlowAudioCoded> Create(const Json::Value& jsResponse);
                virtual bool Commit(const ApiVersion& version);


                virtual bool UpdateFromJson(const Json::Value& jsData);
                void SetMediaType(const std::string& sMediaType);

                std::string CreateSDPMediaLine(unsigned short nPort) const override;
                std::string CreateSDPAttributeLines(std::shared_ptr<const Source> pSource) const override;

            protected:

            private:


                unsigned int m_nSampleRate;
                std::string m_sMediaType;

                static std::map<std::string, unsigned short> m_mRtpTypes;
        };
    };
};
