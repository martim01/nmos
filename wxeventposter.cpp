#include "wxeventposter.h"
#include "log.h"
#include "mdns.h"


DEFINE_EVENT_TYPE(wxEVT_NMOS_MDNS_ALLFORNOW)
DEFINE_EVENT_TYPE(wxEVT_NMOS_MDNS_FINISHED)
DEFINE_EVENT_TYPE(wxEVT_NMOS_MDNS_RESOLVED)
DEFINE_EVENT_TYPE(wxEVT_NMOS_MDNS_REMOVED)
DEFINE_EVENT_TYPE(wxEVT_NMOS_CURL_DONE)
DEFINE_EVENT_TYPE(wxEVT_NMOS_REG_ERROR)

wxEventPoster::wxEventPoster(wxEvtHandler* pHandler) :
m_pHandler(pHandler)
{

}

void wxEventPoster::InstanceResolved(dnsInstance* pInstance)
{
    if(m_pHandler)
    {
        wxCommandEvent* pEvent = new wxCommandEvent(wxEVT_NMOS_MDNS_RESOLVED);
        pEvent->SetClientData(reinterpret_cast<void*>(pInstance));
        wxQueueEvent(m_pHandler, pEvent);
    }
}

void wxEventPoster::InstanceRemoved(const std::string& sInstance)
{
    if(m_pHandler)
    {
        wxCommandEvent* pEvent = new wxCommandEvent(wxEVT_NMOS_MDNS_REMOVED);
        pEvent->SetString(wxString::FromAscii(sInstance.c_str()));
        wxQueueEvent(m_pHandler, pEvent);
    }
}

void wxEventPoster::AllForNow(const std::string& sService)
{
    if(m_pHandler)
    {
        wxCommandEvent* pEvent = new wxCommandEvent(wxEVT_NMOS_MDNS_ALLFORNOW);
        pEvent->SetString(wxString::FromAscii(sService.c_str()));
        wxQueueEvent(m_pHandler, pEvent);
    }
}

void wxEventPoster::Finished()
{
    if(m_pHandler)
    {
        wxCommandEvent* pEvent = new wxCommandEvent(wxEVT_NMOS_MDNS_FINISHED);
        wxQueueEvent(m_pHandler, pEvent);
    }
}

void wxEventPoster::CurlDone(unsigned long nResult, const std::string& sResult, long nUserType)
{
    if(m_pHandler)
    {
        wxCommandEvent* pEvent(new wxCommandEvent(wxEVT_NMOS_CURL_DONE));
        pEvent->SetInt(nResult);
        pEvent->SetString(wxString::FromAscii(sResult.c_str()));
        pEvent->SetExtraLong(nUserType);
        wxQueueEvent(m_pHandler, pEvent);
    }
}

void wxEventPoster::RegistrationNodeError()
{
    if(m_pHandler)
    {
        wxCommandEvent* pEvent = new wxCommandEvent(wxEVT_NMOS_REG_ERROR);
        wxQueueEvent(m_pHandler, pEvent);
    }
}
