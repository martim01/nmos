#pragma once
#include "curlevent.h"
#include <wx/event.h>

class wxCurlEventPoster : public CurlEvent
{
    public:
        wxCurlEventPoster(wxEvtHandler* pHandler);
        void CurlDone(unsigned long nResult, const std::string& sResponse, long nUserType);

    private:
        wxEvtHandler* m_pHandler;

};


