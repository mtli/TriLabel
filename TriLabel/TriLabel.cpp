#include <fstream>
#include <afxwin.h>
#include <gdiplus.h>

#include "Common.h"
#include "AppData.h"
#include "CMainWnd.h"
#include "TriLabel.h"


TriLabel thisinst;


BEGIN_MESSAGE_MAP(CMainWnd, CFrameWnd)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_WM_QUERYENDSESSION()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_COMMAND(ID_HELP, OnCmdHelp)
	ON_COMMAND(ID_PREVIOUS, OnCmdPrevious)
	ON_COMMAND(ID_NEXT, OnCmdNext)
	ON_COMMAND(ID_SAVEPROGRESS, OnCmdSave)
	ON_COMMAND(ID_LOADPROGRESS, OnCmdLoad)
	ON_COMMAND(ID_OUTPUTRESULT, OnCmdOutput)
	ON_COMMAND_RANGE(ID_LABELTYPE1, ID_LABELTYPE3, OnCmdLabel)
	ON_COMMAND(ID_SELECTALL, OnCmdSelectAll)
	ON_COMMAND(ID_DELETE, OnCmdDelete)
	ON_COMMAND(ID_UNIONMERGE, OnCmdUnionMerge)
	ON_COMMAND(ID_INTERSECTMERGE, OnCmdIntersectMerge)
	ON_MESSAGE(WM_USER_DIALOG_DESTROYED, OnDialogDestroyed)
END_MESSAGE_MAP()

PTSTR OpenTextDlg()
{
	OPENFILENAME ofn = {0};
	static TCHAR szFilter[] =	TEXT("Text Files (*.txt)\0*.txt\0")
								TEXT("All Files (*.*)\0*.*\0\0");
	ofn.lStructSize       = sizeof(OPENFILENAME) ;
	ofn.hwndOwner         = NULL;
	ofn.hInstance         = NULL;
	ofn.lpstrFilter       = szFilter;
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter    = 0;
	ofn.nFilterIndex      = 0;
	ofn.lpstrFile         = NULL;
	ofn.nMaxFile          = MAX_PATH ;
	ofn.lpstrFileTitle    = NULL;
	ofn.nMaxFileTitle     = MAX_PATH ;
	ofn.lpstrInitialDir   = NULL;
	ofn.lpstrTitle        = NULL;
	ofn.Flags             = 0;
	ofn.nFileOffset       = 0;
	ofn.nFileExtension    = 0;
	ofn.lpstrDefExt       = TEXT("txt") ;
	ofn.lCustData         = 0L;
	ofn.lpfnHook          = NULL;
	ofn.lpTemplateName    = NULL;

	PTSTR pszFullPathName = new TCHAR[MAX_PATH];
	PTSTR pszFileName = new TCHAR[MAX_PATH];

	memset(pszFullPathName, 0, sizeof(MAX_PATH));
	memset(pszFileName, 0, sizeof(MAX_PATH));

	ofn.lpstrFile         = pszFullPathName;
	ofn.lpstrFileTitle    = pszFileName;
	ofn.Flags             = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_EXPLORER;
	if(GetOpenFileName(&ofn))
	{
		delete pszFileName;
		return pszFullPathName;
	}
	else
	{
		delete pszFullPathName;
		delete pszFileName;
		return NULL;
	}
}

BOOL TriLabel::InitInstance()
{
	if(Gdiplus::Ok != Gdiplus::GdiplusStartup(&m_gdiplusToken, &m_gdiplusStartupInput, NULL))
	{
		::MessageBox(NULL, _T("GDI+ failed to initialize!"),
			m_pszAppName, MB_ICONERROR);
		return FALSE;
	}
	if(IDNO == AfxMessageBox(_T("Load new detection file? Press 'No' to load previous work."), MB_YESNO | MB_ICONQUESTION))
	{
		if(!g_data.Load())
			return FALSE;
	}
	else
	{
		g_data.m_nFileIdx = 0;
		PTSTR szDetectFileName; 
		if(!(szDetectFileName = OpenTextDlg()))
			return FALSE;
		if(!g_data.LoadDetection(szDetectFileName))
			return FALSE;
		delete szDetectFileName;
	}
	m_pMainWnd = new CMainWnd;
	m_pMainWnd->ShowWindow(SW_SHOWMAXIMIZED);
	m_pMainWnd->UpdateWindow();
	return TRUE;
}

int TriLabel::ExitInstance()
{
	Gdiplus::GdiplusShutdown(m_gdiplusToken);
	return CWinApp::ExitInstance();
}

