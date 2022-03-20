#include <fstream>

#include <vector>
#include <afxwin.h>
#include <gdiplus.h>
#include <strsafe.h>

#include "Common.h"
#include "AppData.h"
#include "SwImg.h"
#include "CMainWnd.h"


CMainWnd::CMainWnd(): 
	m_ms(MouseState_None), m_pHelpDlg(NULL)
{
	CString strWndClass = AfxRegisterWndClass(
		CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS,
		AfxGetApp()->LoadStandardCursor(IDC_ARROW), 
		(HBRUSH)(COLOR_APPWORKSPACE + 1),
		AfxGetApp()->LoadIcon(MAKEINTRESOURCE(IDI_APPICON))
		);

	LoadAccelTable(MAKEINTRESOURCE(IDR_DEFAULT));
	CreateEx(0, strWndClass, _T ("TriLabel"),
		WS_OVERLAPPEDWINDOW | WS_MAXIMIZE,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL);
}

CMainWnd::~CMainWnd()
{
}

int CMainWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if(CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	if(!m_bb.Init(GetDC()->m_hDC))
	{
		::MessageBox(NULL, _T("Cannot initialize back buffer!"), 
			_T("Fatal Error!"), MB_ICONERROR);
		return -1;
	}
	m_ce = g_data.m_annot[g_data.m_nFileIdx];
	if(!m_curImg.ReadImage(m_ce.filepath.c_str()))
		return -1;
	m_ce.erase_invisible(m_curImg.GetPicWidth(), m_curImg.GetPicHeight());
	CRect rect;
    GetClientRect(&rect);
	FillRect(m_bb.GetDC(), &rect, (HBRUSH)GetStockObject(WHITE_BRUSH));
	Gdiplus::Graphics graphics(m_bb.GetDC());
	m_curImg.SetClient(rect);
	m_ce.clientboxes = m_ce.roundboxes;
	for(size_t i = 0; i < m_ce.clientboxes.size(); i++)
		m_curImg.ImgToCLT(m_ce.clientboxes[i]);
	m_ce.update_resizehandle();
	m_curImg.Draw(graphics);
	UpdateTitleText();
	return 0;
}

void CMainWnd::OnPaint()
{
    CRect rect;
	if(!GetUpdateRect(&rect))
		// GetUpdateRect() must be called before ::BeginPaint()
		// in CPaintDC constructor
		GetClientRect(&rect);
	CPaintDC dc(this);
	Gdiplus::Rect grect;
	CvtCRect2Rect(rect, &grect);
//	Gdiplus::Graphics graphics(dc.m_hDC);
//	graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
	m_bb.ToScreen(dc.m_hDC, rect);
	for(size_t i = 0; i < m_ce.clientboxes.size(); i++)
	{
		CRect cvt1, cvt2;
		CvtRect2CRect(m_ce.clientboxes[i], &cvt1);
		CvtRect2CRect(m_ce.resizehandle[i], &cvt2);
		m_curImg.DrawBox(&dc, cvt1, 
			g_data.m_LabelColor[m_ce.selected[i] ? (m_ce.labels[i].lt + 3): m_ce.labels[i].lt].ToCOLORREF(),
			cvt2);
	}
	dc.SetBkMode(TRANSPARENT);
	dc.SelectStockObject(HOLLOW_BRUSH);
	CRect cvt;
	CvtRect2CRect(m_rectMouseDrag, &cvt);
	CPen pen;
	CPen* pOldPen;
	switch(m_ms)
	{
	case MouseState_GroupSelection:
		pen.CreatePen(PS_DOT, 1, ST_SELBOX_COLOR);
		pOldPen = dc.SelectObject(&pen);
		dc.Rectangle(&cvt);
		dc.SelectObject(pOldPen);
		break;
	case MouseState_AddingBox:
		m_curImg.DrawBox(&dc, cvt, g_data.m_LabelColor[DEFAULT_LABELSTATE].ToCOLORREF());
		break;
	}
}

void CMainWnd::OnSize(UINT nType, int cx, int cy)
{
	CFrameWnd::OnSize(nType, cx, cy);
	m_curImg.SetClient(cx, cy);
	m_ce.clientboxes = m_ce.roundboxes;
	for(size_t i = 0; i < m_ce.clientboxes.size(); i++)
		m_curImg.ImgToCLT(m_ce.clientboxes[i]);
	m_ce.update_resizehandle();
	CRect rect(0, 0, cx, cy);
	FillRect(m_bb.GetDC(), &rect, (HBRUSH)GetStockObject(WHITE_BRUSH));
	Gdiplus::Graphics graphics(m_bb.GetDC());
	m_curImg.Draw(graphics);
}

void CMainWnd::OnLButtonDown(UINT nFlags, CPoint pt)
{
	m_rectMouseDrag.X = pt.x;
	m_rectMouseDrag.Y = pt.y;
	m_rectMouseDrag.Width = 0;
	m_rectMouseDrag.Height = 0;

	m_ptStart = pt;

	if(!(nFlags & MK_CONTROL))
	{// non mulitple selection mode
		for(size_t i = 0; i < m_ce.selected.size(); i++)
		{
			if(m_ce.selected[i])
			{
				CRect cvt;
				CvtRect2CRect(m_ce.clientboxes[i], &cvt);
				InvalidateRect(&cvt, FALSE);
				m_ce.selected[i] = false;
			}
		}
		m_ce.selected.assign(m_ce.selected.size(), false);
	}
	if(!m_curImg.GetPicRectInCLT().Contains(pt.x, pt.y))
	{// Cursor not on the image
		m_ms = MouseState_GroupSelection;
		CRect clip;
		GetClientRect(&clip);
		ClientToScreen(&clip);
		::ClipCursor(clip);
		return;
	}

	if(nFlags & MK_SHIFT)
	{
		CRect clip;
		CvtRect2CRect(m_curImg.GetPicRectInCLT(), &clip);
		ClientToScreen(&clip);
		::ClipCursor(clip);
		ChangeCursorIcon(IDC_CROSS);
		m_ms = MouseState_AddingBox;
		return;
	}

	// checking resize handle selection
	for(size_t i = 0; i < m_ce.resizehandle.size(); i++)
		if(m_ce.resizehandle[i].Contains(pt.x, pt.y))
		{
			m_ms = MouseState_ResizeObject;
			m_ptLast = pt;
			m_uCurSel = i;
			CRect clip;
			CvtRect2CRect(m_curImg.GetPicRectInCLT(), &clip);
			ClientToScreen(&clip);
			::ClipCursor(clip);
			ChangeCursorIcon(IDC_SIZENWSE);
			return;
		}
	
	// checking box selection
	// choose the box that contains the point
	// with min area
	Gdiplus::Rect testing;
	std::vector<unsigned long long> areas(m_ce.clientboxes.size(), 0);
	for(size_t i = 0; i < m_ce.clientboxes.size(); i++)
		if(m_ce.clientboxes[i].Contains(pt.x, pt.y))
			areas[i] = (unsigned long long)m_ce.clientboxes[i].Width * m_ce.clientboxes[i].Height;
	size_t min = -1;
	for(size_t i = 0; i < areas.size(); i++)
		if(areas[i])
			if(min == -1 || areas[min] > areas[i])
				min = i;
	if(min != -1)
	{
		if(!m_ce.selected[min])
		{
			CRect cvt;
			CvtRect2CRect(m_ce.clientboxes[min], &cvt);
			InvalidateRect(&cvt, FALSE);
			m_ce.selected[min] = true;
		}
		m_uCurSel = min;
		m_ptLast = pt;
		CRect clip;
		CvtRect2CRect(m_curImg.GetPicRectInCLT(), &clip);
		ClientToScreen(&clip);
		::ClipCursor(clip);
		m_ms = MouseState_ObjectSelected;
		return;
	}
	CRect clip;
	GetClientRect(&clip);
	ClientToScreen(&clip);
	::ClipCursor(clip);
	m_ms = MouseState_GroupSelection;

}

void CMainWnd::OnLButtonUp(UINT nFlags, CPoint pt)
{
	CRect crect;
	CSize box;
	switch(m_ms)
	{
	case MouseState_GroupSelection:
		// the things are done while moving
		m_ms = MouseState_None;
		CvtRect2CRect(m_rectMouseDrag, &crect);
		InvalidateRect(&crect, FALSE);
		::ClipCursor(NULL);
		break;
	case MouseState_AddingBox:
		m_ms = MouseState_None;
		CvtRect2CRect(m_rectMouseDrag, &crect);
		InvalidateRect(&crect, FALSE);
		box = pt - m_ptStart;
		if(abs(box.cx) >= ST_MIN_BOX_SIZE
			&& abs(box.cy) >= ST_MIN_BOX_SIZE)
		{
			m_ce.push_back(m_ptStart, pt, m_curImg);
			g_data.m_bSaved = false;
		}
		::ClipCursor(NULL);
		ChangeCursorIcon(IDC_ARROW);
		break;

	case MouseState_DragObject:
	case MouseState_ResizeObject:
		ChangeCursorIcon(IDC_ARROW);
		// fall through
	case MouseState_ObjectSelected:	
		m_ms = MouseState_None;
		m_ce.finalize_box(m_uCurSel, m_curImg);
		::ClipCursor(NULL);
		g_data.m_bSaved = false;
		break;
	}
}

void CMainWnd::OnMouseMove(UINT nFlags, CPoint pt)
{
	CRect crect;
	switch(m_ms)
	{
	case MouseState_GroupSelection:
		CvtRect2CRect(m_rectMouseDrag, &crect);
		InvalidateRect(&crect, FALSE);
		CvtCPoints2Rect(m_ptStart, pt, &m_rectMouseDrag);
		CvtRect2CRect(m_rectMouseDrag, &crect);
		InvalidateRect(&crect, FALSE);
		for(size_t i = 0; i < m_ce.clientboxes.size(); i++)
			if(m_ce.selected[i])
			{
				if(!m_rectMouseDrag.Contains(m_ce.clientboxes[i]))
				{
					m_ce.selected[i] = false;
					CRect cvt;
					CvtRect2CRect(m_ce.clientboxes[i], &cvt);
					InvalidateRect(&cvt, FALSE);
				}
			}
			else
			{
				if(m_rectMouseDrag.Contains(m_ce.clientboxes[i]))
					m_ce.selected[i] = true;
			}
		break;
	case MouseState_AddingBox:
		CvtRect2CRect(m_rectMouseDrag, &crect);
		InvalidateRect(&crect, FALSE);
		CvtCPoints2Rect(m_ptStart, pt, &m_rectMouseDrag);
		CvtRect2CRect(m_rectMouseDrag, &crect);
		InvalidateRect(&crect, FALSE);
		break;
	case MouseState_ObjectSelected:
		m_ms = MouseState_DragObject;
		ChangeCursorIcon(IDC_SIZEALL);
		// fall through
	case MouseState_DragObject:
		CvtRect2CRect(m_ce.clientboxes[m_uCurSel], &crect);
		InvalidateRect(&crect, FALSE);
		m_ce.move_box(m_uCurSel, pt - m_ptLast, m_curImg);
		CvtRect2CRect(m_ce.clientboxes[m_uCurSel], &crect);
		InvalidateRect(&crect, FALSE);	
		m_ptLast = pt;
		break;
	case MouseState_ResizeObject:
		CvtRect2CRect(m_ce.clientboxes[m_uCurSel], &crect);
		InvalidateRect(&crect, FALSE);
		m_ce.resize_box(m_uCurSel, pt - m_ptLast, m_curImg);
		if(m_ce.clientboxes[m_uCurSel].Width * m_ce.clientboxes[m_uCurSel].Height < 3)
		{
			int i = 1;
		}
		CvtRect2CRect(m_ce.clientboxes[m_uCurSel], &crect);
		InvalidateRect(&crect, FALSE);	
		m_ptLast = pt;
		break;
	}
}

BOOL CMainWnd::OnMouseWheel(UINT nFlags, short zDelta, CPoint point)
{
	if(zDelta > 0)
		SendMessage(WM_COMMAND, MAKEWPARAM(ID_PREVIOUS, 0), 0);
	else SendMessage(WM_COMMAND, MAKEWPARAM(ID_NEXT, 0), 0);
	return FALSE;
}

void CMainWnd::OnClose()
{
	if(!g_data.m_bSaved)
	{
		int res = MessageBox(
			_T("Do you to save before closing?"),
			AfxGetAppName(),
			MB_YESNOCANCEL);
		if((res == IDYES && !Save())
			|| res == IDCANCEL)
			return;
	}
	return CFrameWnd::OnClose();
}

BOOL CMainWnd::OnQueryEndSession()
{
	if(!g_data.m_bSaved)
	{
		int res = MessageBox(
			_T("Do you to save before closing?"),
			AfxGetAppName(),
			MB_YESNOCANCEL);
		if((res == IDYES && !Save())
			|| res == IDCANCEL)
			return FALSE;
	}
	return TRUE;
}

void CMainWnd::PostNcDestroy()
{
	CFrameWnd::PostNcDestroy();
}

void CMainWnd::OnCmdHelp()
{
	if(m_pHelpDlg)
		m_pHelpDlg->SetFocus();
	else 
	{
		m_pHelpDlg = new CHelpDialog;
		m_pHelpDlg->Create(IDD_HELP);
		m_pHelpDlg->ShowWindow(SW_SHOW);
	}
}

void CMainWnd::OnCmdPrevious()
{
	if(!g_data.m_nFileIdx 
		|| !ChangeImage(g_data.m_nFileIdx - 1))
	{
		FlashWindow(TRUE);
		::MessageBeep(MB_ICONERROR);
		return;
	}
	Invalidate();
}

void CMainWnd::OnCmdNext()
{
	if(g_data.m_nFileIdx + 1 == g_data.m_annot.size())
	{
		if(IDYES == AfxMessageBox(
			_T("Good job! You have annotated all the images. Output the annotation now and quit?"),
			MB_YESNO | MB_DEFBUTTON1))
		{
			Save();
			if(!g_data.Output())
				return;
			SendMessage(WM_CLOSE);
		}
		return;
	}
	if(!ChangeImage(g_data.m_nFileIdx + 1))
	{
		FlashWindow(TRUE);
		::MessageBeep(MB_ICONERROR);
		return;
	}
	Invalidate();
}

void CMainWnd::OnCmdSave()
{
	if(Save())
	{
		MessageBox(_T("Save successful!"));
	}
}

void CMainWnd::OnCmdLoad()
{
	if(g_data.Load())
	{
		MessageBox(_T("Load successful!"));
	}
}

void CMainWnd::OnCmdOutput()
{
	if(g_data.Output())
	{
		MessageBox(_T("Output successful!"));
	}
}

void CMainWnd::OnCmdLabel(UINT nID)
{
	LabelType lt = (LabelType)(nID -  ID_LABELTYPE1);
	for(size_t i = 0; i < m_ce.selected.size(); i++)
		if(m_ce.selected[i])
		{
			m_ce.labels[i].lt = lt;
			CRect cvt;
			CvtRect2CRect(m_ce.clientboxes[i], &cvt);
			InvalidateRect(&cvt, FALSE);
			g_data.m_bSaved = false;
		}
}

void CMainWnd::OnCmdDelete()
{
	for(size_t i = 0; i < m_ce.selected.size();)
		if(m_ce.selected[i])
		{
			CRect cvt;
			CvtRect2CRect(m_ce.clientboxes[i], &cvt);
			InvalidateRect(&cvt, FALSE);
			m_ce.erase(i);
		}
		else i++;
}

void CMainWnd::OnCmdSelectAll()
{
	for(size_t i = 0; i < m_ce.selected.size();i++)
		if(!m_ce.selected[i])
		{
			m_ce.selected[i] = true;
			CRect cvt;
			CvtRect2CRect(m_ce.clientboxes[i], &cvt);
			InvalidateRect(&cvt, FALSE);
		}
}

void CMainWnd::OnCmdUnionMerge()
{
	size_t nLast, nMerge = 0;
	Gdiplus::Rect resultClt;
	Gdiplus::Rect resultRound;
	FltRect resultFlt;
	for(size_t i = 0; i < m_ce.selected.size(); i++)
		if(m_ce.selected[i])
		{
			if(!nMerge)
			{
				resultClt = m_ce.clientboxes[i];
				resultRound = m_ce.roundboxes[i];
				resultFlt = m_ce.labels[i].rc;
				nLast = i;
				nMerge = 1;
				continue;
			}
			resultClt.Union(resultClt, resultClt, m_ce.clientboxes[i]);
			resultRound.Union(resultRound, resultRound, m_ce.roundboxes[i]);
			resultFlt.Union(m_ce.labels[i].rc);
			if(nMerge == 1)
			{
				CRect cvt;
				CvtRect2CRect(m_ce.clientboxes[nLast], &cvt);
				InvalidateRect(&cvt, FALSE);
				m_ce.erase(nLast);
				i--;
			}
			CRect cvt;
			CvtRect2CRect(m_ce.clientboxes[i], &cvt);
			InvalidateRect(&cvt, FALSE);
			m_ce.erase(i);
			i--;
			nMerge++;
		}
	if(nMerge > 1)
	{
		m_ce.push_back(resultFlt, resultRound, resultClt);
		CRect cvt;
		CvtRect2CRect(resultClt, &cvt);
		InvalidateRect(&cvt, FALSE);
		g_data.m_bSaved = false;
	}
}

void CMainWnd::OnCmdIntersectMerge()
{
	size_t nLast, nMerge = 0;
	BOOL bNotEmtpy = FALSE;
	Gdiplus::Rect resultClt;
	Gdiplus::Rect resultRound;
	FltRect resultFlt;
	for(size_t i = 0; i < m_ce.selected.size(); i++)
		if(m_ce.selected[i])
		{
			if(!nMerge)
			{
				resultClt = m_ce.clientboxes[i];
				resultRound = m_ce.roundboxes[i];
				resultFlt = m_ce.labels[i].rc;
				nLast = i;
				nMerge = 1;
				continue;
			}
			bNotEmtpy = resultClt.Intersect(m_ce.clientboxes[i]);
			resultRound.Intersect(m_ce.roundboxes[i]);
			resultFlt.Intersect(m_ce.labels[i].rc);
			if(nMerge == 1)
			{
				CRect cvt;
				CvtRect2CRect(m_ce.clientboxes[nLast], &cvt);
				InvalidateRect(&cvt, FALSE);
				m_ce.erase(nLast);
				i--;
			}
			CRect cvt;
			CvtRect2CRect(m_ce.clientboxes[i], &cvt);
			InvalidateRect(&cvt, FALSE);
			m_ce.erase(i);
			i--;
			nMerge++;
		}
	if(bNotEmtpy)
	{
		m_ce.push_back(resultFlt, resultRound, resultClt);
		g_data.m_bSaved = false;

	}
}

bool CMainWnd::Save()
{
	g_data.m_annot[g_data.m_nFileIdx] = m_ce;
	return g_data.Save();
}

bool CMainWnd::ChangeImage(size_t idx)
{
	m_ce.erase_invisible(m_curImg.GetPicWidth(), m_curImg.GetPicHeight());
	g_data.m_annot[g_data.m_nFileIdx] = m_ce;
	if(!m_curImg.ChangeImage(g_data.m_annot[idx].filepath.c_str()))
		return false;
	m_ce = g_data.m_annot[idx];
	m_ce.erase_invisible(m_curImg.GetPicWidth(), m_curImg.GetPicHeight());
	g_data.m_nFileIdx = idx;
	g_data.m_bSaved = false;
	CRect rect;
    GetClientRect(&rect);
	m_curImg.SetClient(rect);
	FillRect(m_bb.GetDC(), &rect, (HBRUSH)GetStockObject(WHITE_BRUSH));
	Gdiplus::Graphics graphics(m_bb.GetDC());
	m_curImg.Draw(graphics);
	m_ce.clientboxes = m_ce.roundboxes;
	for(size_t i = 0; i < m_ce.clientboxes.size(); i++)
		m_curImg.ImgToCLT(m_ce.clientboxes[i]);
	m_ce.update_resizehandle();
	UpdateTitleText();
	return true;
}

bool CMainWnd::ChangeCursorIcon(LPTSTR szCursorName)
{
	HCURSOR hCursor = LoadCursor(NULL, szCursorName);
	if(!hCursor)
		return false;
	::SetClassLongPtr(m_hWnd, GCL_HCURSOR, (LONG_PTR)hCursor);
	return true;
}

void CMainWnd::UpdateTitleText()
{
#ifdef UNICODE
	const size_t newsizew = strlen(m_ce.filepath.c_str()) + 1;
    size_t convertedChars = 0;
    wchar_t *filepath = new wchar_t[newsizew];
    mbstowcs_s(&convertedChars, filepath, newsizew, m_ce.filepath.c_str(), _TRUNCATE);
#else
	PSTR filepath = m_ce.filepath.c_str();
#endif
	CString title;
	title.Format(_T("TriLabel - %u / %u - %s - %u x %u"),
		g_data.m_nFileIdx + 1,
		g_data.m_annot.size(),
		PathFindFileName(filepath),
		m_curImg.GetPicWidth(),
		m_curImg.GetPicHeight());
	SetWindowText(title);
#ifdef UNICODE
	delete filepath;
#endif
}

LRESULT CMainWnd::OnDialogDestroyed(WPARAM wParam, LPARAM lParam)
{
	m_pHelpDlg = NULL;
	return 0;
}

