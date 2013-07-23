#include "stdafx.h"
#include "CommandQueue.h"
#include "KeyboardHook.h"
#include "../Common/utils.h"

HHOOK CKeyboardHook::_hHook = NULL;
CCommandQueue* CKeyboardHook::_pCmdQueue = NULL;

CKeyboardHook::CKeyboardHook(CCommandQueue* pCmdQ) 
{
	_pCmdQueue = pCmdQ;
	assert(pCmdQ);
}

BOOL CKeyboardHook::Initialize(IDirect3DDevice9* pID3DDev9)
{
	D3DDEVICE_CREATION_PARAMETERS cp;
	if (SUCCEEDED(pID3DDev9->GetCreationParameters(&cp)))
	{
		_hHook = SetWindowsHookEx(WH_KEYBOARD, 
			KeyboardProc, 
			NULL, 
			GetWindowThreadProcessId(cp.hFocusWindow, 0));

		if (_hHook) 
		{
			d_printf(L"d3dRecorderPlugin: Keyboard hook installed.\n");
			return TRUE;
		}
		else
		{
			d_printf(L"d3dRecorderPlugin: Keyboard hook installed.\n");
			return FALSE;
		}
	}

	return FALSE;
}


CKeyboardHook::~CKeyboardHook(void)
{
	d_printf(__FUNCTIONW__);
	UnhookWindowsHookEx(_hHook);
}

LRESULT CALLBACK CKeyboardHook::KeyboardProc(int code, WPARAM wparam, LPARAM lparam)	
{
	assert(_pCmdQueue);

	if (code == HC_ACTION )
	{
		UINT msg = (lparam & 0x80000000) ? WM_KEYUP : WM_KEYDOWN;

		CCommand cmd;
		
		if (msg == WM_KEYUP)
		{
			d_printf(L"%s WM_KEYUP vk=%d\n",__FUNCTIONW__, wparam);

			switch (wparam)
			{
			case VK_DELETE:
				cmd.id = CMD_TAKE_SCREENSHOT;
				_pCmdQueue->Enqueue(cmd);				
				break;

			case VK_F11:
				cmd.id = CMD_VIDEORECORD_START;
				_pCmdQueue->Enqueue(cmd);
				break;

			case VK_F12:
				cmd.id = CMD_VIDEORECORD_STOP;
				_pCmdQueue->Enqueue(cmd);
				break;
			}
		}
		else if (msg == WM_KEYDOWN)
		{
			//d_printf(L"%s WM_KEYDOWN vk=%d\n",__FUNCTIONW__, wparam);
		}
		
	}	

	return CallNextHookEx(0, code, wparam, lparam);
}