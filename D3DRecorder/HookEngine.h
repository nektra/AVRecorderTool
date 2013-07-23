#pragma once
#include "stdafx.h"

#ifdef _M_IX86
#define PLUGIN_DLLNAME L"\\D3DRecorderPlugin.dll"
#define AUDIOSESSHOST_DLLNAME L"\\AudioSessionHost.dll"
#define DEVIAREPTR long
#elif defined _M_X64
#define PLUGIN_DLLNAME L"\\D3DRecorderPlugin64.dll"
#define AUDIOSESSHOST_DLLNAME L"\\AudioSessionHost64.dll"
#define DEVIAREPTR __int64
#endif

class CSpyMgrEventSink;

class CHookEngine
{
	Deviare2::INktSpyMgrPtr m_pSpyMgr;
	Deviare2::INktProcessPtr m_pTargetProc;	
	DWORD				m_dwCookie;
	_variant_t			m_vaContinueEvent;
	CSpyMgrEventSink*	m_pSink;
	HWND				m_hParent;
	CWinThread*			m_pipeThread;

	bool				m_createDeviceHookReady;

	std::map<CString, DWORD_PTR> m_methodAddrMap;
	std::set<Deviare2::INktHookPtr> m_hookSet;

	void GetDllHandlerName(wchar_t* buf, size_t cchBuf);
	void GetAudioSessionHostDllName(wchar_t* buf, size_t cchBuf);
	
	BOOL HookInterfaceMethod(Deviare2::INktProcessPtr proc, 
		DEVIAREPTR pIF, 
		int idx, 
		const wchar_t* fqname,
		const wchar_t* handlerDll);

public:
	CHookEngine(HWND hDlg = NULL);
	~CHookEngine(void);

	BOOL Initialize();
	BOOL LoadLicenseKey(char**);
	BOOL LaunchProcess(const CString& fn);
	BOOL CreatePipeServer(HWND hwndParentNotify);
	void TerminatePipeServer();
	BOOL LoadCustomDll();
	BOOL HookD3D9Creation();
	BOOL HookAudioSession();
	void ResumeProcess();
	void UnhookAll();
	void HookRemoteMethodAddress(const CString&, DWORD_PTR pAddr);
	void SetParentDialog (HWND hWnd) { m_hParent = hWnd; }

	// Received from event sink

	void OnCall_Direct3DCreate9(__in Deviare2::INktHook *Hook, __in Deviare2::INktProcess *proc,
										 __in Deviare2::INktHookCallInfo *callInfo);

	void OnCall_IDirect3D9_CreateDevice(__in Deviare2::INktHook *Hook, __in Deviare2::INktProcess *proc,
												__in Deviare2::INktHookCallInfo *callInfo);

	void OnProcessTerminated(Deviare2::INktProcess* p);
};

