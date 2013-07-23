#pragma once
#include "Console.h"

// Must do OOP! to handle various dx versions.

// font props?

#define USEDEFAULT			0
#define CONSOLE_LINES		2
#define MAX_LINE_LEN		80

class CAVIVideo;

#define MAX_FPS_SAMPLE_COUNT	100

enum IMAGEFORMAT
{
	IMAGEFORMAT_BMP = D3DXIFF_BMP,
	IMAGEFORMAT_DIB	= D3DXIFF_DIB
};

class CRecorderEngine
{
	CAVIVideo* _pAVIVideo;
	CConsole*  _pConsole;
	CConsoleState _lastConsoleState;

	HANDLE _hPipe;
	
	// Direct3D9 Resources

	IDirect3DDevice9*	_pD3DDev9;
	IDirect3DSurface9*	_pD3DSurf9;
	
	bool				_fShowConsole;	
	bool				_bIsRecording;
	DWORD				_videoRate;
	double				_t0;
	DWORD				_lastFrameTime, _elapsedTime;
	int					_iFileSequence;
	double				_fpsSample[MAX_FPS_SAMPLE_COUNT];
	int					_curFpsSample;


	// functions

	BOOL CreateOffScreenSurface();
	void ReleaseOffScreenSurface();

	BOOL CreateConsoleResources();
	void ResetConsoleResources();
	void LostConsoleResources();
	void ReleaseConsoleResources();
	
	BOOL CaptureBackBuffer(IDirect3DSurface9**);
	BOOL SaveBackBufferToFile(const wchar_t* wszFileName, D3DXIMAGE_FILEFORMAT format);
	BOOL SaveBackBufferToMemDIB(LPD3DXBUFFER* ppDestBuf);

	void ExtensionFromImageFormat(IMAGEFORMAT f, wchar_t ext[5]);

	BOOL IsFrameTimeAvailable();
	
public:
	CRecorderEngine();
	~CRecorderEngine();

	BOOL InitIPC();
	BOOL AttachToDevice(IDirect3DDevice9* pD3DDev);
	void DetachFromDevice();
	BOOL IsDeviceAttached() const;
	BOOL InitResources();
	void OnDeviceReset();
	void OnDeviceLost();
	void Update();
	void UpdateFrameRate();

	void ConsoleWriteLine(const wchar_t* wszFormat,...);

	// Screenshot
	
	void MakeSequentialFilename(
		const wchar_t* szTargetPath,	
		const wchar_t* szPrefix,
		wchar_t* szOut_fname,
		size_t cch);

	void TakeScreenshot(const wchar_t* fname, IMAGEFORMAT f);

	// AVI Video 

	BOOL VideoRecorderInit(wchar_t* fname, int w = USEDEFAULT, int h = USEDEFAULT, int fps = USEDEFAULT);
	BOOL IsVideoRecording() const { return _bIsRecording; }
	int  VideoCurrentFrameCount() const;
	BOOL VideoGrabFrame();
	void VideoRecorderStop();	
	
};