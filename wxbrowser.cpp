#include "wxbrowser.h"
#include "log.h"
#include "avahibrowser.h"


DEFINE_EVENT_TYPE(wxEVT_BROWSER_ALLFORNOW)
DEFINE_EVENT_TYPE(wxEVT_BROWSER_FINISHED)
DEFINE_EVENT_TYPE(wxEVT_BROWSER_RESOLVED)

wxBrowserEvent::wxBrowserEvent(wxEvtHandler* pHandler) : ServiceBrowserEvent(),
m_pHandler(pHandler)
{

}

void wxBrowserEvent::InstanceResolved(dnsInstance* pInstance)
{
    Log::Get() << "wxBrowserEvent: Instance '" << pInstance->sName << "' resolved at '" << pInstance->sHostIP << "'" << std::endl;
    if(m_pHandler)
    {
        wxCommandEvent event(wxEVT_BROWSER_RESOLVED);
        event.SetClientData(reinterpret_cast<void*>(pInstance));
        wxPostEvent(m_pHandler, event);
    }
}

void wxBrowserEvent::AllForNow(const std::string& sService)
{
    Log::Get() << "wxBrowserEvent: All for now" << std::endl;
    if(m_pHandler)
    {
        wxCommandEvent* pEvent = new wxCommandEvent(wxEVT_BROWSER_ALLFORNOW);
        pEvent->SetString(wxString::FromAscii(sService.c_str()));
        wxQueueEvent(m_pHandler, pEvent);
    }
}

void wxBrowserEvent::Finished()
{
    if(m_pHandler)
    {
        wxCommandEvent event(wxEVT_BROWSER_FINISHED);
        wxPostEvent(m_pHandler, event);
    }
}
