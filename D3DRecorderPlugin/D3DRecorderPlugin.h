#ifndef D3DRECORDERPLUGIN_H
#define D3DRECORDERPLUGIN_H
#include "ParamHelper.h"

#ifdef D3DRECORDERPLUGIN_EXPORTS
#define D3DRECORDERPLUGIN_API extern "C" __declspec(dllexport)
#else
#define D3DRECORDERPLUGIN_API extern "C" __declspec(dllimport)
#endif

void Handle_IDirect3D9_CreateDevice(IDirect3DDevice9* pID3DDev9);
void Handle_IDirect3DDevice9_EndScene_PRE(IDirect3DDevice9* pID3DDev9);
void Handle_IDirect3DDevice9_EndScene_POST(IDirect3DDevice9* pID3DDev9);
void Handle_IDirect3DDevice9_Reset_PRE(IDirect3DDevice9* pID3DDev9);
void Handle_IDirect3DDevice9_Reset_POST(DEVIARERETVAL rv, IDirect3DDevice9* pID3DDev9);
void Handle_IDirect3DDevice9_Present_POST(IDirect3DDevice9* pID3DDev9);
void Handle_IDirect3DDevice9_Present_PRE(IDirect3DDevice9* pID3DDev9);
void Handle_IDirect3DDevice9_Release_PRE(ULONG rv, IDirect3DDevice9* pID3DDev9);
void Handle_IDirect3DDevice9_Release_POST(ULONG rv, IDirect3DDevice9* pID3DDev9);

#endif //D3DRECORDERPLUGIN_H


