// Note: this class is optimized for screen stays constant resolution
// Changing to a bigger resolution might result in unexpected display

#pragma once

#include <afxwin.h>
#include <GdiPlus.h>

class BackBuffer
{
public:
	bool Init(HDC hDC);
	void Destroy();

	HDC GetDC();

	bool ToScreen(HDC hDC);
	bool ToScreen(HDC hDC, CRect &rect);
	bool ToScreen(HDC hDC, Gdiplus::Rect &rect);


protected:
	HDC m_hCptbDC;
	HBITMAP m_hBitmap;
	UINT m_ScrWidth;
	UINT m_ScrHeight;
};