#pragma once

class TriLabel: public CWinApp
{
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

protected:
	Gdiplus::GdiplusStartupInput m_gdiplusStartupInput;
	ULONG_PTR m_gdiplusToken;
};
