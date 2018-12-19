#pragma once
#include <wx/event.h>

class wxNmos : public wxEvtHandler
{
    public:
        static wxNmos& Get();

};
