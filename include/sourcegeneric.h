#pragma once
#include "source.h"
#include "nmosdlldefine.h"

class NMOS_EXPOSE SourceGeneric : public Source
{
    public:
        SourceGeneric(std::string sLabel, std::string sDescription, std::string sDeviceId, enumFormat eFormat);
        SourceGeneric();
        virtual bool UpdateFromJson(const Json::Value& jsData);
    private:

};
