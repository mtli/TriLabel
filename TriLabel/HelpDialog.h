#pragma once

#include <afxdialogex.h>

#include "Common.h"
// CHelpDialog �Ի���

class CHelpDialog : public CDialog
{
	DECLARE_DYNAMIC(CHelpDialog)

public:
	CHelpDialog(CWnd* pParent = NULL);
	virtual ~CHelpDialog();

// �Ի�������
	enum { IDD = IDD_HELP };

protected:
	virtual void OnOK();
	virtual void OnCancel();
	virtual void PostNcDestroy();
	virtual void DoDataExchange(CDataExchange* pDX);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

	HBRUSH m_hBrush[6];
	DECLARE_MESSAGE_MAP()
};
