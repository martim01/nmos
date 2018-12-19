#pragma once
#include "eventposter.h"
#include <wx/event.h>

class wxEventPoster : public EventPoster
{
    public:
        wxEventPoster(wxEvtHandler* pHandler);
        virtual ~wxEventPoster(){}
        virtual void CurlDone(unsigned long nResult, const std::string& sResponse, long nType);
        virtual void InstanceResolved(dnsInstance* pInstance);
        virtual void AllForNow(const std::string& sService);
        virtual void Finished();
        virtual void RegistrationNodeError();
        virtual void InstanceRemoved(const std::string& sInstance);
    private:
        wxEvtHandler* m_pHandler;
};

DECLARE_EXPORTED_EVENT_TYPE(WXEXPORT, wxEVT_NMOS_MDNS_RESOLVED, -1)
DECLARE_EXPORTED_EVENT_TYPE(WXEXPORT, wxEVT_NMOS_MDNS_REMOVED, -1)
DECLARE_EXPORTED_EVENT_TYPE(WXEXPORT, wxEVT_NMOS_MDNS_ALLFORNOW, -1)
DECLARE_EXPORTED_EVENT_TYPE(WXEXPORT, wxEVT_NMOS_MDNS_FINISHED, -1)
DECLARE_EXPORTED_EVENT_TYPE(WXEXPORT, wxEVT_NMOS_CURL_DONE, -1)
DECLARE_EXPORTED_EVENT_TYPE(WXEXPORT, wxEVT_NMOS_REG_ERROR, -1)

