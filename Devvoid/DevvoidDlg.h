// DevvoidDlg.h : header file
//

#if !defined(AFX_DEVVOIDDLG_H__B12C0AC8_84CB_4859_8E32_5A5EDBAC6068__INCLUDED_)
#define AFX_DEVVOIDDLG_H__B12C0AC8_84CB_4859_8E32_5A5EDBAC6068__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "OutputWnd.h"


/////////////////////////////////////////////////////////////////////////////
// CDevvoidDlg dialog

class CDevvoidDlg : public CDialog
{
// Construction
public:
	CDevvoidDlg(CWnd* pParent = NULL);	// standard constructor

	void OnOK();
	void OnCancel();

	void EndCompileThread();
	void EndBsp();
	void EndLight();

	TOutputWnd	m_wndOutput;


// Dialog Data
	//{{AFX_DATA(CDevvoidDlg)
	enum { IDD = IDD_DEVVOID_DIALOG };
	CProgressCtrl	m_progress;
	CSpinButtonCtrl	m_spSamples;
	CListBox	m_fileList;
	CString	m_szCurrentDir;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDevvoidDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	void DisableButtons();

	void BeginCompileThread();
	void UpdateFileList();

	HICON		m_hIcon;
	COLORREF	m_ambColor;
	bool		m_bCompiling;
	
	CString		m_mapFile;
	bool		m_bBspAndLight;


	friend void Progress_SetRange(int min, int max);
	friend void Progress_SetStep(int step);
	friend int  Progress_Step();


	// Generated message map functions
	//{{AFX_MSG(CDevvoidDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnClose();
	afx_msg void OnBrowse();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnCompile();
	afx_msg void OnLight();
	afx_msg void OnColor();
	afx_msg void OnBsplight();
	afx_msg void OnSelcancelFilelist();
	afx_msg void OnSelchangeFilelist();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DEVVOIDDLG_H__B12C0AC8_84CB_4859_8E32_5A5EDBAC6068__INCLUDED_)
