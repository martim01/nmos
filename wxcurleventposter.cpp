#include "wxcurleventposter.h"

DEFINE_EVENT_TYPE(wxEVT_CURL_DONE)

wxCurlEventPoster::wxCurlEventPoster(wxEvtHandler* pHandler) : m_pHandler(pHandler)
{

}

void wxCurlEventPoster::CurlDone(unsigned long nResult, const std::string& sResult)
{
    if(m_pHandler)
    {
        wxCommandEvent* pEvent(new wxCommandEvent(wxEVT_CURL_DONE));
        pEvent->SetInt(nResult);
        pEvent->SetString(wxString::FromAscii(sResult.c_str()));
        wxQueueEvent(m_pHandler, pEvent);
    }
}
