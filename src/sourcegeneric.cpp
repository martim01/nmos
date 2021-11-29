#include "sourcegeneric.h"

using namespace pml::nmos;

SourceGeneric::SourceGeneric(const std::string& sLabel, const std::string& sDescription, const std::string& sDeviceId, enumFormat eFormat) :
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
        m_ssJsonError << "'format' is not a string" ;
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
            m_ssJsonError << "'format' " <<jsData["format"].asString() <<" incorrect" ;
        }
    }
    return m_bIsOk;
}
std::shared_ptr<SourceGeneric> SourceGeneric::Create(const Json::Value& jsResponse)
{
    auto pResource  = std::make_shared<SourceGeneric>();
    if(pResource->UpdateFromJson(jsResponse))
    {
        return pResource;
    }
    return nullptr;
}
