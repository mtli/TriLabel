// HelpDialog.cpp : 实现文件
//

#include "HelpDialog.h"
#include "AppData.h"

// CHelpDialog 对话框

IMPLEMENT_DYNAMIC(CHelpDialog, CDialog)

CHelpDialog::CHelpDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CHelpDialog::IDD, pParent)
{
	for(size_t i = 0; i < _countof(m_hBrush); i++)
		m_hBrush[i] = CreateSolidBrush(g_data.m_LabelColor[i].ToCOLORREF());
}

CHelpDialog::~CHelpDialog()
{
}

void CHelpDialog::OnOK()
{
	DestroyWindow();
}

void CHelpDialog::OnCancel()
{
	DestroyWindow();
}

void CHelpDialog::PostNcDestroy() 
{
	size_t j = _countof(m_hBrush);
	for(size_t i = 0; i < _countof(m_hBrush); i++)
		DeleteObject(m_hBrush[i]);
	CDialog::PostNcDestroy();
    AfxGetMainWnd()->SendMessage(WM_USER_DIALOG_DESTROYED, 0, 0);
	delete this;
}

void CHelpDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

HBRUSH CHelpDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	int nStaticID = pWnd->GetDlgCtrlID();

	if(nStaticID >= IDC_STATIC_C0
		&& nStaticID <= IDC_STATIC_C5)
	{
		pDC->SetBkColor(g_data.m_LabelColor[(size_t)(nStaticID - IDC_STATIC_C0)].ToCOLORREF());
		return m_hBrush[(size_t)(nStaticID - IDC_STATIC_C0)];
	}

	return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}

BEGIN_MESSAGE_MAP(CHelpDialog, CDialog)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

