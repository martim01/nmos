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




    private:
        std::map<std::string, std::string> m_mChannel;

};
