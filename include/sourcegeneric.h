#pragma once
#include "source.h"
#include "dlldefine.h"

class NMOS_EXPOSE SourceGeneric : public Source
{
    public:
        SourceGeneric(std::string sLabel, std::string sDescription, std::string sDeviceId, enumFormat eFormat);

    private:

};