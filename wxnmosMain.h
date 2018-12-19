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
#include <wx/listbox.h>
#include <wx/dialog.h>
#include <wx/timer.h>
//*)

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
        //*)

        //(*Identifiers(wxnmosDialog)
        static const long ID_LISTBOX1;
        static const long ID_TIMER1;
        //*)

        //(*Declarations(wxnmosDialog)
        wxListBox* m_plbxLog;
        wxBoxSizer* BoxSizer2;
        wxBoxSizer* BoxSizer1;
        wxTimer m_timerHeartbeat;
        //*)

        void OnBrowserResolved(wxCommandEvent& event);
        void OnBrowserAllForNow(wxCommandEvent& event);
        void OnBrowserFinished(wxCommandEvent& event);
        void OnRegistrationError(wxCommandEvent& event);
        void OnLog(wxCommandEvent& event);
        void OnCurlDone(wxCommandEvent& event);

        void Log(const wxString& sLog);

        int m_nRegisterState;

        DECLARE_EVENT_TABLE()
};

#endif // WXNMOSMAIN_H
