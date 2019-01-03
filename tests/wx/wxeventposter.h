#pragma once
#include "eventposter.h"
#include "connection.h"
#include <wx/event.h>

class wxEventPoster : public EventPoster
{
    public:
        wxEventPoster(wxEvtHandler* pHandler);
        virtual ~wxEventPoster(){}

    protected:
        void CurlDone(unsigned long nResult, const std::string& sResponse, long nType);
        void InstanceResolved(dnsInstance* pInstance);
        void AllForNow(const std::string& sService);
        void Finished();
        void RegistrationNodeError();
        void InstanceRemoved(const std::string& sInstance);

        void Target(const std::string& sReceiverId, std::shared_ptr<Sender> pSender, unsigned short nPort);
        void PatchSender(const std::string& sSenderId, const connectionSender& conPatch, unsigned short nPort);
        void PatchReceiver(const std::string& sReceiverId, const connectionReceiver& conPatch, unsigned short nPort);
        void ActivateSender(const std::string& sSenderId);
        void ActivateReceiver(const std::string& sReceiverId);

    private:
        wxEvtHandler* m_pHandler;
};


class wxNmosEvent : public wxCommandEvent
{

public:
    /** Constructor
    *   @param commandType should be wxEVT_NI
    *   @param id the ID of the button list control
    **/
    wxNmosEvent(wxEventType type);

    wxNmosEvent() : wxCommandEvent(){}

    /** Copy Constructor
    *   @param event a wxNIEvent
    **/
    wxNmosEvent(const wxNmosEvent& event);

    /** Destructor
    **/
    ~wxNmosEvent(){}

    /** Creates a copy of the wxNmosEvent
    *   @return <i>wxNmosEvent</i>
    **/
    virtual wxEvent *Clone() const { return new wxNmosEvent(*this); }

    void SetSender(std::shared_ptr<Sender> pSender);
    const std::shared_ptr<Sender> GetSender() const;

    void SetSenderConnection(const connectionSender& con);
    const connectionSender& GetSenderConnection() const;

    void SetReceiverConnection(const connectionReceiver& con);
    const connectionReceiver& GetReceiverConnection() const;

    wxString GetId() const;
    wxString GetCurlResponse() const;
    wxString GetService() const;
    int GetPort() const;
    int GetCurlResult() const;
    int GetCurlType() const;

    DECLARE_DYNAMIC_CLASS(wxNmosEvent)



private:

    std::shared_ptr<Sender> m_pSender;
    connectionSender m_conSender;
    connectionReceiver m_conReceiver;

};

typedef void (wxEvtHandler::*wxNmosEventFunction)(wxNmosEvent&);


BEGIN_DECLARE_EVENT_TYPES()
    DECLARE_EXPORTED_EVENT_TYPE(WXEXPORT, wxEVT_NMOS_MDNS_RESOLVED, -1)
    DECLARE_EXPORTED_EVENT_TYPE(WXEXPORT, wxEVT_NMOS_MDNS_ALLFORNOW, -1)
    DECLARE_EXPORTED_EVENT_TYPE(WXEXPORT, wxEVT_NMOS_MDNS_FINISHED, -1)
    DECLARE_EXPORTED_EVENT_TYPE(WXEXPORT, wxEVT_NMOS_CURL_DONE, -1)
    DECLARE_EXPORTED_EVENT_TYPE(WXEXPORT, wxEVT_NMOS_REG_ERROR, -1)
    DECLARE_EXPORTED_EVENT_TYPE(WXEXPORT, wxEVT_NMOS_TARGET, -1)
    DECLARE_EXPORTED_EVENT_TYPE(WXEXPORT, wxEVT_NMOS_PATCH_SENDER, -1)
    DECLARE_EXPORTED_EVENT_TYPE(WXEXPORT, wxEVT_NMOS_PATCH_RECEIVER, -1)
    DECLARE_EXPORTED_EVENT_TYPE(WXEXPORT, wxEVT_NMOS_ACTIVATE_RECEIVER, -1)
    DECLARE_EXPORTED_EVENT_TYPE(WXEXPORT, wxEVT_NMOS_ACTIVATE_SENDER, -1)
END_DECLARE_EVENT_TYPES()

#define wxNmosEventHandler(func) \
    (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(wxNmosEventFunction, &func)

#define EVT_NMOS_MDNS_RESOLVED(id,fn) DECLARE_EVENT_TABLE_ENTRY(wxEVT_NMOS_MDNS_RESOLVED,id,-1,(wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction) (wxNmosEventFunction) &fn, (wxObject*) NULL),
#define EVT_NMOS_MDNS_ALLFORNOW(id,fn) DECLARE_EVENT_TABLE_ENTRY(wxEVT_NMOS_MDNS_ALLFORNOW,id,-1,(wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction) (wxNmosEventFunction) &fn, (wxObject*) NULL),
#define EVT_NMOS_MDNS_FINISHED(id,fn) DECLARE_EVENT_TABLE_ENTRY(wxEVT_NMOS_MDNS_FINISHED,id,-1,(wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction) (wxNmosEventFunction) &fn, (wxObject*) NULL),
#define EVT_NMOS_CURL_DONE(id,fn) DECLARE_EVENT_TABLE_ENTRY(wxEVT_NMOS_CURL_DONE,id,-1,(wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction) (wxNmosEventFunction) &fn, (wxObject*) NULL),
#define EVT_NMOS_REG_ERROR(id,fn) DECLARE_EVENT_TABLE_ENTRY(wxEVT_NMOS_REG_ERROR,id,-1,(wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction) (wxNmosEventFunction) &fn, (wxObject*) NULL),
#define EVT_NMOS_TARGET(id,fn) DECLARE_EVENT_TABLE_ENTRY(wxEVT_NMOS_TARGET,id,-1,(wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction) (wxNmosEventFunction) &fn, (wxObject*) NULL),
#define EVT_NMOS_PATCH_SENDER(id,fn) DECLARE_EVENT_TABLE_ENTRY(wxEVT_NMOS_PATCH_SENDER,id,-1,(wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction) (wxNmosEventFunction) &fn, (wxObject*) NULL),
#define EVT_NMOS_PATCH_RECEIVER(id,fn) DECLARE_EVENT_TABLE_ENTRY(wxEVT_NMOS_PATCH_RECEIVER,id,-1,(wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction) (wxNmosEventFunction) &fn, (wxObject*) NULL),
#define EVT_NMOS_ACTIVATE_RECEIVER(id,fn) DECLARE_EVENT_TABLE_ENTRY(wxEVT_NMOS_ACTIVATE_RECEIVER,id,-1,(wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction) (wxNmosEventFunction) &fn, (wxObject*) NULL),
#define EVT_NMOS_ACTIVATE_SENDER(id,fn) DECLARE_EVENT_TABLE_ENTRY(wxEVT_NMOS_ACTIVATE_SENDER,id,-1,(wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction) (wxNmosEventFunction) &fn, (wxObject*) NULL),





