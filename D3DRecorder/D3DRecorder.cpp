
// D3DRecorder.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "D3DRecorder.h"
#include "D3DRecorderDlg.h"
#include "HookEngine.h"

using namespace Deviare2;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CD3DRecorderApp

BEGIN_MESSAGE_MAP(CD3DRecorderApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CD3DRecorderApp construction

CD3DRecorderApp::CD3DRecorderApp()
{

}


// The one and only CD3DRecorderApp object

CD3DRecorderApp theApp;


// CD3DRecorderApp initialization

BOOL CD3DRecorderApp::InitInstance()
{
	CoInitializeEx(0,COINIT_MULTITHREADED);

	m_pHookEngine = new CHookEngine;

	if (m_pHookEngine->Initialize())
	{		
		INITCOMMONCONTROLSEX InitCtrls;
		InitCtrls.dwSize = sizeof(InitCtrls);
		InitCtrls.dwICC = ICC_WIN95_CLASSES;
		InitCommonControlsEx(&InitCtrls);

		CWinApp::InitInstance();

		CShellManager *pShellManager = new CShellManager;

		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

		SetRegistryKey(_T("Nektra"));

		CD3DRecorderDlg dlg;
		m_pMainWnd = &dlg;
		INT_PTR nResponse = dlg.DoModal();
		if (nResponse == IDOK)
		{
			// TODO: Place code here to handle when the dialog is
			//  dismissed with OK
		}
		else if (nResponse == IDCANCEL)
		{
			// TODO: Place code here to handle when the dialog is
			//  dismissed with Cancel
		}
		else if (nResponse == -1)
		{
			TRACE(traceAppMsg, 0, "Warning: dialog creation failed, so application is terminating unexpectedly.\n");
			TRACE(traceAppMsg, 0, "Warning: if you are using MFC controls on the dialog, you cannot #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS.\n");
		}

		// Delete the shell manager created above.
		if (pShellManager != NULL)
		{
			delete pShellManager;
		}

	}
	else
		TRACE("Initialization Failed, exiting...\n");

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.

	if (m_pHookEngine)
	{
		delete m_pHookEngine;
	}

	CoUninitialize();

	return FALSE;
}

