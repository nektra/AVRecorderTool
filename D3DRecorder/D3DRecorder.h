
// D3DRecorder.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols

class CHookEngine;

// CD3DRecorderApp:
// See D3DRecorder.cpp for the implementation of this class
//

#include "../Common/wm.h"
class CD3DRecorderApp : public CWinApp
{
public:
	CD3DRecorderApp();

// Overrides
public:
	virtual BOOL InitInstance();
	
	CHookEngine* GetHookEngine() const { return m_pHookEngine; }

// Implementation

	DECLARE_MESSAGE_MAP()
private:
	CHookEngine* m_pHookEngine;
};

extern CD3DRecorderApp theApp;