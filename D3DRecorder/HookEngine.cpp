#include "stdafx.h"
#include "SpyMgrEventSink.h"
#include "HookEngine.h"
#include "PipeServer.h"
#include "../Common/utils.h"
#include "../Common/wm.h"

using namespace Deviare2;

CHookEngine::CHookEngine(HWND hDlg)  : m_hParent(hDlg),
	m_dwCookie(0),
	m_pTargetProc(0),
	m_pipeThread(NULL),
	m_createDeviceHookReady(false)
{ 
}

BOOL CHookEngine::LoadLicenseKey(char** ppLicenseBuf)
{
	wchar_t dir[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, dir);

	try
	{		
		CFile licenseFile(L"license.txt", CFile::modeRead);
		const size_t cch = licenseFile.GetLength();

		*ppLicenseBuf = new char[cch+1];

		licenseFile.Read(*ppLicenseBuf, (UINT)cch+1);
		return TRUE;
	}
	catch (CFileException* e)
	{		
		wchar_t errData[400];
		e->GetErrorMessage(errData, 400, 0);
		d_printf(L"%s Exception loading license file: %s\\%s \n", __FUNCTIONW__,dir,  errData);
		return FALSE;
	}
	
}

BOOL CHookEngine::Initialize()
{
	HRESULT hr;

	if (SUCCEEDED(hr = m_pSpyMgr.CreateInstance(__uuidof(NktSpyMgr))))
	{
		char* szLicense;
		BOOL b = LoadLicenseKey(&szLicense);

		if (!b)
		{
			d_printf(L"Cannot open license file, loading without Deviare license.\n");
		}
		else
		{
			d_printf(L"Using license from file\n");
			m_pSpyMgr->put_LicenseKey(_bstr_t(szLicense));
			delete[] szLicense;
		}
		
		long ret;
		hr = m_pSpyMgr->Initialize(&ret);
		if (SUCCEEDED(hr) && ret == 0)
		{
			m_pSink = new CSpyMgrEventSink(this);

			TRACE("HookEngine successfully initialized.\n");			
			return TRUE;
		}
		else
		{
			TRACE("HookEngine initialization error. HR=0x%08x RC=%d\n", hr, ret);
		}
	}
	else
	{
		TRACE("Cannot instance SpyMgr. HR=0x%08x\n",hr);
	}

	return FALSE;
}
BOOL CHookEngine::CreatePipeServer(HWND hwndParentNotify)
{
	InterlockedExchange(&g_killServer, 0);
	m_pipeThread = AfxBeginThread(CPipeServer::threadProc, (LPVOID) hwndParentNotify);
	return (m_pipeThread != NULL);
}

void CHookEngine::TerminatePipeServer()
{
	InterlockedExchange(&g_killServer, 1);

	if (m_pipeThread)
	{
		WaitForSingleObject(m_pipeThread->m_hThread, INFINITE);
	}
}

BOOL CHookEngine::LaunchProcess(const CString& fn)
{
	ASSERT(m_pSpyMgr);

	wchar_t szDir[MAX_PATH];
	StringCchCopy(szDir, MAX_PATH, fn);
	PathRemoveFileSpec(szDir);

	wchar_t szCurDir[MAX_PATH];

	GetCurrentDirectory(MAX_PATH, szCurDir);
	SetCurrentDirectory(szDir);

	HRESULT hr = m_pSpyMgr->__CreateProcess(_bstr_t(fn), VARIANT_TRUE, 
		&m_vaContinueEvent, &m_pTargetProc);

	if (FAILED(hr))
	{
		TRACE("Process Launch failed for target binary: %s. HR=0x%08x\n",fn, hr);
	}
	else
	{
		// Keep informaed for events
		//
		SetCurrentDirectory(szCurDir);

		ASSERT(m_pSink);

		if (!m_dwCookie)
		{
			AfxConnectionAdvise(m_pSpyMgr, DIID_DNktSpyMgrEvents, m_pSink->GetIDispatch(FALSE), FALSE, &m_dwCookie);
		}
	}

	return SUCCEEDED(hr);
}

void CHookEngine::GetDllHandlerName(wchar_t* buf, size_t cchBuf)
{
	GetModuleFileName(AfxGetInstanceHandle(), buf, (DWORD) cchBuf);
	PathRemoveFileSpec(buf);
	StringCchCat(buf, cchBuf, PLUGIN_DLLNAME);
}

void CHookEngine::ResumeProcess()
{
	m_pSpyMgr->ResumeProcess(m_pTargetProc, m_vaContinueEvent);					
}

BOOL CHookEngine::HookD3D9Creation()
{
	INktHookPtr pD3DCreate9Hook;

	if (SUCCEEDED(m_pSpyMgr->CreateHook(_variant_t("d3d9.dll!Direct3DCreate9"), eNktHookFlags::flgOnlyPostCall, 
		&pD3DCreate9Hook)))
	{
		long PID;
		m_pTargetProc->get_Id(&PID);
		pD3DCreate9Hook->Attach(_variant_t(PID), FALSE);
		pD3DCreate9Hook->Hook(FALSE);

		m_hookSet.insert(pD3DCreate9Hook);

		return TRUE;
	}

	return FALSE;
}

void CHookEngine::OnCall_Direct3DCreate9(__in Deviare2::INktHook *Hook, __in Deviare2::INktProcess *proc,
										 __in Deviare2::INktHookCallInfo *callInfo)
{
	d_printf(L"%s\n",__FUNCTIONW__);

	DEVIAREPTR pVal;

	INktParamPtr pResult;
	callInfo->Result(&pResult);	
	pResult->get_PointerVal(&pVal);

	// We got a IDirect3D9 ptr. Now hook CreateDevice by Indexing vtable.
	//

	INktProcessMemoryPtr pMem;
	proc->Memory(&pMem);

	DEVIAREPTR lpD3D9_VT;
	pMem->get_SSizeTVal(pVal, &lpD3D9_VT);

	if (lpD3D9_VT)
	{
		DEVIAREPTR lpMethodAddr; 
		pMem->get_SSizeTVal((DEVIAREPTR)
			(lpD3D9_VT + VTI_IDirect3D9::CreateDevice * sizeof(DEVIAREPTR)), &lpMethodAddr);

		if (lpMethodAddr)
		{			
			INktHookPtr pHook;

			m_pSpyMgr->CreateHookForAddress(lpMethodAddr, 
				_bstr_t(L"IDirect3D9::CreateDevice"), 
				eNktHookFlags::flgOnlyPostCall,
				&pHook);

			long PID;
			proc->get_Id(&PID);
			_variant_t vPID(PID);

			wchar_t handlerDllName[MAX_PATH];
			GetDllHandlerName(handlerDllName, MAX_PATH);			

			// Add a remote handler for initialization.			
			pHook->AddCustomHandler(handlerDllName, eNktHookCustomHandlerFlags::flgChDisableExceptionChecks, L"");

			pHook->Hook(TRUE);
			pHook->Attach(vPID, TRUE);			

			m_hookSet.insert(pHook);
		}
	}
}

void CHookEngine::OnCall_IDirect3D9_CreateDevice(__in Deviare2::INktHook *Hook, __in Deviare2::INktProcess *proc,
												 __in Deviare2::INktHookCallInfo *callInfo)
{
	d_printf(L"%s\n",__FUNCTIONW__);



	INktParamsEnumPtr pParams;
	callInfo->Params(&pParams);

	//
	// Ensure hooking for hardware-rasterizer ONLY
	//

	INktParamPtr pParamDevType;
	pParams->GetAt(2, &pParamDevType);
	long devType;
	pParamDevType->get_LongVal(&devType);

	if (devType != D3DDEVTYPE_HAL)
	{
		d_printf(L"WARNING! %s: Ignoring hooks for device type D3DDEVTYPE=%d\n",__FUNCTIONW__,devType);
		return;
	}

	//
	// Get IDirect3DDevice9 ptr and hook Reset/EndScene/Present by Indexing vtable.
	// 
	INktParamPtr pParamD3DDevice;
	pParams->GetAt(6, &pParamD3DDevice);	
	pParamD3DDevice->Evaluate(&pParamD3DDevice);

	DEVIAREPTR pD3DDevice;
	pParamD3DDevice->get_PointerVal(&pD3DDevice);

	INktProcessMemoryPtr pMem;
	proc->Memory(&pMem);

	DEVIAREPTR lpD3DDevice9_VT;
	pMem->get_SSizeTVal(pD3DDevice, &lpD3DDevice9_VT);

	//
	// Ensure we  hook this VTable addresses once
	//

	if (!m_createDeviceHookReady)
	{
		if (lpD3DDevice9_VT)
		{		
			wchar_t handlerDllName[MAX_PATH];
			GetDllHandlerName(handlerDllName, MAX_PATH);				
			HookInterfaceMethod(proc, lpD3DDevice9_VT, VTI_IDirect3D9Device::Release, L"IDirect3DDevice9::Release", handlerDllName);
			HookInterfaceMethod(proc, lpD3DDevice9_VT, VTI_IDirect3D9Device::EndScene, L"IDirect3DDevice9::EndScene", handlerDllName);
			HookInterfaceMethod(proc, lpD3DDevice9_VT, VTI_IDirect3D9Device::Present, L"IDirect3DDevice9::Present", handlerDllName);
			HookInterfaceMethod(proc, lpD3DDevice9_VT, VTI_IDirect3D9Device::Reset, L"IDirect3DDevice9::Reset", handlerDllName);

			m_createDeviceHookReady = true;
		}
	}
	else
	{
		d_printf(L"Already hooked IDirect3DDevice9 method addresses.\n");
	}
}

BOOL CHookEngine::HookInterfaceMethod(Deviare2::INktProcessPtr proc, 
									  DEVIAREPTR pIF, 
									  int idx, 
									  const wchar_t* fqname,
									  const wchar_t* handlerDll)
{
	INktProcessMemoryPtr pMem;
	proc->Memory(&pMem);

	DEVIAREPTR lpMethodAddr;
	HRESULT hr = E_FAIL;

	pMem->get_SSizeTVal((DEVIAREPTR)(pIF + idx * sizeof(DEVIAREPTR)), &lpMethodAddr);

	if (lpMethodAddr)
	{
		INktHookPtr pHook;

		if (SUCCEEDED(hr = m_pSpyMgr->CreateHookForAddress(lpMethodAddr, _bstr_t(fqname), 0, &pHook)))
		{
			if (handlerDll)
			{
				pHook->AddCustomHandler(_bstr_t(handlerDll), 
					eNktHookCustomHandlerFlags::flgChDisableExceptionChecks, L"");		
			}

			long vPID;
			proc->get_Id(&vPID);

			pHook->Hook(TRUE);
			pHook->Attach(_variant_t(vPID), TRUE);

			m_hookSet.insert(pHook);
		}
		else
		{
			d_printf(L"CreateHookForAddress for method %s failed HR=0x%08x.", fqname, hr);
		}
	}

	return SUCCEEDED(hr);
}

void CHookEngine::UnhookAll()
{
	if (m_pTargetProc)
	{
		long pid;
		m_pTargetProc->get_Id(&pid);

		for (auto it = m_hookSet.begin(); it != m_hookSet.end(); it++)
		{
			CComBSTR fName;
			(*it)->get_FunctionName(&fName);

			d_printf(L"UnhookAll>  Unhooking function %s...\n", fName);

			(*it)->Detach(_variant_t(pid), TRUE);
			(*it)->Unhook(TRUE);
			m_pSpyMgr->DestroyHook(*it);			
		}

		m_hookSet.clear();
	}
}

CHookEngine::~CHookEngine(void)
{	
	TerminatePipeServer();
	UnhookAll();

	AfxConnectionUnadvise(m_pSpyMgr, DIID_DNktSpyMgrEvents, m_pSink->GetIDispatch(FALSE), FALSE, m_dwCookie);
	m_dwCookie = 0;
	delete m_pSink;
}

void CHookEngine::OnProcessTerminated(Deviare2::INktProcess* proc)
{
	long thisPID, runningPID;

	proc->get_Id(&thisPID);
	m_pTargetProc->get_Id(&runningPID);

	if (thisPID == runningPID)
	{
		TerminatePipeServer();
		UnhookAll();
		m_createDeviceHookReady = false;

		SendMessage(m_hParent, WM_PROCESSTERMINATED, 0 ,0);


	}
}
