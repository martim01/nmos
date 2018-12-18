/***************************************************************
 * Name:      wxnmosMain.cpp
 * Purpose:   Code for Application Frame
 * Author:    Matthew Martin (martim01@outlook.com)
 * Created:   2018-12-17
 * Copyright: Matthew Martin ()
 * License:
 **************************************************************/

#include "wxnmosMain.h"
#include <wx/msgdlg.h>
#include "wxbrowser.h"
#include "nodeapi.h"
#include "sourceaudio.h"
#include "flowaudioraw.h"
#include "device.h"
#include "sender.h"
#include "receiver.h"
#include "wxbrowser.h"
#include "wxlogoutput.h"
#include "wxcurleventposter.h"

//(*InternalHeaders(wxnmosDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//helper functions
enum wxbuildinfoformat {
    short_f, long_f };

wxString wxbuildinfo(wxbuildinfoformat format)
{
    wxString wxbuild(wxVERSION_STRING);

    if (format == long_f )
    {
#if defined(__WXMSW__)
        wxbuild << _T("-Windows");
#elif defined(__UNIX__)
        wxbuild << _T("-Linux");
#endif

#if wxUSE_UNICODE
        wxbuild << _T("-Unicode build");
#else
        wxbuild << _T("-ANSI build");
#endif // wxUSE_UNICODE
    }

    return wxbuild;
}

//(*IdInit(wxnmosDialog)
const long wxnmosDialog::ID_LISTBOX1 = wxNewId();
const long wxnmosDialog::ID_TIMER1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(wxnmosDialog,wxDialog)
    //(*EventTable(wxnmosDialog)
    //*)
END_EVENT_TABLE()

wxnmosDialog::wxnmosDialog(wxWindow* parent,wxWindowID id)
{
    //(*Initialize(wxnmosDialog)
    Create(parent, id, _("wxWidgets app"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
    BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
    BoxSizer2 = new wxBoxSizer(wxVERTICAL);
    m_plbxLog = new wxListBox(this, ID_LISTBOX1, wxDefaultPosition, wxSize(1280,600), 0, 0, 0, wxDefaultValidator, _T("ID_LISTBOX1"));
    BoxSizer2->Add(m_plbxLog, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BoxSizer1->Add(BoxSizer2, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 4);
    SetSizer(BoxSizer1);
    m_timerHeartbeat.SetOwner(this, ID_TIMER1);
    BoxSizer1->Fit(this);
    BoxSizer1->SetSizeHints(this);

    Connect(ID_TIMER1,wxEVT_TIMER,(wxObjectEventFunction)&wxnmosDialog::OntimerHeartbeatTrigger);
    Connect(wxID_ANY,wxEVT_CLOSE_WINDOW,(wxObjectEventFunction)&wxnmosDialog::OnClose);
    //*)
    Connect(wxID_ANY, wxEVT_BROWSER_ALLFORNOW, (wxObjectEventFunction)&wxnmosDialog::OnBrowserAllForNow);
    Connect(wxID_ANY, wxEVT_BROWSER_FINISHED, (wxObjectEventFunction)&wxnmosDialog::OnBrowserFinished);
    Connect(wxID_ANY, wxEVT_BROWSER_RESOLVED, (wxObjectEventFunction)&wxnmosDialog::OnBrowserResolved);
    Connect(wxID_ANY, wxEVT_NMOS_LOG, (wxObjectEventFunction)&wxnmosDialog::OnLog);
    Connect(wxID_ANY, wxEVT_CURL_DONE, (wxObjectEventFunction)&wxnmosDialog::OnCurlDone);

    Log::Get().SetOutput(new wxLogOutput(this));


    NodeApi::Get().Init(8080, "host1", "host1");
    NodeApi::Get().GetSelf().AddApiVersion("v1.2");
//    NodeApi::Get().GetSelf().AddVersion("v1.0");
//    NodeApi::Get().GetSelf().AddVersion("v1.1");


    //NodeApi::Get().GetSelf().AddEndpoint("192.168.1.35", 8080, false);
    NodeApi::Get().GetSelf().AddInternalClock("clk0");
    NodeApi::Get().GetSelf().AddPTPClock("clk1", true, "IEEE1588-2008", "08-00-11-ff-fe-21-e1-b0", true);
    NodeApi::Get().GetSelf().AddInterface("eth0", "74-26-96-db-87-31", "74-26-96-db-87-32");


    Device* pDevice = new Device("TestDevice", "TestDescription", Device::GENERIC,NodeApi::Get().GetSelf().GetId());
    SourceAudio* pSource = new SourceAudio("TestAudio", "TestDescription", pDevice->GetId());
    pSource->AddChannel("Left", "L");
    pSource->AddChannel("Right", "R");

    FlowAudioRaw* pFlow = new FlowAudioRaw("TestFlow", "TestDescription", pSource->GetId(), pDevice->GetId(),48000, FlowAudioRaw::L24);
    Sender* pSender(new Sender("TestSender", "Description", pFlow->GetId(), Sender::RTP_MCAST, pDevice->GetId(), "http://192.168.1.35/by-name/pam.sdp"));
    pSender->AddInterfaceBinding("eth0");


    Receiver* pReceiver = new Receiver("Test Receiver", "TestDescription", Receiver::RTP_MCAST, pDevice->GetId(), Receiver::AUDIO);
    pReceiver->AddCap("audio/L24");
    pReceiver->AddCap("audio/L20");
    pReceiver->AddCap("audio/L16");
    pReceiver->AddInterfaceBinding("eth0");

    NodeApi::Get().GetDevices().AddResource(pDevice);
    NodeApi::Get().GetSources().AddResource(pSource);
    NodeApi::Get().GetFlows().AddResource(pFlow);
    NodeApi::Get().GetReceivers().AddResource(pReceiver);
    NodeApi::Get().GetSenders().AddResource(pSender);
    NodeApi::Get().Commit();

    NodeApi::Get().StartServices(new wxBrowserEvent(this), new wxCurlEventPoster(this));

}

wxnmosDialog::~wxnmosDialog()
{
    //(*Destroy(wxnmosDialog)
    //*)
}

void wxnmosDialog::OnQuit(wxCommandEvent& event)
{
    Close();
}

void wxnmosDialog::OnAbout(wxCommandEvent& event)
{
    wxString msg = wxbuildinfo(long_f);
    wxMessageBox(msg, _("Welcome to..."));
}

void wxnmosDialog::Log(const wxString& sLog)
{
    int nItem = m_plbxLog->Append(sLog);
    m_plbxLog->SetFirstItem(nItem);
}

void wxnmosDialog::OnBrowserResolved(wxCommandEvent& event)
{
    Log(wxT("Registation node resolved"));
}

void wxnmosDialog::OnBrowserAllForNow(wxCommandEvent& event)
{
    Log(wxT("Browser - all for now "));
    if(event.GetString() == "_nmos-registration._tcp")
    {
        m_nRegisterState = NodeApi::Get().Register();
        if(m_nRegisterState == NodeApi::REG_FAILED)
        {
            Log("No registration nodes found. Browse again in 30 seconds");
            m_timerHeartbeat.Start(30000, true);
        }
    }

}

void wxnmosDialog::OnCurlDone(wxCommandEvent& event)
{
    switch(event.GetExtraLong())
    {
        case NodeApi::CURL_REGISTER:
            Log(wxString::Format(wxT("Register: %d %s"), event.GetInt(), event.GetString().c_str()));
            if(event.GetInt() == 201)
            {
                m_nRegisterState = NodeApi::Get().Register();
                if(m_nRegisterState == NodeApi::REG_DONE)
                {//node finished registering
                    m_timerHeartbeat.Start(5000);

                    //let's also do a query here
                    NodeApi::Get().Query("nodes");
                }
                else if(m_nRegisterState == NodeApi::REG_FAILED)
                {
                    Log("No registration nodes found. Browse again in 30 seconds");
                    m_timerHeartbeat.Start(30000, true);
                }
            }
            else if(event.GetInt() == 200)
            {   //node already exists delete it.
                m_nRegisterState = NodeApi::Get().Unregister();
            }
            break;
        case NodeApi::CURL_DELETE:
            Log(wxString::Format(wxT("Delete: %d %s"), event.GetInt(), event.GetString().c_str()));
            if(event.GetInt() == 204)
            {
                m_nRegisterState = NodeApi::Get().Unregister();
            }
            break;
        case NodeApi::CURL_HEARTBEAT:
            if(event.GetInt() != 200)
            {   //SOMETHING HAS GONE WRONG
                Log(wxString::Format(wxT("Heartbeat: %d %s"), event.GetInt(), event.GetString().c_str()));
                //We need to try and register again - or go peer-to-peer
                Log("Registration node gone. Browse again in 30 seconds");
                m_nRegisterState = NodeApi::REG_FAILED;
                m_timerHeartbeat.Start(30000, true);
            }
            break;
        case NodeApi::CURL_QUERY:
            Log(wxString::Format(wxT("Query: %d %s"), event.GetInt(), event.GetString().c_str()));
            break;
    }
}



void wxnmosDialog::OnBrowserFinished(wxCommandEvent& event)
{
    Log(wxT("Browser - finished"));
}


void wxnmosDialog::OnLog(wxCommandEvent& event)
{
    Log(event.GetString());
}

void wxnmosDialog::OntimerHeartbeatTrigger(wxTimerEvent& event)
{

    if(m_nRegisterState == NodeApi::REG_DONE)
    {
        NodeApi::Get().RegistrationHeartbeat();
    }
    else
    {
        Log::Get(Log::DEBUG) << "Ontimer: " << m_nRegisterState << std::endl;
        NodeApi::Get().BrowseForRegistrationNode(new wxBrowserEvent(this));
    }
}

void wxnmosDialog::OnClose(wxCloseEvent& event)
{
//    if(m_nRegisterState != NodeApi::REG_START)
//    {
//        NodeApi::Get().Unregister();
//        event.Veto();
//    }
//    else
    {
        event.Skip();
    }
}
