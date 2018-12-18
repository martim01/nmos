#pragma once
#include "log.h"
#include <wx/event.h>

class wxLogOutput : public LogOutput
{
    public:
        wxLogOutput(wxEvtHandler* pHandler);
        void Flush(int nLogLevel, const std::stringstream&  logStream);
    private:
        wxEvtHandler* m_pHandler;
};

DECLARE_EXPORTED_EVENT_TYPE(WXEXPORT, wxEVT_NMOS_LOG, -1)

