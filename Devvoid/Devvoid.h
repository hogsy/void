// Devvoid.h : main header file for the DEVVOID application
//

#if !defined(AFX_DEVVOID_H__50A0BE38_AD7D_4C29_A5DD_1BD74311F559__INCLUDED_)
#define AFX_DEVVOID_H__50A0BE38_AD7D_4C29_A5DD_1BD74311F559__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CDevvoidApp:
// See Devvoid.cpp for the implementation of this class
//

class CDevvoidApp : public CWinApp
{
	bool ChangeToVoidDir();

public:

	char m_exePath[_MAX_PATH];

	CDevvoidApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDevvoidApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CDevvoidApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DEVVOID_H__50A0BE38_AD7D_4C29_A5DD_1BD74311F559__INCLUDED_)
