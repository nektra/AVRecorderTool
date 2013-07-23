#include "stdafx.h"
#include "D3DRecorderPlugin.h"
#include "RecorderEngine.h"
#include "../Common/utils.h"
#include "../Common/ipc.h"
#include "ParamHelper.h"
#include "KeyboardHook.h"
#include "CommandQueue.h"

using namespace Deviare2;

#pragma comment ( lib, "d3dx9.lib" )

// Global
//

ULONG g_resourceRefCount = 0;
CRecorderEngine *  g_pRecEng = NULL;
CKeyboardHook * g_pKeybHook = NULL;
CCommandQueue * g_pCmdQueue = NULL;
WNDPROC g_oldWndProc = NULL;
HWND g_subclassWnd = NULL;

double t0 = 0.0f; 

// 
// Subclassed Window procedure
//
LRESULT CALLBACK D3DWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
	case WM_CLOSE:

		d_printf(L"D3DWindowProc WM_CLOSE\n");
		g_pRecEng->DetachFromDevice();
		break;

	case WM_NCDESTROY:

		d_printf(L"D3DWindowProc WM_NCDESTROY\n");
		SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)g_oldWndProc);
		break;
	}

	return CallWindowProc(g_oldWndProc, hwnd, msg, wparam, lparam);
}

///////////////////////////////////////////////////////////////////////////////

HRESULT WINAPI OnLoad()
{
	d_printf(__FUNCTIONW__);

	g_pRecEng = new CRecorderEngine();
	g_pCmdQueue = new CCommandQueue;
	g_pKeybHook = new CKeyboardHook(g_pCmdQueue);

	return (g_pRecEng->InitIPC() ? S_OK : E_FAIL);
}

VOID WINAPI OnUnload()
{
	d_printf(__FUNCTIONW__);

	if (g_pRecEng != NULL)
		delete g_pRecEng;

	if (g_pCmdQueue != NULL)
		delete g_pCmdQueue;

	if (g_pKeybHook != NULL)
		delete g_pKeybHook;	
}

HRESULT WINAPI OnHookAdded(__in Deviare2::INktHookInfo *lpHookInfo, __in DWORD dwChainIndex,
						   __in LPCWSTR szParametersW)
{
	CComBSTR fnName;
	lpHookInfo->get_FunctionName(&fnName);
	d_printf(L"D3DRecorderPlugin.dll: OnHookAdded for: %s\n", fnName);
	return S_OK;
}

VOID WINAPI OnHookRemoved(__in INktHookInfo *lpHookInfo, __in DWORD dwChainIndex)
{	
	CComBSTR fnName;
	lpHookInfo->get_FunctionName(&fnName);
	d_printf(L"D3DRecorderPlugin.dll: OnHookRemoved for: %s\n", fnName);
}

HRESULT WINAPI OnFunctionCall(__in INktHookInfo *lpHookInfo, __in DWORD dwChainIndex,
							  __in INktHookCallInfoPlugin *lpHookCallInfoPlugin)
{
	CComBSTR fnName;
	lpHookInfo->get_FunctionName(&fnName);

	VARIANT_BOOL bIsPre;
	lpHookCallInfoPlugin->get_IsPreCall(&bIsPre);

	//d_printf(L"D3DRecorderPlugin.dll: OnFunctionCall for: %s (%s)  \n", fnName, bIsPre ? L"PRE" : L"POST");

	if (fnName == "IDirect3D9::CreateDevice")
	{
		DWORD dt = GetLongParam(lpHookCallInfoPlugin, 2);
		HWND hWndFocus = static_cast<HWND>(GetHandleParam(lpHookCallInfoPlugin, 3));
		D3DPRESENT_PARAMETERS* pPresentParams = static_cast<D3DPRESENT_PARAMETERS*>(GetPointerParam(lpHookCallInfoPlugin, 5));

		if (dt == D3DDEVTYPE_HAL) // We care only about hardware rasterizers.
		{
			IDirect3DDevice9* pD3DDev9 = GetDeviceFromParamInfo(lpHookCallInfoPlugin, 6, true);

			d_printf(L"IDirect3D9::CreateDevice successfully returned interface ptr = 0x%p\n", pD3DDev9);

			if (pD3DDev9)
			{
				HWND hSubclass =  pPresentParams->hDeviceWindow == NULL ? hWndFocus : pPresentParams->hDeviceWindow;

				if (!hSubclass)
				{
					d_printf(L"Cannot determine device window handle!!!! -- skipping subclass\n");
				}
				else
				{
					if (g_subclassWnd != hSubclass)
					{
						g_subclassWnd = hSubclass;
						d_printf(L"Subclassing window HWND=%d\n", hSubclass);					
						g_oldWndProc = (WNDPROC) SetWindowLongPtr(hSubclass, GWLP_WNDPROC, (LONG_PTR)  D3DWindowProc);					
					}
				}

				Handle_IDirect3D9_CreateDevice(pD3DDev9);
			}
			else
			{
				d_printf(L"IDirect3D9::CreateDevice returned NULL interface pointer!\n");
			}
		}
		else
		{
			d_printf(L"WARNING!! Ignoring IDirect3D9::CreateDevice handler for D3DDEVTYPE = %d\n", dt);
		}		
	}
	else
	{
		//
		// Filter out those function calls
		//
		lpHookCallInfoPlugin->FilterSpyMgrEvent();

		if (fnName == "IDirect3DDevice9::Release")
		{
			if (bIsPre == VARIANT_TRUE)
			{
				//...
			}
			else
			{
				Handle_IDirect3DDevice9_Release_POST(GetUlongRetVal(lpHookCallInfoPlugin), 
					GetDeviceFromParamInfo(lpHookCallInfoPlugin));	
			}

			return S_OK;
		}	
		else	
			if (fnName == "IDirect3DDevice9::EndScene")
			{		
				if (bIsPre == VARIANT_TRUE)
				{
					Handle_IDirect3DDevice9_EndScene_PRE(GetDeviceFromParamInfo(lpHookCallInfoPlugin));
				}
				else
				{
					Handle_IDirect3DDevice9_EndScene_POST(GetDeviceFromParamInfo(lpHookCallInfoPlugin));
				}
			}
			else if (fnName == "IDirect3DDevice9::Present")
			{
				if (bIsPre == VARIANT_TRUE)
				{
					Handle_IDirect3DDevice9_Present_PRE(GetDeviceFromParamInfo(lpHookCallInfoPlugin));
				}
				else
				{
					Handle_IDirect3DDevice9_Present_POST(GetDeviceFromParamInfo(lpHookCallInfoPlugin));
				}
			}
			else if (fnName == "IDirect3DDevice9::Reset")
			{		
				if (bIsPre == VARIANT_TRUE)
				{
					Handle_IDirect3DDevice9_Reset_PRE(GetDeviceFromParamInfo(lpHookCallInfoPlugin));
				}
				else
				{
					Handle_IDirect3DDevice9_Reset_POST(GetSizeTRetVal(lpHookCallInfoPlugin), 
						GetDeviceFromParamInfo(lpHookCallInfoPlugin));
				}
			}
			else
			{
				d_printf(L"d3dRecorderPlugin Error: I dont know to handle call for %s\n -- returning E_FAIL", fnName);
				return E_FAIL;
			}
	}

	return S_OK;
}


void Handle_IDirect3D9_CreateDevice(IDirect3DDevice9* pID3DDev9)
{

	if (g_pRecEng->IsDeviceAttached())
	{
		d_printf(L"Already hooked another D3D device, releasing...");
		g_pRecEng->DetachFromDevice();
	}

	// Check number of refs we're adding up.
	//

	d_printf(L"%s\n",__FUNCTIONW__);

	pID3DDev9->AddRef();
	ULONG rc0 = pID3DDev9->Release();

	g_pKeybHook->Initialize(pID3DDev9);
	g_pRecEng->AttachToDevice(pID3DDev9);
	g_pRecEng->InitResources();	

	pID3DDev9->AddRef();
	ULONG rc1 = pID3DDev9->Release();

	d_printf(L"Before CreateDevice RC=%d, after RC=%d\n", rc0, rc1);

	g_resourceRefCount = rc1-rc0;

	g_pRecEng->ConsoleWriteLine(L"Nektra D3D Tool Ready.");	
}

void Handle_IDirect3DDevice9_Present_POST(IDirect3DDevice9* pID3DDev9)
{	
	UNREFERENCED_PARAMETER(pID3DDev9);
}

void Handle_IDirect3DDevice9_Present_PRE(IDirect3DDevice9* pID3DDev9)
{
	assert(g_pRecEng);
	
	if (g_pRecEng->IsDeviceAttached())
	{
		CCommand cmd = g_pCmdQueue->Peek();

		switch (cmd.id)
		{
		case CMD_TAKE_SCREENSHOT:

			g_pCmdQueue->Dequeue();

			{
				wchar_t fname[MAX_PATH];
				wchar_t outputDir[MAX_PATH];
				GetOutputDir(outputDir,MAX_PATH);

				g_pRecEng->MakeSequentialFilename(outputDir, L"PIC", fname, MAX_PATH);
				StringCchCat(fname, MAX_PATH, L".bmp");

				g_pRecEng->TakeScreenshot(fname, IMAGEFORMAT_BMP);

				g_pRecEng->ConsoleWriteLine(L"Screenshot saved to: %s", fname);
			}

			break;

		case CMD_VIDEORECORD_START:
			g_pCmdQueue->Dequeue();

			if (!g_pRecEng->IsVideoRecording())
			{
				wchar_t fname[MAX_PATH];
				wchar_t outputDir[MAX_PATH];
				GetOutputDir(outputDir,MAX_PATH);

				g_pRecEng->MakeSequentialFilename(outputDir, L"VID", fname, MAX_PATH);
				StringCchCat(fname, MAX_PATH, L".avi");

				g_pRecEng->VideoRecorderInit(fname);
				g_pRecEng->ConsoleWriteLine(L"Video recording started to: %s", fname);
			}
			else
			{
				g_pRecEng->ConsoleWriteLine(L"Video already recording ... ");

			}
			break;

		case CMD_VIDEORECORD_STOP:

			g_pCmdQueue->Dequeue();

			if (g_pRecEng->IsVideoRecording())
			{
				int cFrames = g_pRecEng->VideoCurrentFrameCount();
				g_pCmdQueue->Dequeue();
				g_pRecEng->VideoRecorderStop();

				g_pRecEng->ConsoleWriteLine(L"Video recording stopped. Frame count: ", cFrames);
			}
			else
			{
				g_pRecEng->ConsoleWriteLine(L"Can't stop. Video is not recording.");
			}

			break;
		}

		// If recording, grab current frame.
		//
		if (g_pRecEng->IsVideoRecording())
		{
			if (g_pRecEng->VideoGrabFrame() == FALSE)
			{
				g_pRecEng->ConsoleWriteLine(L"(!) Video recording stopped due to error.");
				g_pRecEng->VideoRecorderStop();
			}
		}

		// Update fps
		//

		g_pRecEng->UpdateFrameRate();
	}

}

void Handle_IDirect3DDevice9_EndScene_PRE(IDirect3DDevice9* pID3DDev9)
{
	assert(g_pRecEng);
	
	if (g_pRecEng->IsDeviceAttached())
	{
		if (pID3DDev9->TestCooperativeLevel() == D3D_OK)
		{		
			g_pRecEng->Update();
		}
	}
}

void Handle_IDirect3DDevice9_EndScene_POST(IDirect3DDevice9* pID3DDev9)
{
	UNREFERENCED_PARAMETER(pID3DDev9);

	/*assert(g_pRecEng);
	assert(pID3DDev9);*/
}

void Handle_IDirect3DDevice9_Reset_PRE(IDirect3DDevice9* pID3DDev9)
{
	d_printf(L"%s\n",__FUNCTIONW__);

	assert(g_pRecEng);

	if (g_pRecEng->IsDeviceAttached())
	{
		if (pID3DDev9)
		{		
			if (g_pRecEng->IsVideoRecording())
			{
				g_pRecEng->ConsoleWriteLine(L"(!) Video recording stopped due to device reset.");
				g_pRecEng->VideoRecorderStop();
			}

			g_pRecEng->OnDeviceReset();
		}
	}
}

void Handle_IDirect3DDevice9_Reset_POST(DEVIARERETVAL rv, IDirect3DDevice9* pID3DDev9)
{
	d_printf(L"%s\n",__FUNCTIONW__);

	assert(g_pRecEng);

	if (g_pRecEng->IsDeviceAttached())
	{

		//Check for lost device
		//

		if (pID3DDev9)
		{
			if (g_pRecEng->IsVideoRecording())
			{
				g_pRecEng->ConsoleWriteLine(L"(!) Video recording stopped due to device reset.");
				g_pRecEng->VideoRecorderStop();
			}

			if (rv == D3D_OK) 
			{
				//  Reset succeeded.
				g_pRecEng->AttachToDevice(pID3DDev9);
				g_pRecEng->InitResources();
			}

			else if (rv == D3DERR_DEVICELOST)
			{
				g_pRecEng->OnDeviceLost();
			}
		}
	}
}

void Handle_IDirect3DDevice9_Release_PRE(ULONG rv, IDirect3DDevice9* pID3DDev9)
{
	// dummy.
}

void Handle_IDirect3DDevice9_Release_POST(ULONG rv, IDirect3DDevice9* pID3DDev9)
{
	// dummy.
}

