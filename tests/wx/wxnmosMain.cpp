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
#include "wxlogoutput.h"
#include "wxeventposter.h"

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
const long wxnmosDialog::ID_BUTTON1 = wxNewId();
const long wxnmosDialog::ID_CHECKBOX1 = wxNewId();
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
    wxBoxSizer* BoxSizer3;

    Create(parent, id, _("wxWidgets app"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
    BoxSizer1 = new wxBoxSizer(wxVERTICAL);
    BoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    m_pbtnQueryNodes = new wxButton(this, ID_BUTTON1, _("Query Nodes"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    BoxSizer3->Add(m_pbtnQueryNodes, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    m_pchbxScroll = new wxCheckBox(this, ID_CHECKBOX1, _("Scroll"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
    m_pchbxScroll->SetValue(true);
    BoxSizer3->Add(m_pchbxScroll, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BoxSizer1->Add(BoxSizer3, 0, wxALL|wxEXPAND, 5);
    BoxSizer2 = new wxBoxSizer(wxVERTICAL);
    m_plbxLog = new wxListBox(this, ID_LISTBOX1, wxDefaultPosition, wxSize(1280,600), 0, 0, 0, wxDefaultValidator, _T("ID_LISTBOX1"));
    BoxSizer2->Add(m_plbxLog, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BoxSizer1->Add(BoxSizer2, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 4);
    SetSizer(BoxSizer1);
    m_timerHeartbeat.SetOwner(this, ID_TIMER1);
    BoxSizer1->Fit(this);
    BoxSizer1->SetSizeHints(this);

    Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&wxnmosDialog::OnbtnQueryNodesClick);
    Connect(ID_CHECKBOX1,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&wxnmosDialog::OnchbxScrollClick);
    Connect(ID_TIMER1,wxEVT_TIMER,(wxObjectEventFunction)&wxnmosDialog::OntimerHeartbeatTrigger);
    Connect(wxID_ANY,wxEVT_CLOSE_WINDOW,(wxObjectEventFunction)&wxnmosDialog::OnClose);
    //*)
    Connect(wxID_ANY, wxEVT_NMOS_MDNS_ALLFORNOW, (wxObjectEventFunction)&wxnmosDialog::OnBrowserAllForNow);
    Connect(wxID_ANY, wxEVT_NMOS_MDNS_FINISHED, (wxObjectEventFunction)&wxnmosDialog::OnBrowserFinished);
    Connect(wxID_ANY, wxEVT_NMOS_MDNS_RESOLVED, (wxObjectEventFunction)&wxnmosDialog::OnBrowserResolved);
    Connect(wxID_ANY, wxEVT_NMOS_LOG, (wxObjectEventFunction)&wxnmosDialog::OnLog);
    Connect(wxID_ANY, wxEVT_NMOS_CURL_DONE, (wxObjectEventFunction)&wxnmosDialog::OnCurlDone);
    Connect(wxID_ANY, wxEVT_NMOS_REG_ERROR, (wxObjectEventFunction)&wxnmosDialog::OnRegistrationError);

    Log::Get().SetOutput(new wxLogOutput(this));

    m_bScroll = true;

    NodeApi::Get().Init(8080, 8081, "host1", "host1");
    NodeApi::Get().GetSelf().AddApiVersion("v1.2");
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

    NodeApi::Get().AddDevice(pDevice);
    NodeApi::Get().AddSource(pSource);
    NodeApi::Get().AddFlow(pFlow);
    NodeApi::Get().AddReceiver(pReceiver);
    NodeApi::Get().AddSender(pSender);
    NodeApi::Get().Commit();

    NodeApi::Get().StartServices(new wxEventPoster(this));

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
    if(m_bScroll)
    {
        m_plbxLog->SetFirstItem(nItem);
    }
}

void wxnmosDialog::OnBrowserResolved(wxCommandEvent& event)
{
    Log(wxT("Registation node resolved"));
}

void wxnmosDialog::OnBrowserAllForNow(wxCommandEvent& event)
{
    Log(wxT("Browser - all for now "));

}

void wxnmosDialog::OnRegistrationError(wxCommandEvent& event)
{
    Log(wxT("Registration error"));
}

void wxnmosDialog::OnCurlDone(wxCommandEvent& event)
{
    switch(event.GetExtraLong())
    {
        case NodeApi::CURL_QUERY:
            for(std::map<std::string, Resource*>::const_iterator itResource = NodeApi::Get().GetQueryResults().GetResourceBegin(); itResource != NodeApi::Get().GetQueryResults().GetResourceEnd(); ++itResource)
            {
                Log(wxString::Format(wxT("QUERY: %s found %s"), wxString::FromAscii(NodeApi::Get().GetQueryResults().GetType().c_str()).c_str(), wxString::FromAscii(itResource->first.c_str()).c_str()));
            }
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
}

void wxnmosDialog::OnClose(wxCloseEvent& event)
{
    NodeApi::Get().StopServices();
    NodeApi::Get().UnregisterSimple();
    event.Skip();
}

void wxnmosDialog::OnbtnQueryNodesClick(wxCommandEvent& event)
{
    NodeApi::Get().Query(NodeApi::NR_NODES, "");
}

void wxnmosDialog::OnchbxScrollClick(wxCommandEvent& event)
{
    m_bScroll = event.IsChecked();
}
