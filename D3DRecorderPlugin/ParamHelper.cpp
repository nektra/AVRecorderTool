#include "stdafx.h"
#include "ParamHelper.h"

using namespace Deviare2;

IDirect3DDevice9* GetDeviceFromParamInfo(INktHookCallInfoPlugin* lpHookCallInfoPlugin, int index, bool derefPtr)
{
	INktParamsEnumPtr params;
	lpHookCallInfoPlugin->Params(&params);

	INktParamPtr pParam;
	params->GetAt(index, &pParam);

	if (derefPtr)
	{
		pParam->Evaluate(&pParam);
	}
		
	DEVIAREPTR pAddr;
	pParam->get_PointerVal(&pAddr);
	
	return reinterpret_cast<IDirect3DDevice9*>(pAddr);
}

DEVIARERETVAL GetSizeTRetVal(INktHookCallInfoPlugin* lpHookCallInfoPlugin)
{
	INktParamPtr pRV;
	lpHookCallInfoPlugin->Result(&pRV);

	DEVIARERETVAL rv = 0;
	pRV->get_SSizeTVal(&rv);
	return rv;
}

ULONG GetUlongRetVal(INktHookCallInfoPlugin* lpHookCallInfoPlugin)
{
	INktParamPtr pRV;
	lpHookCallInfoPlugin->Result(&pRV);

	ULONG rv = 0;
	pRV->get_ULongVal(&rv);
	return rv;
}

long GetLongParam(INktHookCallInfoPlugin* lpHookCallInfoPlugin, int idx)
{
	INktParamsEnumPtr params;
	lpHookCallInfoPlugin->Params(&params);

	INktParamPtr pp;
	params->GetAt(idx, &pp);

	long ret;
	pp->get_LongVal(&ret);

	return ret;
}

HANDLE GetHandleParam(INktHookCallInfoPlugin* lpHookCallInfoPlugin, int idx)
{
	INktParamsEnumPtr params;
	lpHookCallInfoPlugin->Params(&params);

	INktParamPtr pp;
	params->GetAt(idx, &pp);

	size_t ret;
	pp->get_SizeTVal(&ret);

	return (HANDLE) ret;
}

void* GetPointerParam(INktHookCallInfoPlugin* lpHookCallInfoPlugin, int idx)
{
	INktParamsEnumPtr params;
	lpHookCallInfoPlugin->Params(&params);

	INktParamPtr pp;
	params->GetAt(idx, &pp);

	long long ret;
	pp->get_PointerVal(&ret);

	return (void*) ret;
}
