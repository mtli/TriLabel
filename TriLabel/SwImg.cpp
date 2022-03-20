#include <new>
#include "SwImg.h"

#undef new

SwImg::SwImg()
{
	m_pGDIPBMP = NULL;
}

SwImg::~SwImg()
{
	if(m_pGDIPBMP)
		::delete m_pGDIPBMP;
}

void SwImg::Clear()
{
	if(m_pGDIPBMP)
	{
		::delete m_pGDIPBMP;
		m_pGDIPBMP = NULL;
	}
}

int SwImg::GetPicWidth()
{
	return m_PicW;
}

int SwImg::GetPicHeight()
{
	return m_PicH;
}

bool SwImg::CVMatToBitmap()
{
	size_t padding;
	if(padding = (m_cvimg.cols & (4 - 1)))
	{// padding is required
		padding = 4 - padding;
		int oldcols = m_cvimg.cols;
		cv::copyMakeBorder(m_cvimg, m_cvimg, 0, 0, 0, padding, cv::BORDER_CONSTANT);
		m_cvimg.cols = oldcols;
	}
	m_pGDIPBMP = ::new Gdiplus::Bitmap(m_cvimg.cols, m_cvimg.rows, m_cvimg.step[0], PixelFormat24bppRGB, (BYTE*)m_cvimg.data);
	if(!m_pGDIPBMP)
	{
		::MessageBox(NULL, _T("Memory allocation failed!"), _T("Error!"), MB_ICONERROR);
		return false;
	}
	return true;
}

bool SwImg::ReadImage(LPCSTR szFileName)
{
	m_cvimg = cv::imread(szFileName);
	if(!m_cvimg.data)
	{
		::MessageBoxA(NULL, szFileName, "Cannot load image!", MB_ICONERROR);
		return false;
	}
	CVMatToBitmap();
	m_PicW = m_pGDIPBMP->GetWidth();
	m_PicH = m_pGDIPBMP->GetHeight();
	return true;
}

bool SwImg::ReadImage(LPCWSTR szFileName)
{
	CHAR ansiFileName[MAX_PATH];
	if(!WideCharToMultiByte(CP_ACP, 0, szFileName, -1, ansiFileName, MAX_PATH, NULL, NULL))
	{
		PopErrorBox(_T("WideCharToMultiByte()"), NULL);
		return false;
	}
	return ReadImage(ansiFileName);
}

bool SwImg::ChangeImage(LPCSTR szFileName)
{
	cv::Mat test;
	test = cv::imread(szFileName);
	if(!test.data)
	{
		::MessageBoxA(NULL, szFileName, "Cannot load image!", MB_ICONERROR);
		return false;
	}
	Clear();
	m_cvimg = test;
	CVMatToBitmap();
	m_PicW = m_pGDIPBMP->GetWidth();
	m_PicH = m_pGDIPBMP->GetHeight();
	return true;
}

bool SwImg::ChangeImage(LPCWSTR szFileName)
{
	CHAR ansiFileName[MAX_PATH];
	if(!WideCharToMultiByte(CP_ACP, 0, szFileName, -1, ansiFileName, MAX_PATH, NULL, NULL))
	{
		PopErrorBox(_T("WideCharToMultiByte()"), NULL);
		return false;
	}
	return ChangeImage(ansiFileName);
}

CSize& SwImg::CLTScaleToImg(CSize &size)
{
	size.cx = MulDiv(size.cx, m_PicW, m_PicRect.Width);
	size.cy = MulDiv(size.cy, m_PicH, m_PicRect.Height);
	return size;
}

CSize& SwImg::ImgScaleToCLT(CSize &size)
{
	size.cx = MulDiv(size.cx, m_PicRect.Width, m_PicW);
	size.cy = MulDiv(size.cy, m_PicRect.Height, m_PicH);
	return size;
}

POINT& SwImg::CLTToImg(POINT &pt)
{
	pt.x = MulDiv(pt.x - m_PicRect.X, m_PicW, m_PicRect.Width);
	pt.y = MulDiv(pt.y - m_PicRect.Y, m_PicH, m_PicRect.Height);
	return pt;
}

POINT& SwImg::ImgToCLT(POINT &pt)
{
	pt.x = MulDiv(pt.x, m_PicRect.Width, m_PicW) + m_PicRect.X;
	pt.y = MulDiv(pt.y, m_PicRect.Height, m_PicH) + m_PicRect.Width;
	return pt;
}

Gdiplus::Rect& SwImg::CLTToImg(Gdiplus::Rect &grect)
{
	grect.X = MulDiv(grect.X - m_PicRect.X, m_PicW, m_PicRect.Width);
	grect.Y = MulDiv(grect.Y - m_PicRect.Y, m_PicH, m_PicRect.Height);
	grect.Width = MulDiv(grect.Width, m_PicW, m_PicRect.Width);
	grect.Height = MulDiv(grect.Height, m_PicH, m_PicRect.Height);
	return grect;
}

void SwImg::CLTToImg(Gdiplus::Rect &grect, FltRect *pfrc)
{
	pfrc->x = (float)(grect.X - m_PicRect.X) * m_PicW / m_PicRect.Width;
	pfrc->y = (float)(grect.Y - m_PicRect.Y) * m_PicH / m_PicRect.Height;
	pfrc->w = (float)grect.Width * m_PicW / m_PicRect.Width;
	pfrc->h = (float)grect.Height * m_PicH / m_PicRect.Height;
}

Gdiplus::Rect& SwImg::ImgToCLT(Gdiplus::Rect &grect)
{
	grect.X = MulDiv(grect.X, m_PicRect.Width, m_PicW) + m_PicRect.X;
	grect.Y = MulDiv(grect.Y, m_PicRect.Height, m_PicH) + m_PicRect.Y;
	grect.Width = MulDiv(grect.Width, m_PicRect.Width, m_PicW);
	grect.Height = MulDiv(grect.Height, m_PicRect.Height, m_PicH);
	return grect;
}

void SwImg::SetClient()
{
	if((unsigned long long)m_cxClient * m_PicH
		== 
		(unsigned long long)m_cyClient * m_PicW)
		// proportional
	{
		m_PicRect.X = 0;
		m_PicRect.Y = 0;
		m_PicRect.Width = m_cxClient;
		m_PicRect.Height = m_cyClient;
	}
	else if((unsigned long long)m_cxClient * m_PicH
		> 
	 (unsigned long long)m_cyClient * m_PicW)
	{ // empty at left and right
		int cxMag = MulDiv(m_PicW, m_cyClient, m_PicH);
		m_PicRect.X = (m_cxClient - cxMag + 1) / 2;
		m_PicRect.Y = 0;
		m_PicRect.Width = cxMag;
		m_PicRect.Height = m_cyClient;
	}
	else
	{ // empty at top and bottom
		int cyMag = MulDiv(m_PicH, m_cxClient, m_PicW);
		m_PicRect.X = 0; 
		m_PicRect.Y = (m_cyClient - cyMag + 1) / 2;
		m_PicRect.Width = m_cxClient;
		m_PicRect.Height = cyMag;
	}
}

void SwImg::SetClient(RECT &rect)
{
	m_cxClient = rect.right - rect.left;
	m_cyClient = rect.bottom - rect.top;
	SetClient();
}

void SwImg::SetClient(UINT cxClient, UINT cyClient)
{
	m_cxClient = cxClient;
	m_cyClient = cyClient;
	SetClient();
}

Gdiplus::Rect& SwImg::GetPicRectInCLT()
{
	return m_PicRect;
}

Gdiplus::Status SwImg::Draw(Gdiplus::Graphics &gr)
{
	return gr.DrawImage(m_pGDIPBMP, m_PicRect);
}

Gdiplus::Status SwImg::DrawBoxPlus(Gdiplus::Graphics &gr, Gdiplus::Rect &grect, Gdiplus::Color c)
{
	Gdiplus::Pen pen(c, BOXWIDTH);
	return gr.DrawRectangle(&pen, grect);
}


bool SwImg::DrawBox(CDC *pDC, CRect &rect, COLORREF c)
{
	pDC->SelectStockObject(HOLLOW_BRUSH);
	CPen pen(PS_INSIDEFRAME, BOXWIDTH, c);
	CPen* pOldPen = pDC->SelectObject(&pen);
	bool bRetv = static_cast<bool>(pDC->Rectangle(&rect));
	pDC->SelectObject(pOldPen);
	return bRetv;
}

bool SwImg::DrawBox(CDC *pDC, CRect &rect, COLORREF c, CRect &resizehandle)
{
	if(!DrawBox(pDC, rect, c))
		return false;
	pDC->SelectStockObject(WHITE_BRUSH);
	CPen pen(PS_INSIDEFRAME, 1, ST_RESIZEHANDLE_COLOR);
	CPen* pOldPen = pDC->SelectObject(&pen);
	bool bRetv = static_cast<bool>(pDC->Rectangle(&resizehandle));
	pDC->SelectObject(pOldPen);
	return bRetv;
}
