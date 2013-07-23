#pragma once

#define MAX_STRING_CCH  1024

template <typename T>
inline void SafeDelete(T* p)
{
	delete p;
	p = NULL;
}

static void d_printf(const wchar_t* szFormat, ...)
{
	wchar_t wBuf[MAX_STRING_CCH];	
	va_list va;

	va_start(va, szFormat);
	vswprintf (wBuf, MAX_STRING_CCH, szFormat, va);
	va_end(va);

	OutputDebugString(wBuf);
}

static void DumpDIBHeader(BITMAPINFOHEADER* bih)
{
	d_printf(L"Dump of BITMAPINFOHEADER at 0x%p", bih);
	d_printf(L"   biBitCount: %d\n", bih->biBitCount);
	d_printf(L"   biClrUsed: %d\n", bih->biClrUsed);
	d_printf(L"   biHeight: %d\n", bih->biHeight);
	d_printf(L"   biWidth: %d\n", bih->biWidth);
	d_printf(L"   biSizeImage: %d\n", bih->biSizeImage);
	d_printf(L"   biPlanes: %d\n", bih->biPlanes);	
}


__inline UINT_PTR CoGetMethodAddress(LPVOID pVT, UINT index)
{
	UINT_PTR *pdwVirtualTable;
	pdwVirtualTable = (UINT_PTR*)*(UINT_PTR*)pVT;
	return pdwVirtualTable[index];	
}

static void GetOutputDir( wchar_t* szDir, DWORD cchBuf)
{
	DWORD cBytes = cchBuf*sizeof(WCHAR);

	SHRegGetUSValue(L"Software\\Nektra\\D3DRecorder\\Configuration",
		L"OutputFolder", NULL, szDir, &cBytes, FALSE, 0, 0);		
}

//////////////////////////////////////////////////////////////////////////////
//
// VTABLE Function Indices for Core Audio interfaces
//
//////////////////////////////////////////////////////////////////////////////

struct VTI_IAudioRenderClient
{
	static const int QueryInterface =0;
	static const int AddRef = 1;
	static const int Release = 2;
	static const int GetBuffer = 3;
	static const int ReleaseBuffer = 4;
};

struct VTI_IAudioCaptureClient
{
	static const int QueryInterface =0;
	static const int AddRef = 1;
	static const int Release = 2;
	static const int GetBuffer = 3;
	static const int ReleaseBuffer = 4;
};

struct VTI_IAudioClient
{
	static const int QueryInterface =0;
	static const int AddRef = 1;
	static const int Release = 2;
	static const int Initialize = 3;
	static const int Start  = 10;
	static const int Stop  = 11;
	static const int Reset = 12;
	static const int GetService = 14;	
};

//////////////////////////////////////////////////////////////////////////////
//
// Struct for Core Audio  Interface pointers
//
//////////////////////////////////////////////////////////////////////////////

interface IAudioRenderClient;
interface IAudioCaptureClient;
interface IAudioClient;

struct COREAUDIO_VTABLE_PTR
{
	DWORD_PTR			pvt_IAudioRenderClient;
	DWORD_PTR			pvt_IAudioCaptureClient;
	DWORD_PTR			pvt_IAudioClient;
};

//////////////////////////////////////////////////////////////////////////////
//
// VTABLE Function Indices for Direct3D 9.0 Interfaces
//
//////////////////////////////////////////////////////////////////////////////

struct VTI_IDirect3D9
{
	static const int QueryInterface =0;
	static const int AddRef = 1;
	static const int Release = 2;

	//...

	static const int CreateDevice = 16;
};

struct VTI_IDirect3D9Device
{
	static const int Queryinterface = 0;
	static const int AddRef = 1;
	static const int Release = 2;
	static const int TestCooperativeLevel = 3;
	static const int GetAvailableTextureMem = 4;
	static const int EvictManagedResources = 5;
	static const int GetDirect3D = 6;
	static const int GetDeviceCaps = 7;
	static const int GetDisplayMode = 8;
	static const int GetCreationParameters = 9;
	static const int SetCursorProperties = 10;
	static const int SetCursorPosition = 11;
	static const int ShowCursor = 12;
	static const int CreateAdditionalSwapChain = 13;
	static const int GetSwapChain = 14;
	static const int GetNumberOfSwapChains = 15;
	static const int Reset = 16;
	static const int Present = 17;
	static const int GetBackBuffer = 18;
	static const int GetRasterStatus = 19;
	static const int SetDialogBoxMode = 20;
	static const int SetGammaRamp = 21;
	static const int GetGammaRamp = 22;
	static const int CreateTexture = 23;
	static const int CreateVolumeTexture = 24;
	static const int CreateCubeTexture = 25;
	static const int CreateVertexBuffer = 26;
	static const int CreateIndexBuffer = 27;
	static const int CreateRenderTarget = 28;
	static const int CreateDepthStencilSurface = 29;
	static const int UpdateSurface = 30;
	static const int UpdateTexture = 31;
	static const int GetRenderTargetData = 32;
	static const int GetFrontBufferData = 33;
	static const int StretchRect = 34;
	static const int ColorFill = 35;
	static const int CreateOffscreenPlainSurface = 36;
	static const int SetRenderTarget = 37;
	static const int GetRenderTarget = 38;
	static const int SetDepthStencilSurface = 39;
	static const int GetDepthStencilSurface = 40;
	static const int BeginScene = 41;
	static const int EndScene = 42;
	static const int Clear = 43;
	static const int SetTransform = 44;
	static const int GetTransform = 45;
	static const int MultiplyTransform = 46;
	static const int SetViewport = 47;
	static const int GetViewport = 48;
	static const int SetMaterial = 49;
	static const int GetMaterial = 50;
	static const int SetLight = 51;
	static const int GetLight = 52;
	static const int LightEnable = 53;
	static const int GetLightEnable = 54;
	static const int SetClipPlane = 55;
	static const int GetClipPlane = 56;
	static const int SetRenderState = 57;
	static const int GetRenderState = 58;
	static const int CreateStateBlock = 59;
	static const int BeginStateBlock = 60;
	static const int EndStateBlock = 61;
	static const int SetClipStatus = 62;
	static const int GetClipStatus = 63;
	static const int GetTexture = 64;
	static const int SetTexture = 65;
	static const int GetTextureStageState = 66;
	static const int SetTextureStageState = 67;
	static const int GetSamplerState = 68;
	static const int SetSamplerState = 69;
	static const int ValidateDevice = 70;
	static const int SetPaletteEntries = 71;
	static const int GetPaletteEntries = 72;
	static const int SetCurrentTexturePalette = 73;
	static const int GetCurrentTexturePalette = 74;
	static const int SetScissorRect = 75;
	static const int GetScissorRect = 76;
	static const int SetSoftwareVertexProcessing = 77;
	static const int GetSoftwareVertexProcessing = 78;
	static const int SetNPatchMode = 79;
	static const int GetNPatchMode = 80;
	static const int DrawPrimitive = 81;
	static const int DrawIndexedPrimitive = 82;
	static const int DrawPrimitiveUP = 83;
	static const int DrawIndexedPrimitiveUP = 84;
	static const int ProcessVertices = 85;
	static const int CreateVertexDeclaration = 86;
	static const int SetVertexDeclaration = 87;
	static const int GetVertexDeclaration = 88;
	static const int SetFVF = 89;
	static const int GetFVF = 90;
	static const int CreateVertexShader = 91;
	static const int SetVertexShader = 92;
	static const int GetVertexShader = 93;
	static const int SetVertexShaderConstantF = 94;
	static const int GetVertexShaderConstantF = 95;
	static const int SetVertexShaderConstantI = 96;
	static const int GetVertexShaderConstantI = 97;
	static const int SetVertexShaderConstantB = 98;
	static const int GetVertexShaderConstantB = 99;
	static const int SetStreamSource = 100;
	static const int GetStreamSource = 101;
	static const int SetStreamSourceFreq = 102;
	static const int GetStreamSourceFreq = 103;
	static const int SetIndices = 104;
	static const int GetIndices = 105;
	static const int CreatePixelShader = 106;
	static const int SetPixelShader = 107;
	static const int GetPixelShader = 108;
	static const int SetPixelShaderConstantF = 109;
	static const int GetPixelShaderConstantF = 110;
	static const int SetPixelShaderConstantI = 111;
	static const int GetPixelShaderConstantI = 112;
	static const int SetPixelShaderConstantB = 113;
	static const int GetPixelShaderConstantB = 114;
	static const int DrawRectPatch = 115;
	static const int DrawTriPatch = 116;
	static const int DeletePatch = 117;
	static const int CreateQuery = 118;
	static const int NumberOfFunctions = 118;
};

//