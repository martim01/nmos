/***************************************************************
 * Name:      wxnmosMain.h
 * Purpose:   Defines Application Frame
 * Author:    Matthew Martin (martim01@outlook.com)
 * Created:   2018-12-17
 * Copyright: Matthew Martin ()
 * License:
 **************************************************************/

#ifndef WXNMOSMAIN_H
#define WXNMOSMAIN_H

//(*Headers(wxnmosDialog)
#include <wx/sizer.h>
#include <wx/checkbox.h>
#include <wx/listbox.h>
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/timer.h>
//*)

class wxNmosEvent;

class wxnmosDialog: public wxDialog
{
    public:

        wxnmosDialog(wxWindow* parent,wxWindowID id = -1);
        virtual ~wxnmosDialog();

    private:

        //(*Handlers(wxnmosDialog)
        void OnQuit(wxCommandEvent& event);
        void OnAbout(wxCommandEvent& event);
        void OntimerHeartbeatTrigger(wxTimerEvent& event);
        void OnClose(wxCloseEvent& event);
        void OnbtnQueryNodesClick(wxCommandEvent& event);
        void OnchbxScrollClick(wxCommandEvent& event);
        //*)

        //(*Identifiers(wxnmosDialog)
        static const long ID_BUTTON1;
        static const long ID_CHECKBOX1;
        static const long ID_LISTBOX1;
        static const long ID_TIMER1;
        //*)

        //(*Declarations(wxnmosDialog)
        wxListBox* m_plbxLog;
        wxCheckBox* m_pchbxScroll;
        wxBoxSizer* BoxSizer2;
        wxButton* m_pbtnQueryNodes;
        wxBoxSizer* BoxSizer1;
        wxTimer m_timerHeartbeat;
        //*)

        void OnLog(wxCommandEvent& event);

        void OnBrowserResolved(wxNmosEvent& event);
        void OnBrowserAllForNow(wxNmosEvent& event);
        void OnBrowserFinished(wxNmosEvent& event);
        void OnRegistrationError(wxNmosEvent& event);

        void OnCurlDone(wxNmosEvent& event);
        void OnTarget(wxNmosEvent& event);
        void OnPatchSender(wxNmosEvent& event);
        void OnPatchReceiver(wxNmosEvent& event);
        void OnActivateSender(wxNmosEvent& event);
        void OnActivateReceiver(wxNmosEvent& event);

        void Log(const wxString& sLog);

        int m_nRegisterState;
        bool m_bScroll;

        DECLARE_EVENT_TABLE()
};

#endif // WXNMOSMAIN_H
