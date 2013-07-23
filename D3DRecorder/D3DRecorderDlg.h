
// D3DRecorderDlg.h : header file
//

#pragma once
#include "afxeditbrowsectrl.h"
#include "afxwin.h"

struct IPCMESSAGE;

// CD3DRecorderDlg dialog
class CD3DRecorderDlg : public CDialogEx
{
// Construction
public:
	CD3DRecorderDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_D3DRECORDER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CMFCEditBrowseCtrl m_browseCtrl;	
	afx_msg void OnBnClickedLaunch();
	afx_msg LRESULT OnIPCMessage(WPARAM,LPARAM);
	afx_msg void OnEnChangeTargetfolder();
	afx_msg LRESULT OnProcessTerminated(WPARAM,LPARAM);
	CMFCEditBrowseCtrl m_browseFolder;
	CButton m_btnLaunch;
//	CString m_statusText;
	CStatic m_statusText;
};
