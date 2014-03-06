#pragma once
#include "stdafx.h"

#ifdef _M_IX86
#define SIZETVAL unsigned long
#define REGSIZE long
#define DEVIAREPTR long
#elif defined _M_X64
#define SIZETVAL unsigned long long
#define REGSIZE __int64
#define DEVIAREPTR __int64
#endif
#define DEVIARERETVAL REGSIZE

IDirect3DDevice9* GetDeviceFromParamInfo(Deviare2::INktHookCallInfoPlugin* lpHookCallInfoPlugin, 
										 int index = 0, 
										 bool deref = false);

DEVIARERETVAL	GetSizeTRetVal(Deviare2::INktHookCallInfoPlugin* lpHookCallInfoPlugin);
ULONG			GetUlongRetVal(Deviare2::INktHookCallInfoPlugin* lpHookCallInfoPlugin);

long			GetLongParam(Deviare2::INktHookCallInfoPlugin* lpHookCallInfoPlugin, int idx);
HANDLE			GetHandleParam(Deviare2::INktHookCallInfoPlugin* lpHookCallInfoPlugin, int idx);
void*			GetPointerParam(Deviare2::INktHookCallInfoPlugin* lpHookCallInfoPlugin, int idx);
