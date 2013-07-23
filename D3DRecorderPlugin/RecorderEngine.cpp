#include "stdafx.h"
#include "AVIVideo.h"
#include "../Common/utils.h"
#include "../Common/ipc.h"
#include "RecorderEngine.h"
#include "Console.h"

CRecorderEngine::CRecorderEngine() :
	_hPipe(NULL),
	_pD3DDev9(0),
	_pD3DSurf9(0),
	_bIsRecording(false),
	_fShowConsole (false) ,
	_videoRate(0),
	_iFileSequence(0),
	_t0(0.0f),
	_curFpsSample(0)
{ 
	ZeroMemory( &_lastConsoleState, sizeof(CConsoleState));
	
	_pAVIVideo = new CAVIVideo;
}

CRecorderEngine::~CRecorderEngine()
{
	d_printf(__FUNCTIONW__);

	delete _pAVIVideo;

	if (_hPipe)
		CloseHandle(_hPipe);	
}

BOOL CRecorderEngine::InitIPC()
{
	d_printf(__FUNCTIONW__);

	_hPipe = CreateFile(D3DRECORDER_PIPENAME,
		GENERIC_READ|GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);

	if (_hPipe == INVALID_HANDLE_VALUE)
	{
		d_printf(L"Cannot connect to named pipe %s. Lasterr = 0x%x\n", D3DRECORDER_PIPENAME, GetLastError());
		return FALSE;
	}
	else
	{
		d_printf(L"Connected to named pipe %s.\n",D3DRECORDER_PIPENAME);
		return TRUE;
	}
}

BOOL CRecorderEngine::AttachToDevice(IDirect3DDevice9* pD3DDev)
{
	d_printf(__FUNCTIONW__);

	_pD3DDev9 = pD3DDev;
	
	return pD3DDev !=NULL;
}

void CRecorderEngine::DetachFromDevice()
{	
	ReleaseOffScreenSurface();
	LostConsoleResources();
	ReleaseConsoleResources();	
	
	_pD3DDev9 = NULL;

}

BOOL CRecorderEngine::InitResources()
{
	return CreateOffScreenSurface() && CreateConsoleResources();
}

BOOL CRecorderEngine::IsDeviceAttached() const
{
	return _pD3DDev9 != NULL;
}

BOOL CRecorderEngine::CreateConsoleResources()
{
	if(!_pConsole)
	{
		if (_lastConsoleState.fOnResetOrLost)
		{
			_pConsole = new CConsole(_pD3DDev9, _lastConsoleState);
			_lastConsoleState.fOnResetOrLost = false;
		}
		else
		{
			_pConsole  = new CConsole(_pD3DDev9, 0, 0, 4, true);
		}
	}
	
	return (_pConsole != NULL);
}

void CRecorderEngine::ReleaseConsoleResources()
{
	d_printf(__FUNCTIONW__);
	assert(_pConsole);

	delete _pConsole;
	_pConsole = NULL;
}

void CRecorderEngine::ResetConsoleResources()
{
	d_printf(__FUNCTIONW__);
	assert(_pConsole);

	_pConsole->OnResetDevice();	
}

void CRecorderEngine::LostConsoleResources()
{
	d_printf(__FUNCTIONW__);
	assert(_pConsole);

	_pConsole->OnLostDevice();	
}

void CRecorderEngine::Update()
{
	_pConsole->Update();	
}

void CRecorderEngine::UpdateFrameRate()
{	
	double t = (double) timeGetTime();
	double dt = (t - _t0);
	_t0 = t;

	double cFt = 1000 / dt;

	_fpsSample[++_curFpsSample % MAX_FPS_SAMPLE_COUNT] = cFt;

	double fps = 0;

    for (int i = 0; i < MAX_FPS_SAMPLE_COUNT; i++)
	{
        fps += _fpsSample[i];
	}

    fps /= (double) MAX_FPS_SAMPLE_COUNT;
	_pConsole->SetCurrentFps(fps);
}

void CRecorderEngine::ConsoleWriteLine(const wchar_t* wszFormat,  ...)
{
	wchar_t wBuf[MAX_STRING_CCH];	
	va_list va;

	va_start(va, wszFormat);
	StringCchVPrintf(wBuf, MAX_STRING_CCH, wszFormat, va);
	va_end(va);

	_pConsole->AddLine(wBuf);	
}

BOOL CRecorderEngine::CreateOffScreenSurface()
{
	d_printf(__FUNCTIONW__);

	assert(_pD3DDev9);

	HRESULT hr;

	CComPtr<IDirect3DSurface9> pBackBuf;
	if (SUCCEEDED(hr = _pD3DDev9->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuf)))
	{
		D3DSURFACE_DESC sd;
		pBackBuf->GetDesc(&sd);

		if (FAILED(hr = _pD3DDev9->CreateOffscreenPlainSurface(sd.Width, 
			sd.Height, 
			sd.Format, 
			D3DPOOL_SYSTEMMEM, 
			&_pD3DSurf9, NULL)))
		{
			d_printf(L"Cannot create OffScreenPlainSurface. HR is 0x%08x\n", hr);
		}
	}		

	return SUCCEEDED(hr);	
}

void CRecorderEngine::ReleaseOffScreenSurface()
{
	d_printf(__FUNCTIONW__);

	if (_pD3DSurf9)
	{
		_pD3DSurf9->Release();
		_pD3DSurf9 = NULL;
	}
}

void CRecorderEngine::OnDeviceReset()
{	
	d_printf(__FUNCTIONW__);

	_lastConsoleState = _pConsole->SaveConsoleState();
	_lastConsoleState.fOnResetOrLost = true;

	ResetConsoleResources();	
	ReleaseConsoleResources();	
	ReleaseOffScreenSurface();	
}

void CRecorderEngine::OnDeviceLost()
{	
	d_printf(__FUNCTIONW__);

	_lastConsoleState = _pConsole->SaveConsoleState();
	_lastConsoleState.fOnResetOrLost = true;

	LostConsoleResources();
	ReleaseOffScreenSurface();
}

BOOL CRecorderEngine::CaptureBackBuffer(IDirect3DSurface9** pBackBuf)
{
	d_printf(__FUNCTIONW__);
	assert(_pD3DDev9);
	assert(_pD3DSurf9);

	HRESULT hr;

	if (SUCCEEDED(hr = _pD3DDev9->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, pBackBuf)))
	{
		if (FAILED(hr = _pD3DDev9->GetRenderTargetData(*pBackBuf, _pD3DSurf9)))
		{
			d_printf(L"%s Cannot get data from backbuffer. HR is 0x%08x\n", __FUNCTIONW__, hr);						
		}
	}
	else
	{
		d_printf(L"%s Cannot get backbuffer data HR is 0x%08x",__FUNCTIONW__,hr);
	}

	return SUCCEEDED(hr);
}

BOOL CRecorderEngine::SaveBackBufferToMemDIB(LPD3DXBUFFER* ppD3DBuffer)
{
	d_printf(__FUNCTIONW__);
	HRESULT hr;
	if (FAILED(hr = D3DXSaveSurfaceToFileInMemory(ppD3DBuffer, D3DXIFF_DIB, _pD3DSurf9, NULL, NULL)))
	{
		d_printf(L"%s: Error saving buffer HR is 0x%08x\n",__FUNCTIONW__,hr);
	}

	return SUCCEEDED(hr);
}

BOOL CRecorderEngine::SaveBackBufferToFile(const wchar_t* wszFileName, D3DXIMAGE_FILEFORMAT format)
{
	d_printf(__FUNCTIONW__);
	HRESULT hr;
	if (FAILED(hr = D3DXSaveSurfaceToFile(wszFileName, format, _pD3DSurf9, NULL, NULL)))
	{
		d_printf(L"%s: Error saving file HR is 0x%08x\n",__FUNCTIONW__,hr);
	}

	return SUCCEEDED(hr);
}

void CRecorderEngine::MakeSequentialFilename(const wchar_t* szPath, 
											 const wchar_t* szPrefix,
											 wchar_t* szOutName, 
											 size_t cch)
{
	const size_t COUNTBUF = 10;
	wchar_t szFSeq[COUNTBUF];

	assert(szOutName);
	ZeroMemory(szOutName, cch * sizeof(wchar_t));

	wchar_t szFileName[MAX_PATH];
	GetModuleFileName(NULL, szFileName, MAX_PATH);
	PathStripPath(szFileName);

	if (szPath)
	{
		StringCchCat(szOutName, cch, szPath);
		StringCchCat(szOutName, cch, L"\\");
	}

	if (szPrefix && lstrlen(szPrefix) > 0)
	{
		StringCchCat(szOutName, cch, szPrefix);
		StringCchCat(szOutName, cch, L"-");
	}

	StringCchCat(szOutName, cch, szFileName); 
	StringCchCat(szOutName, cch, L"-");
	StringCchPrintf(szFSeq, COUNTBUF, L"%08d", _iFileSequence);
	StringCchCat(szOutName, cch, szFSeq);

	_iFileSequence++;
}

void CRecorderEngine::ExtensionFromImageFormat(IMAGEFORMAT f, wchar_t out[5])
{
	switch (f)
	{
	case IMAGEFORMAT_BMP:
		StringCchCopy(out, 5, L".BMP");
		break;

	case IMAGEFORMAT_DIB:
		StringCchCopy(out, 5, L".DIB");
		break;
	}
}

void CRecorderEngine::TakeScreenshot(const wchar_t* fname, IMAGEFORMAT f)
{
	d_printf(L"%s Saving screenshot to %s...\n", __FUNCTIONW__, fname);

	IDirect3DSurface9* pBackBuf = NULL;

	if (CaptureBackBuffer(&pBackBuf))
	{
		SaveBackBufferToFile(fname, (D3DXIMAGE_FILEFORMAT) f);
		pBackBuf->Release();
	}
}

BOOL CRecorderEngine::VideoRecorderInit(wchar_t* fname, int w, int h, int fps)
{
	HRESULT hr; 
	assert(_pAVIVideo);

	if (SUCCEEDED(hr = _pAVIVideo->Create(fname)))
	{
		FRAMESIZE fs;

		if (w == USEDEFAULT || h == USEDEFAULT)
		{
			D3DSURFACE_DESC sd;
			_pD3DSurf9->GetDesc(&sd);

			fs.height = sd.Height;
			fs.width = sd.Width;
		}
		else
		{
			fs.height = h;
			fs.width = w;	
		}

		if (fps == USEDEFAULT)
		{
			fps = 15;
		}

		_pAVIVideo->SetVideoOptions("", &fs, fps);
		_videoRate = fps;
		_bIsRecording = true;
		
		_lastFrameTime = timeGetTime();
		_elapsedTime = 0;

		return TRUE;
	}
	else
	{
		d_printf(L"%s CAviVideo::Create failed, HR is 0x%08x\n", __FUNCTIONW__, hr);

	}

	return FALSE;
}

BOOL CRecorderEngine::IsFrameTimeAvailable()
{
	const int MS_LIMIT = 8000;

	DWORD now = timeGetTime();

	DWORD elpTime = (now > _lastFrameTime) ? (now - _lastFrameTime) : 1;
	_lastFrameTime = now;

	_elapsedTime += (elpTime * _videoRate);

	if (_elapsedTime >= 1000 + _videoRate)
	{
		if (_elapsedTime >= MS_LIMIT)
			_elapsedTime = MS_LIMIT;

		_elapsedTime -= 1000 + _videoRate;

		return TRUE;
	}	

	return FALSE;
}

BOOL CRecorderEngine::VideoGrabFrame()
{
	assert(_pAVIVideo);

	IDirect3DSurface9* pBackBuf;
	HRESULT hr;

	// Collect only at fixed sample rate
	//
	
	if (IsFrameTimeAvailable())
	{
		if (CaptureBackBuffer(&pBackBuf))
		{
			LPD3DXBUFFER pD3DXBuf;

			if (SaveBackBufferToMemDIB(&pD3DXBuf))
			{			
				if (FAILED(hr = _pAVIVideo->GrabFrame(pD3DXBuf->GetBufferPointer())))
				{
					d_printf(L"%s : AVIVIdeo::GrabFrame FAILED. HR=0x%08x\n",__FUNCTIONW__, hr);

					ConsoleWriteLine(L"GrabFrame FAIL HR=0x%08x. Check codec framebuffer format support/choose another codec", hr);
					return FALSE;
				}

				pD3DXBuf->Release();
			}

			pBackBuf->Release();
		}
	}

	return TRUE;
}

void CRecorderEngine::VideoRecorderStop()
{
	assert(_pAVIVideo);

	if (_bIsRecording)
	{
		_pAVIVideo->Close();
		_bIsRecording = false;
		_elapsedTime = 0;		
	}
}

int CRecorderEngine::VideoCurrentFrameCount() const
{
	return _pAVIVideo->GetFrameCount();
}