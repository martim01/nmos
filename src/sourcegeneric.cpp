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
    m_bIsOk &= jsData["format"].isString();

    if(m_bIsOk)
    {
        if(jsData["format"] == "urn:x-nmos:format:video")
        {
            SetFormat(VIDEO);
        }
        else if(jsData["format"] == "urn:x-nmos:format:data")
        {
            SetFormat(DATA);
        }
        else if(jsData["format"] == "urn:x-nmos:format:mux")
        {
            SetFormat(MUX);
        }
        else
        {
            m_bIsOk = false;
        }
    }
    return m_bIsOk;
}
