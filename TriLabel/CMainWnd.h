#pragma once

#include <vector>

#include "Common.h"
#include "BackBuffer.h"
#include "SwImg.h"
#include "HelpDialog.h"

class CMainWnd: public CFrameWnd
{
public: 
	CMainWnd();
	~CMainWnd();
	virtual void CMainWnd::PostNcDestroy();

protected:
	typedef enum
	{
		MouseState_None,
		MouseState_GroupSelection,
		MouseState_AddingBox,
		MouseState_ObjectSelected,
		MouseState_DragObject,
		MouseState_ResizeObject
	} MouseState;

	MouseState m_ms;
	CPoint m_ptStart, m_ptLast;
	Gdiplus::Rect m_rectMouseDrag;

	BackBuffer m_bb;
	SwImg m_curImg;
	CurrentEntry m_ce;
	size_t m_uCurSel;

	CHelpDialog *m_pHelpDlg;

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnClose();	
	afx_msg BOOL OnQueryEndSession();
	
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);

	afx_msg void OnLButtonDown(UINT nFlags, CPoint pt);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint pt);
	afx_msg void OnMouseMove(UINT nFlags, CPoint pt);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint point);
	afx_msg void OnCmdHelp();
	afx_msg void OnCmdPrevious();
	afx_msg void OnCmdNext();
	afx_msg void OnCmdSave();
	afx_msg void OnCmdLoad();
	afx_msg void OnCmdOutput();
	afx_msg void OnCmdLabel(UINT nID);
	afx_msg void OnCmdSelectAll();
	afx_msg void OnCmdDelete();
	afx_msg void OnCmdUnionMerge();
	afx_msg void OnCmdIntersectMerge();

	afx_msg LRESULT OnDialogDestroyed(WPARAM wParam, LPARAM lParam);
	bool Save();
	bool ChangeImage(size_t idx);
	bool ChangeCursorIcon(LPTSTR szCursorName);
	void UpdateTitleText();
	DECLARE_MESSAGE_MAP()
};