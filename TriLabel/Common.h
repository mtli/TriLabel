#pragma once

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <stdlib.h>
#include <math.h>
#include <afxwin.h>
#include <strsafe.h>
#include <Shlwapi.h>
#include <GdiPlus.h>

//#ifdef _DEBUG
//#define new DEBUG_NEW
//#endif
#define WM_USER_DIALOG_DESTROYED	(WM_USER + 0x101)

#include "WinDispError.h"

#include "resource.h"

#include "Settings.h"

inline int Round(float f)
{
  return static_cast<int>(floor(f + 0.5f));
}

inline void CvtCPoints2Rect(CPoint &pt1, CPoint &pt2, Gdiplus::Rect *pgrect)
{
	INT xl = __min(pt1.x, pt2.x);
	INT xr = __max(pt1.x, pt2.x);
	INT yt = __min(pt1.y, pt2.y);
	INT yb = __max(pt1.y, pt2.y);
	pgrect->X = xl;
	pgrect->Y = yt;
	pgrect->Width = xr - xl + 1;
	pgrect->Height = yb - yt + 1;
}

inline void CvtCRect2Rect(CRect &crc, Gdiplus::Rect *pgrect)
{
	pgrect->X = crc.left;
	pgrect->Y = crc.top;
	pgrect->Width = crc.right - crc.left;
	pgrect->Height = crc.bottom - crc.top;
}

inline void CvtRect2CRect(Gdiplus::Rect &grect, CRect *pcrc)
{
	pcrc->left = grect.X;
	pcrc->top = grect.Y;
	pcrc->right = grect.X + grect.Width;
	pcrc->bottom = grect.Y + grect.Height;
}

inline void MakePositive(Gdiplus::Rect &grc)
{
	if(grc.Width < 0)
	{
		grc.X += grc.Width + 1;
		grc.Width = -grc.Width;
	}
	if(grc.Height < 0)
	{
		grc.Y += grc.Height + 1;
		grc.Height = -grc.Height;
	}
}

class FltRect
{
public:
	FltRect();
	FltRect(Gdiplus::Rect &grect);

	void Union(FltRect &fltrc);
	void Intersect(FltRect &fltrc);
	void Offset(float dx, float dy);
	void Resize(float dx, float dy);
	void MakePositive();

	Gdiplus::Rect Round();
	float x, y, w, h;
};