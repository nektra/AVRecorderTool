#pragma once
#include "../Common/ipc.h"

// CPipeServer

extern volatile LONG g_killServer;

class CPipeServer
{
	CPipeServer();           
	virtual ~CPipeServer();

public:
	static UINT threadProc( LPVOID pParam );
	static HANDLE _hPipe;
};


