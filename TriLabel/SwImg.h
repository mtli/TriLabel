// Show Image Class, image related

#pragma once
#include <afxwin.h>
#include <gdiplus.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "Common.h"

#define BOXWIDTH	3

class SwImg
{
public:
	SwImg();
	~SwImg();

	void Clear();

	int GetPicWidth();
	int GetPicHeight();
	bool ReadImage(LPCSTR szFileName);
	bool ReadImage(LPCWSTR szFileName);
	bool ChangeImage(LPCSTR szFileName);
	bool ChangeImage(LPCWSTR szFileName);
	void SetClient();
	void SetClient(UINT cxClient, UINT cyClient);
	void SetClient(RECT &rect);
	Gdiplus::Rect& GetPicRectInCLT();

	POINT& CLTToImg(POINT &pt);
	CSize& CLTScaleToImg(CSize &size);
	Gdiplus::Rect& CLTToImg(Gdiplus::Rect &grect);
	void CLTToImg(Gdiplus::Rect &grect, FltRect *pfrc);

	POINT& ImgToCLT(POINT &pt);
	CSize& ImgScaleToCLT(CSize &size);
	Gdiplus::Rect& ImgToCLT(Gdiplus::Rect &grect);

	Gdiplus::Status Draw(Gdiplus::Graphics &gr);
	bool DrawBox(CDC *pDC, CRect &rect, COLORREF c);
	bool DrawBox(CDC *pDC, CRect &rect, COLORREF c, CRect &resizehandle);
	Gdiplus::Status DrawBoxPlus(Gdiplus::Graphics &gr, Gdiplus::Rect &grect, Gdiplus::Color c);

protected:
	bool CVMatToBitmap();
	Gdiplus::Bitmap	*m_pGDIPBMP;
	cv::Mat m_cvimg;
	Gdiplus::Rect m_PicRect;
	int m_PicW, m_PicH;
	int m_cxClient, m_cyClient;
	CPen m_Pen;
};