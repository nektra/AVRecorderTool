
// D3DRecorderDlg.cpp : implementation file
//

#include "stdafx.h"
#include "HookEngine.h"
#include "D3DRecorder.h"
#include "D3DRecorderDlg.h"
#include "afxdialogex.h"
#include "../Common/ipc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();
	
	// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CD3DRecorderDlg dialog



CD3DRecorderDlg::CD3DRecorderDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CD3DRecorderDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CD3DRecorderDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TARGETBINARYFILE, m_browseCtrl);
	DDX_Control(pDX, IDC_TARGETFOLDER, m_browseFolder);
	DDX_Control(pDX, IDC_LAUNCH, m_btnLaunch);
	//  DDX_Text(pDX, IDC_STATUSTEXT, m_statusText);
	DDX_Control(pDX, IDC_STATUSTEXT, m_statusText);
}

BEGIN_MESSAGE_MAP(CD3DRecorderDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_LAUNCH, &CD3DRecorderDlg::OnBnClickedLaunch)
	ON_MESSAGE(WM_IPCMESSAGE, OnIPCMessage)
	ON_MESSAGE(WM_PROCESSTERMINATED, OnProcessTerminated)
END_MESSAGE_MAP()


// CD3DRecorderDlg message handlers

BOOL CD3DRecorderDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	m_browseCtrl.EnableFileBrowseButton(L".exe", L"Executable files (*.exe)|*.exe");

	m_browseCtrl.SetWindowText(theApp.GetProfileString(L"Configuration", L"LastLaunch"));
	m_browseFolder.SetWindowTextW(theApp.GetProfileString(L"Configuration", L"OutputFolder"));

	m_statusText.SetWindowText(L"Stopped");


	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CD3DRecorderDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CD3DRecorderDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CD3DRecorderDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CD3DRecorderDlg::OnBnClickedLaunch()
{
	CString strFilename, strFolder;
	m_browseCtrl.GetWindowText(strFilename);
	m_browseFolder.GetWindowText(strFolder);

	CFileStatus fs;

	if (!CFile::GetStatus(strFilename.GetBuffer(), fs))
	{
		::AfxMessageBox(L"Cannot open target executable file", MB_ICONERROR);
		return;
	}

	if (!CFile::GetStatus(strFolder.GetBuffer(), fs))
	{
		::AfxMessageBox(L"Cannot open target folder for capture data", MB_ICONERROR);
		return;
	}

	theApp.WriteProfileString(L"Configuration", L"LastLaunch", strFilename);
	theApp.WriteProfileString(L"Configuration", L"OutputFolder", strFolder);

	// Try to launch this process thorugh SpyMgr.

	CHookEngine* pHookEngine = theApp.GetHookEngine();

	if (pHookEngine->CreatePipeServer(GetSafeHwnd()))   // Pass this HWND for interthread messaging
	{
		if (pHookEngine->LaunchProcess(strFilename))
		{
			pHookEngine->SetParentDialog(this->GetSafeHwnd());
			m_btnLaunch.EnableWindow(0);
			m_statusText.SetWindowTextW(L"Process is running..."); 
			pHookEngine->HookD3D9Creation();		
			pHookEngine->ResumeProcess();
		}
	}
}

LRESULT CD3DRecorderDlg::OnIPCMessage(WPARAM wParam,LPARAM)
{
	IPCMESSAGE* pMsg = (IPCMESSAGE*)wParam;
	ASSERT(pMsg);

	TRACE("Unknown IPC message!");
	
	delete pMsg;
	pMsg = NULL;
	return 0L;
}

LRESULT CD3DRecorderDlg::OnProcessTerminated(WPARAM,LPARAM)
{

	m_statusText.SetWindowText(L"Stopped.");
	m_btnLaunch.EnableWindow(1);
	return 0L;
}