// DevvoidDlg.cpp : implementation file
//
#include "stdafx.h"
#include "Devvoid.h"
#include "DevvoidDlg.h"
#include "Std_lib.h"
#include "Com_util.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


const char DEFAULT_WORLDS_DIR [] = "Worlds";
const char DEFAULT_GAME_DIR[] = "Game";

struct LightParms
{
	LightParms () { ambientColor = 0; samples = 0; }

	char szFileName[_MAX_PATH];
	char szPath[_MAX_PATH];
	COLORREF ambientColor;
	int	samples;
};

UINT BeginBSPCompile( LPVOID pParam );
UINT BeginLightMapCompile( LPVOID pParam );


//==========================================================================
// CAboutDlg dialog used for App About
//==========================================================================

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//==========================================================================
//==========================================================================


/*
================================================
Constructor
================================================
*/
CDevvoidDlg::CDevvoidDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDevvoidDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDevvoidDlg)
	m_szCurrentDir = _T("");
	m_bCompiling = false;
	m_ambColor = 0;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}


void CDevvoidDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDevvoidDlg)
	DDX_Control(pDX, IDC_PROGRESS, m_progress);
	DDX_Control(pDX, IDC_SPIN1, m_spSamples);
	DDX_Control(pDX, IDC_FILELIST, m_fileList);
	DDX_Text(pDX, IDC_CURRENTDIR, m_szCurrentDir);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDevvoidDlg, CDialog)
	//{{AFX_MSG_MAP(CDevvoidDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
	ON_WM_CREATE()
	ON_BN_CLICKED(IDC_COMPILE, OnCompile)
	ON_BN_CLICKED(IDC_LIGHT, OnLight)
	ON_BN_CLICKED(IDC_COLOR, OnColor)
	ON_BN_CLICKED(IDC_BSPLIGHT, OnBsplight)
	ON_LBN_SELCANCEL(IDC_FILELIST, OnSelcancelFilelist)
	ON_LBN_SELCHANGE(IDC_FILELIST, OnSelchangeFilelist)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDevvoidDlg message handlers
/*
================================================
OnInitDialog
================================================
*/
BOOL CDevvoidDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
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

	// TODO: Add extra initialization here
	m_szCurrentDir = _T(GetVoidPath());
	m_szCurrentDir += _T("\\");
	m_szCurrentDir += _T(DEFAULT_GAME_DIR);
	m_szCurrentDir += _T("\\");
	m_szCurrentDir += _T(DEFAULT_WORLDS_DIR);

	GetDlgItem(IDC_EDSAMPLES)->SetWindowText("1");
	m_spSamples.SetBuddy(GetDlgItem(IDC_EDSAMPLES));
	m_spSamples.SetRange(1,3);
	m_spSamples.SetPos(1);
	
	UpdateFileList();
	m_fileList.SetFocus();

	OnSelchangeFilelist();

	UpdateData(FALSE);
	
	return FALSE; //TRUE;  // return TRUE  unless you set the focus to a control
}


/*
================================================
OnSysCommand
================================================
*/
void CDevvoidDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}


/*
================================================
If you add a minimize button to your dialog, you will need the code below
to draw the icon.  For MFC applications using the document/view model,
this is automatically done for you by the framework.
================================================
*/
void CDevvoidDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

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
		CDialog::OnPaint();
	}
}


/*
================================================
The system calls this to obtain the cursor to display while the user drags
the minimized window.
================================================
*/
HCURSOR CDevvoidDlg::OnQueryDragIcon()
{	return (HCURSOR) m_hIcon;
}

/*
================================================
Override standard handlers to get rid of 
escape/enter quits
================================================
*/
void CDevvoidDlg::OnOK(){}
void CDevvoidDlg::OnCancel(){}
void CDevvoidDlg::OnClose() 
{	DestroyWindow();
}



void CDevvoidDlg::OnSelcancelFilelist() 
{
	// TODO: Add your control notification handler code here
	AfxMessageBox("ListBox lost focus");
	DisableButtons();
}

void CDevvoidDlg::OnSelchangeFilelist() 
{
	// TODO: Add your control notification handler code here

//	AfxMessageBox("Changed focus");

	if(m_bCompiling)
		return;

	if(m_fileList.GetCurSel() == LB_ERR)
	{
		ComPrintf("Selection changed to none\n");
		return;
	}

	CButton * pButton = 0;
	CString curItem;
	
	m_fileList.GetText(m_fileList.GetCurSel(), curItem);

	if(Util::CompareExts(curItem,"map"))
	{
		pButton = (CButton*)GetDlgItem(IDC_LIGHT);
		if(pButton)
			pButton->EnableWindow(FALSE);

		pButton = (CButton*)GetDlgItem(IDC_COMPILE);
		if(pButton)
			pButton->EnableWindow(TRUE);
		return;
	}
	
	if(Util::CompareExts(curItem,"wld"))
	{
		pButton = (CButton*)GetDlgItem(IDC_LIGHT);
		if(pButton)
			pButton->EnableWindow(TRUE);

		pButton = (CButton*)GetDlgItem(IDC_COMPILE);
		if(pButton)
			pButton->EnableWindow(FALSE);
		return;
	}
	
	DisableButtons();

//	ComPrintf("Selected %s\n",curItem);
}


/*
================================================
Create the log window
================================================
*/
int CDevvoidDlg::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
	CRect rect;

	GetClientRect(&rect);

	rect.left = 10;
	rect.top = rect.bottom/2;
	rect.bottom -= 10;
	rect.right -= 10;

	m_wndOutput.Create(0,0,WS_VISIBLE|WS_BORDER,rect,this,0,0); 

	m_wndOutput.SetMaxLines(512);
	m_wndOutput.SetBackColour(RGB( 0, 0, 0 ));
	m_wndOutput.SetDefaultTextColour(0);	// ? weird
	
	return 0;
}

/*
================================================
Update the fileList in the List box
================================================
*/
void CDevvoidDlg::UpdateFileList()
{
	CString oldSelection;
	if(m_fileList.GetCurSel() != LB_ERR)
		m_fileList.GetText(m_fileList.GetCurSel(), oldSelection);

	m_fileList.ResetContent();

	WIN32_FIND_DATA	findData;
	HANDLE hSearch = INVALID_HANDLE_VALUE;
	
	//Add maps

	CString szSearchPath = m_szCurrentDir + "\\*.map";

	hSearch	= ::FindFirstFile(szSearchPath,&findData);
	
	if(hSearch == INVALID_HANDLE_VALUE)
	{
		DisableButtons();
		return;
	}

	m_fileList.AddString(findData.cFileName);
	
	while(::FindNextFile(hSearch,&findData))
	{	m_fileList.AddString(findData.cFileName);
	}
		
	::FindClose(hSearch);


	//Add wlds
	szSearchPath = m_szCurrentDir + "\\*.wld";

	hSearch	= ::FindFirstFile(szSearchPath,&findData);
	
	if(hSearch == INVALID_HANDLE_VALUE)
	{
		DisableButtons();
		return;
	}

	m_fileList.AddString(findData.cFileName);
	
	while(::FindNextFile(hSearch,&findData))
	{	m_fileList.AddString(findData.cFileName);
	}
		
	::FindClose(hSearch);


	m_fileList.GetFocus();	
	if(oldSelection.GetLength())
		m_fileList.SelectString(-1,oldSelection);
	else if(m_fileList.GetCount())
		m_fileList.SetCurSel(0);
}


/*
================================================
Browse for folder
================================================
*/
void CDevvoidDlg::OnBrowse() 
{
	
	BROWSEINFO		browseInfo;
	LPITEMIDLIST	pIdList;

	memset(&browseInfo,0, sizeof(BROWSEINFO)); 
	browseInfo.hwndOwner = m_hWnd;
	browseInfo.ulFlags = BIF_RETURNONLYFSDIRS;
	browseInfo.pidlRoot = 0;
	browseInfo.lpszTitle = "Select the maps Folder in your Void directory";
	browseInfo.lpfn = 0;
	browseInfo.lParam = 0;
	browseInfo.iImage = 0;
	
	pIdList = ::SHBrowseForFolder(&browseInfo);

	if(pIdList)
	{
		char path[_MAX_PATH];
		::SHGetPathFromIDList(pIdList,path);

		if(_strnicmp(path, GetVoidPath(), strlen(GetVoidPath())))
		{	
			AfxMessageBox("Please select a worlds path within your Void directory\n");
			return;
		}

		m_szCurrentDir = path;
		UpdateFileList();
		UpdateData(FALSE);
	}
}

/*
================================================
Compile the given map
================================================
*/
void CDevvoidDlg::OnCompile() 
{
	if(m_fileList.GetCurSel() == LB_ERR)
	{
		AfxMessageBox("CDevvoidDlg::OnCompile::Select a map file first");
		return;
	}

	CString szListItem;
	m_fileList.GetText(m_fileList.GetCurSel(), szListItem);
	
	if(!szListItem.GetLength())
	{
		AfxMessageBox("Select a file to compile");
		return;
	}

	CButton * pCheckBox = (CButton*)GetDlgItem(IDC_FASTMODE);

	if(pCheckBox && pCheckBox->GetCheck()==1)
		g_bFastBSP = true;

	char * fileName = new char [_MAX_PATH];
	sprintf(fileName,"%s/%s/%s/%s", GetVoidPath(), DEFAULT_GAME_DIR, DEFAULT_WORLDS_DIR, szListItem);

	if(!AfxBeginThread(BeginBSPCompile,(void*)fileName))
	{
		AfxMessageBox("CDevvoidDlg::OnCompile::Unable to spawn BSP thread");
		return;
	}
	
	BeginCompileThread();
}

/*
================================================
Light the given map
================================================
*/
void CDevvoidDlg::OnLight() 
{
	if(m_fileList.GetCurSel() == LB_ERR)
	{
		AfxMessageBox("CDevvoidDlg::OnCompile::Select a map file first");
		return;
	}

	CString szListItem;
	m_fileList.GetText(m_fileList.GetCurSel(), szListItem);
	
	if(!szListItem.GetLength())
	{
		AfxMessageBox("CDevvoidDlg::OnCompile::Unable to read current selection");
		return;
	}

	LightParms * pLightParms = new LightParms;

	sprintf(pLightParms->szPath,"%s/%s", GetVoidPath(), DEFAULT_GAME_DIR);
	sprintf(pLightParms->szFileName,"%s/%s",DEFAULT_WORLDS_DIR,szListItem);
	pLightParms->ambientColor = m_ambColor;
	pLightParms->samples = m_spSamples.GetPos();

	if(!AfxBeginThread(BeginLightMapCompile,(void*)pLightParms))
	{
		AfxMessageBox("CDevvoidDlg::OnCompile::Unable to spawn BSP thread");
		return;
	}

	BeginCompileThread();
}


/*
================================================
Set Color
================================================
*/
void CDevvoidDlg::OnColor() 
{
	// TODO: Add your control notification handler code here
	CColorDialog dlgColor;


	dlgColor.m_cc.Flags |= CC_ANYCOLOR|CC_FULLOPEN|CC_RGBINIT;
	dlgColor.m_cc.rgbResult = 0;

	if(dlgColor.DoModal() == IDOK)
	{
		m_ambColor = dlgColor.GetColor();
		ComPrintf("Color changed to : %d %d %d\n", 
				GetRValue(m_ambColor),GetGValue(m_ambColor),GetBValue(m_ambColor));
	}
}

/*
================================================
Both Light AND BSP it
================================================
*/
void CDevvoidDlg::OnBsplight() 
{
	// TODO: Add your control notification handler code here
	AfxMessageBox("The, ahem, developer hasn't got around to implementing this yet. "
				  "Please bug him if you want this feature.");	
}


//==========================================================================
//==========================================================================

void CDevvoidDlg::DisableButtons()
{
	CButton * pButton = 0;
	pButton = (CButton*)GetDlgItem(IDC_LIGHT);
	if(pButton)
		pButton->EnableWindow(FALSE);

	pButton = (CButton*)GetDlgItem(IDC_COMPILE);
	if(pButton)
		pButton->EnableWindow(FALSE);
}

/*
================================================
A thread just began
================================================
*/
void CDevvoidDlg::BeginCompileThread()
{	
	DisableButtons();
	m_bCompiling = true;
}


/*
================================================
other buttons are disable when a compile 
is taking place. enable them here
================================================
*/
void CDevvoidDlg::EndCompileThread()
{
	CButton * pButton = 0;
	pButton = (CButton*)GetDlgItem(IDC_LIGHT);
	if(pButton)
		pButton->EnableWindow(TRUE);

	pButton = (CButton*)GetDlgItem(IDC_COMPILE);
	if(pButton)
		pButton->EnableWindow(TRUE);

	m_bCompiling = false;

	UpdateFileList();
	OnSelchangeFilelist();

	ComPrintf("Thread Returned\n");
}



//==========================================================================
//==========================================================================
/*
================================================
BSP worker thread func
================================================
*/
void CompileBsp(const char * szFileName);

UINT BeginBSPCompile( LPVOID pParam )
{
	char * bspFile = (char*)pParam;
	CompileBsp(bspFile);
	delete [] bspFile;

	((CDevvoidDlg*)AfxGetApp()->GetMainWnd())->EndCompileThread();
	return 0;
}


/*
================================================
Lightmaps compilation
================================================
*/
void CompileLightmaps(const char * szPath, const char * szFileName);

UINT BeginLightMapCompile( LPVOID pParam )
{
	LightParms * pLight = (LightParms *)pParam;

	g_ambient[0] = GetRValue(pLight->ambientColor);
	g_ambient[1] = GetGValue(pLight->ambientColor);
	g_ambient[2] = GetBValue(pLight->ambientColor);
	g_dSamples = pLight->samples;

	ComPrintf("Ambient Light : %d %d %d\n", g_ambient[0], g_ambient[1],g_ambient[2]);
	ComPrintf("Num Samples : %d\n", g_dSamples);

	CompileLightmaps(pLight->szPath, pLight->szFileName);
	delete pLight;

	((CDevvoidDlg*)AfxGetApp()->GetMainWnd())->EndCompileThread();
	return 0;
}




void Progress_SetRange(int min, int max)
{
	CDevvoidDlg* pDlg = (CDevvoidDlg*)AfxGetApp()->GetMainWnd();
	if(pDlg)
		pDlg->m_progress.SetRange(min,max);
}

void Progress_SetStep(int step)
{
	CDevvoidDlg* pDlg = (CDevvoidDlg*)AfxGetApp()->GetMainWnd();
	if(pDlg)
		pDlg->m_progress.SetStep(step);
}

int Progress_Step()
{
	CDevvoidDlg* pDlg = (CDevvoidDlg*)AfxGetApp()->GetMainWnd();
	if(!pDlg)
		return -1;
	return pDlg->m_progress.StepIt();
}


