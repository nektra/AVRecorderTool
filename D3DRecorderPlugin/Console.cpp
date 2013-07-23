#include "stdafx.h"
#include "Console.h"
#include "..\Common\utils.h"

CConsole::CConsole(IDirect3DDevice9* pD3DDev9,
				   int x, 
				   int y, 
				   int lines /*= 1*/, 
				   bool fShowTime /*= false*/, 
				   bool fShowLineNumber /*= false*/,
				   bool fShowFPSCount /*= true */)
				   
{
	_pD3DDev9 = pD3DDev9;
	_pConsoleFont = NULL;
	_pConsoleSprite = NULL;
	_currentFps  = 0.0f;

	_cst.pos.x = x;
	_cst.pos.y = y;
	
	_cst.cLines = lines;
	_cst.fShowLineNumber = fShowLineNumber;
	_cst.fShowTime = fShowTime;
	_cst.fShowFps = fShowFPSCount;

	_cst.fDirty  = true;

	CreateResources();
}

CConsole::CConsole(IDirect3DDevice9* pD3DDev9, const CConsoleState& prevState)
{
	_pD3DDev9 = pD3DDev9;
	_pConsoleFont = NULL;
	_pConsoleSprite = NULL;

	_cst = prevState;
	
	CreateResources();
}


BOOL CConsole::CreateResources()
{
	d_printf(__FUNCTIONW__);

	HRESULT hr;

	HDC hDC = GetDC( NULL );

	if (!hDC)
	{
		d_printf(L"%s: Cannot get global DC! Lasterr is %d\n", __FUNCTIONW__, GetLastError());
		return FALSE;
	}

	// Get backbuffer desc

	IDirect3DSurface9* pBackBuf;
	if (SUCCEEDED(hr = _pD3DDev9->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuf)))
	{
		pBackBuf->GetDesc(&_backbuf_sd);
		pBackBuf->Release();
	}
	else
	{
		d_printf(L"%s: Cannot get device backbuffer. HR is 0x08%x\n", __FUNCTIONW__, hr);
		return FALSE;
	}

	// cache to avoid further MUL operations

	_fontHeight = -MulDiv (CONSOLE_FONTSIZE, GetDeviceCaps (hDC, LOGPIXELSY), 72);
	ReleaseDC(NULL, hDC);

	hr = D3DXCreateFont(_pD3DDev9, _fontHeight,0 , FW_NORMAL, 1, 
		FALSE, DEFAULT_CHARSET, 
		OUT_DEFAULT_PRECIS, 
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
		CONSOLE_FONTFACE,
		&_pConsoleFont);

	if (FAILED(hr))
	{
		d_printf(L"%s: Font creation failed. HR is 0x08%x\n", __FUNCTIONW__, hr);
		return FALSE;
	}

	hr = D3DXCreateSprite(_pD3DDev9, &_pConsoleSprite);

	if (FAILED(hr))
	{
		d_printf(L"%s: Sprite creation failed. HR is 0x08%x\n", __FUNCTIONW__, hr);
		return FALSE;
	}

	return TRUE;
}

void CConsole::DestroyResources()
{
	_pConsoleFont->Release();
	_pConsoleSprite->Release();
	Clear();
}

CConsole::~CConsole(void)
{
	DestroyResources();
}

void CConsole::OnResetDevice()
{
	_pConsoleFont->OnResetDevice();
	_pConsoleSprite->OnResetDevice();	
}

void CConsole::OnLostDevice()
{
	_pConsoleFont->OnLostDevice();
	_pConsoleSprite->OnLostDevice();	
}

void CConsole::Update()
{
	HRESULT hr;

	if (SUCCEEDED(hr = _pConsoleSprite->Begin(D3DXSPRITE_SORT_TEXTURE | D3DXSPRITE_ALPHABLEND)))
	{
		static wchar_t* buf;

		if (_cst.fDirty)
		{
			GenerateOutputBuffer(&buf);
		}	

		RECT rc;
		rc.top = rc.left = 1;
		rc.right = _backbuf_sd.Width;
		rc.bottom = _backbuf_sd.Height;

		if (_pConsoleFont->DrawTextW(_pConsoleSprite, 
			buf, 
			-1, 
			&rc, DT_TOP | DT_NOCLIP | DT_WORDBREAK | DT_LEFT, 
			D3DCOLOR_RGBA(0,0,0,255)) == 0)
		{
			d_printf(L"IDXD3Sprite::DrawTextW FAILED\n");
		}

		// Draw shadow

		rc.top = rc.left = 0;		

		if (_pConsoleFont->DrawTextW(_pConsoleSprite, 
			buf, 
			-1, 
			&rc, DT_TOP | DT_NOCLIP | DT_WORDBREAK | DT_LEFT, 
			D3DCOLOR_RGBA(0,255,0,255)) == 0)
		{
			d_printf(L"IDXD3Sprite::DrawTextW FAILED\n");
		}	

		// If enabled, draw FPS counter
		//

		if (_cst.fShowFps)
		{
			static const size_t FPS_STRLEN = 20;
			wchar_t wszFps[FPS_STRLEN];

			StringCchPrintfEx(wszFps, FPS_STRLEN, 0, 0, 0, L"FPS: %d", (int) max(0,_currentFps));

			if (_pConsoleFont->DrawTextW(_pConsoleSprite, 
				wszFps, 
				-1, 
				&rc, DT_TOP | DT_RIGHT | DT_NOCLIP, D3DCOLOR_RGBA(64,255,0,255)) == 0)
			{
				d_printf(L"IDXD3Sprite::DrawTextW FAILED\n");
			}	
		}

		_pConsoleSprite->End();

		if (_cst.fDirty)
		{
			delete [] buf;
		}
	}	
	else
	{
		d_printf(L"IDXD3Sprite::Begin FAILED. HR = 0x%08x\n", hr);
	}	
}

void CConsole::AddLine(wchar_t* wszText)
{
	if (_lineBuffer.size() == _cst.cLines)
	{
		assert(  *(_lineBuffer.begin()));
		delete[] (* (_lineBuffer.begin()));
		_lineBuffer.erase(_lineBuffer.begin());
	}

	wchar_t* newStr = new wchar_t[MAX_STRING_CCH];
	memset(newStr, 0, MAX_STRING_CCH * sizeof(wchar_t));

	if (_cst.fShowTime)
	{
		SYSTEMTIME tm;
		GetLocalTime(&tm);

		StringCchPrintf(newStr, MAX_STRING_CCH, L"[%d:%d:%d.%d] ", 
			tm.wHour, tm.wMinute, tm.wSecond, tm.wMilliseconds);
	}
	
	StringCchCat(newStr, MAX_STRING_CCH, wszText);

	_lineBuffer.push_back(newStr);
	_cst.fDirty = true;	
}

void CConsole::Clear()
{
	for (auto it = _lineBuffer.begin(); it != _lineBuffer.end(); ++it)
		delete [] (*it);
	
	_lineBuffer.clear();	
	_cst.fDirty = true;
}

void CConsole::GenerateOutputBuffer(wchar_t** ppOutBuffer)
{
	const size_t bufSize = (MAX_STRING_CCH + sizeof(wchar_t)) * sizeof(wchar_t) * _cst.cLines;
	*ppOutBuffer = new wchar_t[bufSize]; 	
	memset(*ppOutBuffer, 0, bufSize * sizeof(wchar_t));

	for (auto it = _lineBuffer.begin(); 
		it != _lineBuffer.end();
		++it)
	{
		// Add trailing '\n'
		wchar_t* pEnd = NULL;
		StringCchCatEx(*ppOutBuffer, bufSize, *it, (STRSAFE_LPWSTR*) &pEnd, NULL, NULL);		
		*pEnd = L'\n';
	}
	
	_cst.fDirty = false;
}

const CConsoleState CConsole::SaveConsoleState() const
{
	return _cst;
}

void CConsole::RestoreConsoleState(const CConsoleState& cs)
{
	_cst = cs;
}


