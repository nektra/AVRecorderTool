#pragma once
#include "stdafx.h"

class CCommandQueue;

class CKeyboardHook
{
	static CCommandQueue*	_pCmdQueue;	
	static HHOOK _hHook;

	static LRESULT CALLBACK CKeyboardHook::KeyboardProc(int code, WPARAM wparam, LPARAM lparam);
	
	
public:
	BOOL Initialize(IDirect3DDevice9* pID3DDev9);
	CKeyboardHook(CCommandQueue*);
	~CKeyboardHook(void);

	void Start();
};

