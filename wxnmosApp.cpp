/***************************************************************
 * Name:      wxnmosApp.cpp
 * Purpose:   Code for Application Class
 * Author:    Matthew Martin (martim01@outlook.com)
 * Created:   2018-12-17
 * Copyright: Matthew Martin ()
 * License:
 **************************************************************/

#include "wxnmosApp.h"

//(*AppHeaders
#include "wxnmosMain.h"
#include <wx/image.h>
//*)

IMPLEMENT_APP(wxnmosApp);

bool wxnmosApp::OnInit()
{
    //(*AppInitialize
    bool wxsOK = true;
    wxInitAllImageHandlers();
    if ( wxsOK )
    {
    	wxnmosDialog Dlg(0);
    	SetTopWindow(&Dlg);
    	Dlg.ShowModal();
    	wxsOK = false;
    }
    //*)
    return wxsOK;

}
