#include "wxlogoutput.h"

DEFINE_EVENT_TYPE(wxEVT_NMOS_LOG)

wxLogOutput::wxLogOutput(wxEvtHandler* pHandler) : m_pHandler(pHandler)
{

}

void wxLogOutput::Flush(int nLogLevel, const std::stringstream& logStream)
{
    wxCommandEvent* pEvent = new wxCommandEvent(wxEVT_NMOS_LOG);
    pEvent->SetInt(nLogLevel);
    wxString sLog(wxString::FromAscii(logStream.str().c_str()));
    sLog.Replace(wxT("\n"), wxT(""));
    pEvent->SetString(sLog);
    wxQueueEvent(m_pHandler, pEvent);
}
