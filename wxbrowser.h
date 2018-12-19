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

