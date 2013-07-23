// PipeServer.cpp : implementation file
//

#include "stdafx.h"
#include "D3DRecorder.h"
#include "PipeServer.h"

HANDLE CPipeServer::_hPipe = NULL;

volatile LONG g_killServer;

CPipeServer::CPipeServer()
{
	
}

CPipeServer::~CPipeServer()
{
	if (_hPipe)
		CloseHandle(_hPipe);
}

UINT CPipeServer::threadProc(LPVOID lParam)
{
	_hPipe = CreateNamedPipe(D3DRECORDER_PIPENAME, 
		PIPE_ACCESS_DUPLEX,
		PIPE_TYPE_BYTE | PIPE_READMODE_BYTE,
		PIPE_UNLIMITED_INSTANCES | PIPE_WAIT,
		IPCMESSAGESIZE*2,
		IPCMESSAGESIZE*2,
		NMPWAIT_WAIT_FOREVER,
		NULL);

	if (_hPipe != INVALID_HANDLE_VALUE)
	{
		TRACE("Named Pipe Creation successful. Waiting for connection...\n");

		ConnectNamedPipe(_hPipe, NULL);

		IPCMESSAGE msg;
		DWORD dwRead;			

		while (g_killServer == 0)
		{
			Sleep(100);			 // enough to yield CPU to scheduler
			
			if (ReadFile(_hPipe, &msg, IPCMESSAGESIZE, &dwRead, NULL))
			{
				IPCMESSAGE* pMsg = new IPCMESSAGE;
				RtlCopyMemory((LPVOID)pMsg, &msg, sizeof(IPCMESSAGE));				

				SendMessage((HWND) lParam, WM_IPCMESSAGE, (WPARAM) pMsg, 0); 				
			}			
		}		
	}
	else
	{
		TRACE("Named Pipe Creation failed. Last Error is  %d\n", GetLastError());
	}
	
	return TRUE;
}

