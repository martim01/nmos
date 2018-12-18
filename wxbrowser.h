#pragma once
#include "servicebrowserevent.h"
#include <wx/event.h>

class wxBrowserEvent : public ServiceBrowserEvent
{
    public:
        wxBrowserEvent(wxEvtHandler* pHandler);
        virtual void InstanceResolved(dnsInstance* pInstance);
        virtual void AllForNow(const std::string& sService);
        virtual void Finished();

    protected:
        wxEvtHandler* m_pHandler;
};

DECLARE_EXPORTED_EVENT_TYPE(WXEXPORT, wxEVT_BROWSER_RESOLVED, -1)
DECLARE_EXPORTED_EVENT_TYPE(WXEXPORT, wxEVT_BROWSER_ALLFORNOW, -1)
DECLARE_EXPORTED_EVENT_TYPE(WXEXPORT, wxEVT_BROWSER_FINISHED, -1)
