#include "sourcegeneric.h"

SourceGeneric::SourceGeneric(std::string sLabel, std::string sDescription, std::string sDeviceId, enumFormat eFormat) :
    Source(sLabel, sDescription, sDeviceId, eFormat)
{

}

SourceGeneric::SourceGeneric() : Source(Source::VIDEO)
{

}

bool SourceGeneric::UpdateFromJson(const Json::Value& jsData)
{
    Source::UpdateFromJson(jsData);
    if(jsData["format"].isString() == false)
    {
        m_bIsOk = false;
        m_ssJsonError << "'format' is not a string" << std::endl;
    }

    if(m_bIsOk)
    {
        if(jsData["format"].asString().find("urn:x-nmos:format:video") != std::string::npos)
        {
            SetFormat(VIDEO);
        }
        else if(jsData["format"].asString().find("urn:x-nmos:format:data") != std::string::npos)
        {
            SetFormat(DATA);
        }
        else if(jsData["format"].asString().find("urn:x-nmos:format:mux") != std::string::npos)
        {
            SetFormat(MUX);
        }
        else
        {
            m_bIsOk = false;
            m_ssJsonError << "'format' " <<jsData["format"].asString() <<" incorrect" << std::endl;
        }
    }
    return m_bIsOk;
}
