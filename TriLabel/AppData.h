#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <afxwin.h>
#include <GdiPlus.h>
#include <strsafe.h>

#include "Common.h"
#include "SwImg.h"

#define DEFAULT_LABELSTATE		(LabelType_P)
#define DEFAULT_LABELSCORE_MIN	(0.0f)
#define DEFAULT_LABELSCORE_MAX	(1.0f)
#define DEFAULT_LABELSCORE_MID	(DEFAULT_LABELSCORE_MIN + (DEFAULT_LABELSCORE_MAX - DEFAULT_LABELSCORE_MIN) / 2.0f)

typedef enum
{
	LabelType_P,	// Positive
	LabelType_N,	// Negtive
	LabelType_A		// Ambiguous
} LabelType;

class LabelEntry
{
public:
	LabelEntry(): lt(DEFAULT_LABELSTATE), score(DEFAULT_LABELSCORE_MAX)
	{
	}

	LabelType lt;
	FltRect rc;
	float score;
};

class AnnotationEntry
{
public:
	std::string filepath;
	std::vector<LabelEntry> labels;
};

class CurrentEntry: public AnnotationEntry
{
public:
	CurrentEntry();
	CurrentEntry(AnnotationEntry &ae);

	void resize(size_t newsize);
	void push_back(CPoint &ptStart, CPoint &ptEnd, SwImg &curImg);
	void push_back(LabelEntry &le, Gdiplus::Rect &roundb, Gdiplus::Rect &clientb, bool bSel = true);
	void push_back(FltRect &fltrc, Gdiplus::Rect &roundb, Gdiplus::Rect &clientb, bool bSel = true);
	void erase(size_t idx);
	void move_box(size_t idx, CSize offset, SwImg &curImg); 
	void resize_box(size_t idx, CSize offset, SwImg &curImg); 
	void finalize_box(size_t idx, SwImg &curImg); 
	void update_resizehandle();
	void update_resizehandle(size_t idx);
	void erase_invisible(int width, int height);

	std::vector<Gdiplus::Rect> roundboxes;
	std::vector<Gdiplus::Rect> clientboxes;
	std::vector<Gdiplus::Rect> resizehandle;
	std::vector<bool> selected;
};

class AppData 
{
public:
	AppData();
	bool Save();
	bool Load();
	bool LoadDetection(PTSTR szFileName);
	bool Output();

	static const Gdiplus::Color m_LabelColor[6];
// Saved:
	UINT m_nFileIdx;
	TCHAR m_szDetectFileName[MAX_PATH];
	std::vector<AnnotationEntry> m_annot;

// Not saved:
	bool m_bSaved;

protected:

};

extern AppData g_data;