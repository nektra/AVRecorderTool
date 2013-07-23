#pragma once
#include "stdafx.h"

#define CONSOLE_FONTFACE L"System"
#define CONSOLE_FONTSIZE 12

struct  CConsoleState
{
	POINT	pos;	
	bool	fShowLineNumber;
	bool	fShowTime;
	bool	fShowFps;
	int		cLines;
	bool	fDirty;
	bool	fOnResetOrLost;
};
	
class CConsole
{
	IDirect3DDevice9*	_pD3DDev9;
	LPD3DXFONT			_pConsoleFont;
	LPD3DXSPRITE		_pConsoleSprite;
	int					_fontHeight;
	D3DSURFACE_DESC		_backbuf_sd;

	CConsoleState		_cst;

	double				_currentFps;
	
	typedef std::vector<LPCWSTR> StringBuffer_t;
	StringBuffer_t _lineBuffer;

	//////////////////////////////////////////////////////////////////////////

	void GenerateOutputBuffer(wchar_t** ppOutBuffer);

public:
	CConsole(IDirect3DDevice9* pDev9,
		int x, 
		int y, 
		int lines = 1, 
		bool fShowTime = false, 
		bool fShowLineNumber = false,
		bool fShowFpsCounter = true);

	CConsole(IDirect3DDevice9* pDev9, const CConsoleState& prevState);
	
	~CConsole(void);

	BOOL CreateResources();
	void DestroyResources();

	void Show();
	void Hide();
	void Clear();
	void AddLine(wchar_t* wszText);
	void Update();	

	void OnResetDevice();
	void OnLostDevice();	

	void SetShowLineNumber(bool f) { _cst.fShowLineNumber = f; }
	void SetShowTime(bool f) { _cst.fShowTime = f; }

	const CConsoleState SaveConsoleState() const;
	void RestoreConsoleState(const CConsoleState&);

	void SetCurrentFps(double f) { _currentFps = f; }
};

