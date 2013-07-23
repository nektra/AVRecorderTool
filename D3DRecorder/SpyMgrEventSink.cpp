// SpyMgrEventSink.cpp : implementation file
//

#include "stdafx.h"
#include "D3DRecorder.h"
#include "SpyMgrEventSink.h"
#include "HookEngine.h"

// CSpyMgrEventSink

IMPLEMENT_DYNAMIC(CSpyMgrEventSink, CCmdTarget)


CSpyMgrEventSink::CSpyMgrEventSink(CHookEngine* pHookEng)
{
	ASSERT(pHookEng);

	m_pHookEng = pHookEng;

	EnableAutomation();
}

CSpyMgrEventSink::~CSpyMgrEventSink()
{
}


void CSpyMgrEventSink::OnFinalRelease()
{	
	CCmdTarget::OnFinalRelease();
}


BEGIN_MESSAGE_MAP(CSpyMgrEventSink, CCmdTarget)
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CSpyMgrEventSink, CCmdTarget)
	DISP_FUNCTION_ID(CSpyMgrEventSink,"OnFunctionCalled",dispidNktSpyMgrEventsOnFunctionCalled,OnFunctionCalled,VT_EMPTY,VTS_UNKNOWN VTS_UNKNOWN VTS_UNKNOWN)
	DISP_FUNCTION_ID(CSpyMgrEventSink,"OnProcessTerminated",dispidNktSpyMgrEventsOnProcessTerminated,OnProcessTerminated,VT_EMPTY,VTS_UNKNOWN)
END_DISPATCH_MAP()

BEGIN_INTERFACE_MAP(CSpyMgrEventSink, CCmdTarget)
	INTERFACE_PART(CSpyMgrEventSink, Deviare2::DIID_DNktSpyMgrEvents, Dispatch)
END_INTERFACE_MAP()


// CSpyMgrEventSink message handlers

void CSpyMgrEventSink::OnFunctionCalled (__in Deviare2::INktHook *Hook, __in Deviare2::INktProcess *proc,
										 __in Deviare2::INktHookCallInfo *callInfo)
{
	BSTR fnName;
	Hook->get_FunctionName(&fnName);

	if (_wcsicmp(fnName, L"d3d9.dll!Direct3DCreate9") == 0)
	{
		m_pHookEng->OnCall_Direct3DCreate9(Hook, proc, callInfo);
	}
	else if (_wcsicmp(fnName, L"IDirect3D9::CreateDevice") == 0)
	{
		m_pHookEng->OnCall_IDirect3D9_CreateDevice(Hook, proc, callInfo);
	}	
}

void CSpyMgrEventSink::OnProcessTerminated(__in Deviare2::INktProcess* p)
{	
	m_pHookEng->OnProcessTerminated(p);
}
