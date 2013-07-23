#pragma once

#define D3DRECORDER_PIPENAME L"\\\\.\\pipe\\D3DRECORDER_PIPE"

#define MSG_NULL								0
#define MSG_METHODADDR_DIRECT3D_CREATEDEVICE	1
#define MSG_METHODADDR_DIRECT3DDEVICE_ENDSCENE	2

// 1k messages

struct IPCMESSAGE
{
	int msgType;

	union
	{
		DWORD_PTR address;
	};
};

#define IPCMESSAGESIZE sizeof(IPCMESSAGE)