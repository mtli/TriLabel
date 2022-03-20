#include "BackBuffer.h"

bool BackBuffer::Init(HDC hDC)
{
	m_ScrWidth = GetSystemMetrics(SM_CXSCREEN);
	m_ScrHeight = GetSystemMetrics(SM_CYSCREEN);
	m_hCptbDC = CreateCompatibleDC(hDC);
	m_hBitmap = CreateCompatibleBitmap(hDC, m_ScrWidth, m_ScrHeight);
	SelectObject(m_hCptbDC, m_hBitmap);
	return true;
}

void BackBuffer::Destroy()
{
	DeleteDC(m_hCptbDC);
	DeleteObject(m_hBitmap);
}

HDC BackBuffer::GetDC()
{
	return m_hCptbDC;
}


bool BackBuffer::ToScreen(HDC hDC)
{
	return static_cast<bool>(BitBlt(hDC, 0, 0, m_ScrWidth, m_ScrHeight, m_hCptbDC, 0, 0, SRCCOPY));
}

bool BackBuffer::ToScreen(HDC hDC, CRect &rect)
{
	return static_cast<bool>(BitBlt(hDC, rect.left, rect.top, rect.Width(), rect.Height(), m_hCptbDC, rect.left, rect.top, SRCCOPY));
}

bool BackBuffer::ToScreen(HDC hDC, Gdiplus::Rect &rect)
{
	return static_cast<bool>(BitBlt(hDC, rect.X, rect.Y, rect.Width, rect.Height, m_hCptbDC, rect.X, rect.Y, SRCCOPY));
}