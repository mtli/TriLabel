#include "AppData.h"

#define SAVEFILENAME	(_T("TriLabel.save"))

AppData g_data;

const Gdiplus::Color AppData::m_LabelColor[6] =
{
	ST_LABEL_COLORS
};

FltRect::FltRect()
{
}

FltRect::FltRect(Gdiplus::Rect &grect)
{
	x = static_cast<float>(grect.X);
	y = static_cast<float>(grect.Y);
	w = static_cast<float>(grect.Width);
	h = static_cast<float>(grect.Height);
}

void FltRect::Union(FltRect &fltrc)
{
	w = __max(x + w, fltrc.x + fltrc.w);
	h = __max(y + h, fltrc.y + fltrc.h);
	x = __min(x, fltrc.x);
	y = __min(y, fltrc.y);
	w -= x;
	h -= y;
}

void FltRect::Intersect(FltRect &fltrc)
{
	w = __min(x + w, fltrc.x + fltrc.w);
	h = __min(y + h, fltrc.y + fltrc.h);
	x = __max(x, fltrc.x);
	y = __max(y, fltrc.y);
	w -= x;
	h -= y;
}

void FltRect::Offset(float dx, float dy)
{
	x += dx;
	y += dy;
}

void FltRect::Resize(float dx, float dy)
{
	w += dx;
	h += dy;
	MakePositive();
}

void FltRect::MakePositive()
{
	if(w < 0)
	{
		x = x + w + 1;
		w = -w;
	}
	if(h < 0)
	{
		y = y + h + 1;
		h = -h;
	}
}

Gdiplus::Rect FltRect::Round()
{
	Gdiplus::Rect rc(::Round(x), ::Round(y), ::Round(w), ::Round(h));
	return rc;
}

CurrentEntry::CurrentEntry()
{
}

CurrentEntry::CurrentEntry(AnnotationEntry &ae)
{
	filepath = ae.filepath;
	labels = ae.labels;
	roundboxes.resize(labels.size());
	clientboxes.resize(labels.size());
	resizehandle.resize(labels.size());
	selected.resize(labels.size());
	for(size_t i = 0; i < labels.size(); i++)
		roundboxes[i] = labels[i].rc.Round();
}

void CurrentEntry::resize(size_t newsize)
{
	labels.resize(newsize);
	roundboxes.resize(newsize);
	clientboxes.resize(newsize);
	resizehandle.resize(newsize);
	selected.resize(newsize);
}

void CurrentEntry::push_back(LabelEntry &le, Gdiplus::Rect &roundb, Gdiplus::Rect &clientb, bool bSel)
{
	labels.push_back(le);
	roundboxes.push_back(roundb);
	clientboxes.push_back(clientb);
	size_t idx = resizehandle.size();
	resizehandle.resize(idx + 1);
	update_resizehandle(idx);
	selected.push_back(bSel);
}

void CurrentEntry::push_back(FltRect &fltrc, Gdiplus::Rect &roundb, Gdiplus::Rect &clientb, bool bSel)
{
	LabelEntry le;
	le.rc = fltrc;
	push_back(le, roundb, clientb, bSel);
}

void CurrentEntry::push_back(CPoint &ptStart, CPoint &ptEnd, SwImg &curImg)
{
	Gdiplus::Rect rect;
	CvtCPoints2Rect(ptStart, ptEnd, &rect);
	clientboxes.push_back(rect);
	size_t idx = resizehandle.size();
	resizehandle.resize(idx + 1);
	update_resizehandle(idx);
	curImg.CLTToImg(rect);
	roundboxes.push_back(rect);
	LabelEntry le;
	le.rc = rect;
	labels.push_back(le);
	selected.push_back(true);
}

void CurrentEntry::erase(size_t idx)
{
	labels.erase(labels.begin() + idx);
	roundboxes.erase(roundboxes.begin() + idx);
	clientboxes.erase(clientboxes.begin() + idx);
	resizehandle.erase(resizehandle.begin() + idx);
	selected.erase(selected.begin() + idx);
	g_data.m_bSaved = false;
}

void CurrentEntry::move_box(size_t idx, CSize offset, SwImg &curImg)
{
	clientboxes[idx].Offset(offset.cx, offset.cy);
	update_resizehandle(idx);
}

void CurrentEntry::resize_box(size_t idx, CSize offset, SwImg &curImg)
{
	clientboxes[idx].Width = __max(ST_RESIZEHANDLE_SIZE, clientboxes[idx].Width + offset.cx);
	clientboxes[idx].Height = __max(ST_RESIZEHANDLE_SIZE, clientboxes[idx].Height + offset.cy);
	update_resizehandle(idx);
}

void CurrentEntry::finalize_box(size_t idx, SwImg &curImg)
{
	update_resizehandle(idx);
	roundboxes[idx] = clientboxes[idx];
	curImg.CLTToImg(roundboxes[idx]);
	curImg.CLTToImg(clientboxes[idx], &labels[idx].rc);
}

void CurrentEntry::update_resizehandle()
{
	for(size_t i = 0; i < resizehandle.size(); i++)
		update_resizehandle(i);
}

void CurrentEntry::update_resizehandle(size_t idx)
{
	resizehandle[idx].X = 
		__max(clientboxes[idx].GetLeft(), clientboxes[idx].GetRight())
		- ST_RESIZEHANDLE_SIZE;
	resizehandle[idx].Y = 
		__max(clientboxes[idx].GetBottom(), clientboxes[idx].GetTop())
		- ST_RESIZEHANDLE_SIZE;
	resizehandle[idx].Width = ST_RESIZEHANDLE_SIZE;
	resizehandle[idx].Height = ST_RESIZEHANDLE_SIZE;
}

void CurrentEntry::erase_invisible(int width, int height)
{
	FltRect picrc;
	picrc.x = 0;
	picrc.y = 0;
	picrc.w = width;
	picrc.h = height;
	for(size_t i = 0 ; i < labels.size();)
	{
		if((unsigned long long)labels[i].rc.w
			* labels[i].rc.h < ST_MIN_LABEL_AREA)
			erase(i);
		else
		{
			FltRect tmp = labels[i].rc;
			tmp.Intersect(picrc);
			if(tmp.w <= 0 || tmp.h <= 0)
				erase(i);
			else 
			{
				labels[i].rc = tmp;
				roundboxes[i] = tmp.Round();
				i++;
			}
		}
	}
}

AppData::AppData(): m_nFileIdx(0), m_bSaved(false)
{
}

PTSTR SelectSaveDlg()
{
	OPENFILENAME ofn = {0};
	static TCHAR szFilter[] =	TEXT("TriLabel Saves (*.saves)\0*.saves\0")
								TEXT("All Files (*.*)\0*.*\0\0");
	ofn.lStructSize       = sizeof(OPENFILENAME) ;
	ofn.hwndOwner         = NULL;
	ofn.hInstance         = NULL;
	ofn.lpstrFilter       = szFilter;
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter    = 0;
	ofn.nFilterIndex      = 0;
	ofn.lpstrFile         = NULL;
	ofn.nMaxFile          = MAX_PATH ;
	ofn.lpstrFileTitle    = NULL;
	ofn.nMaxFileTitle     = MAX_PATH ;
	ofn.lpstrInitialDir   = NULL;
	ofn.lpstrTitle        = NULL;
	ofn.Flags             = 0;
	ofn.nFileOffset       = 0;
	ofn.nFileExtension    = 0;
	ofn.lpstrDefExt       = TEXT("txt") ;
	ofn.lCustData         = 0L;
	ofn.lpfnHook          = NULL;
	ofn.lpTemplateName    = NULL;

	PTSTR pszFullPathName = new TCHAR[MAX_PATH];
	PTSTR pszFileName = new TCHAR[MAX_PATH];

	StringCchCopy(pszFullPathName, MAX_PATH, _T("TriLabel.saves"));
	StringCchCopy(pszFileName, MAX_PATH, _T("TriLabel.saves"));

	ofn.lpstrFile         = pszFullPathName;
	ofn.lpstrFileTitle    = pszFileName;
	ofn.Flags             = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_EXPLORER;
	if(GetOpenFileName(&ofn))
	{
		delete pszFileName;
		return pszFullPathName;
	}
	else
	{
		delete pszFullPathName;
		delete pszFileName;
		return NULL;
	}
}

bool AppData::Save()
{
	PTSTR szFileName = SelectSaveDlg();
	if(!szFileName)
		return false;
	std::ofstream outfile(szFileName);
	if(!outfile)
	{
		::MessageBox(NULL, szFileName, 
			_T("Error: Cannot open file for writing!"), MB_ICONERROR);
		return false;
	}
	outfile<<m_nFileIdx<<' '<<m_annot.size()<<'\n';
#ifdef UNICODE
	CHAR ansiFileName[MAX_PATH];
	if(!WideCharToMultiByte(CP_ACP, 0, m_szDetectFileName, -1, ansiFileName, MAX_PATH, NULL, NULL))
	{
		PopErrorBox(_T("WideCharToMultiByte()"), NULL);
		outfile.close();
		return false;
	}
	outfile<<ansiFileName<<'\n';
#else
	outfile<<m_szDetectFileName<<'\n';
#endif
	for(size_t i = 0; i < m_annot.size(); i++)
	{
		outfile<<m_annot[i].filepath<<'\n'<<m_annot[i].labels.size()<<'\n';
		for(size_t j = 0; j < m_annot[i].labels.size(); j++)
			outfile<<(size_t)m_annot[i].labels[j].lt<<' '
			<<m_annot[i].labels[j].rc.x<<' '
			<<m_annot[i].labels[j].rc.y<<' '
			<<m_annot[i].labels[j].rc.w<<' '
			<<m_annot[i].labels[j].rc.h<<' '
			<<m_annot[i].labels[j].score<<'\n';
	}
	outfile.close();
	delete szFileName;
	m_bSaved = true;
	return true;
}

PTSTR SelectLoadDlg()
{
	OPENFILENAME ofn = {0};
	static TCHAR szFilter[] =	TEXT("TriLabel Saves (*.saves)\0*.saves\0")
								TEXT("All Files (*.*)\0*.*\0\0");
	ofn.lStructSize       = sizeof(OPENFILENAME) ;
	ofn.hwndOwner         = NULL;
	ofn.hInstance         = NULL;
	ofn.lpstrFilter       = szFilter;
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter    = 0;
	ofn.nFilterIndex      = 0;
	ofn.lpstrFile         = NULL;
	ofn.nMaxFile          = MAX_PATH ;
	ofn.lpstrFileTitle    = NULL;
	ofn.nMaxFileTitle     = MAX_PATH ;
	ofn.lpstrInitialDir   = NULL;
	ofn.lpstrTitle        = NULL;
	ofn.Flags             = 0;
	ofn.nFileOffset       = 0;
	ofn.nFileExtension    = 0;
	ofn.lpstrDefExt       = TEXT("txt") ;
	ofn.lCustData         = 0L;
	ofn.lpfnHook          = NULL;
	ofn.lpTemplateName    = NULL;

	PTSTR pszFullPathName = new TCHAR[MAX_PATH];
	PTSTR pszFileName = new TCHAR[MAX_PATH];

	StringCchCopy(pszFullPathName, MAX_PATH, _T("TriLabel.saves"));
	StringCchCopy(pszFileName, MAX_PATH, _T("TriLabel.saves"));

	ofn.lpstrFile         = pszFullPathName;
	ofn.lpstrFileTitle    = pszFileName;
	ofn.Flags             = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_EXPLORER;
	if(GetOpenFileName(&ofn))
	{
		delete pszFileName;
		return pszFullPathName;
	}
	else
	{
		delete pszFullPathName;
		delete pszFileName;
		return NULL;
	}
}

bool AppData::Load()
{
	PTSTR szFileName = SelectLoadDlg();
	if(!szFileName)
		return false;
	std::ifstream infile(szFileName);
	if(!infile)
	{
		::MessageBox(NULL, szFileName, 
			_T("Error: Cannot open file for reading!"), MB_ICONERROR);
		return false;
	}
	size_t cnt;
	infile>>m_nFileIdx>>cnt;
	m_annot.resize(cnt);
	std::string buf;
	std::getline(infile, buf); // the line feed
	std::getline(infile, buf);
#ifdef UNICODE
	if(!MultiByteToWideChar(CP_ACP, 0, buf.c_str(), -1, m_szDetectFileName, MAX_PATH))
	{
		PopErrorBox(_T("MultiByteToWideChar()"), NULL);
		infile.close();
		return false;
	}
#else
	StringCchCopy(m_szDetectFileName, MAX_PATH, buf.c_str());
#endif
	size_t i = 0;
	std::string filepath;
	while(std::getline(infile, filepath))
	{
		if(filepath.empty())
			continue;
		m_annot[i].filepath = filepath;
		size_t cnt;
		infile>>cnt;
		m_annot[i].labels.resize(cnt);
		size_t tmp;
		for(size_t j = 0; j < cnt; j++)
		{
			infile>>tmp>>
				m_annot[i].labels[j].rc.x>>
				m_annot[i].labels[j].rc.y>>
				m_annot[i].labels[j].rc.w>>
				m_annot[i].labels[j].rc.h>>
				m_annot[i].labels[j].score;
			m_annot[i].labels[j].lt = (LabelType)tmp;
		}
		i++;
	}
	assert(i == m_annot.size());
	infile.close();
	delete szFileName;
	m_bSaved = true;
	return true;
}

bool AppData::LoadDetection(PTSTR szFileName)
{
	std::ifstream detectfile(szFileName);
	if(!detectfile)
	{
		::MessageBox(NULL, _T("Cannot open file!"),
		szFileName, MB_ICONERROR);
		return false;
	}
	std::string filepath;
	if(!std::getline(detectfile, filepath))
	{
		detectfile.close();
		::MessageBox(NULL, _T("This file is empty"),
		szFileName, MB_ICONINFORMATION);
		return false;
	}
	AnnotationEntry ae;
	if(filepath == ST_OUTPUT_MARKING)
	{// TriLabel format
		while(std::getline(detectfile, ae.filepath))
		{
			if(ae.filepath.empty())
				continue;
			size_t cnt;
			detectfile>>cnt;
			ae.labels.resize(cnt);
			size_t tmp;
			for(size_t i = 0; i < cnt; i++)
			{
				detectfile>>tmp>>
					ae.labels[i].rc.x>>
					ae.labels[i].rc.y>>
					ae.labels[i].rc.w>>
					ae.labels[i].rc.h>>
					ae.labels[i].score;
				ae.labels[i].lt = (LabelType)tmp;
			}
			m_annot.push_back(ae);
		}
	}
	else
	{// fddb format
		do
		{
			if(filepath.empty())
				continue;
			ae.filepath = filepath + ST_FDDB_FORMAT_EXT;
			size_t cnt;
			detectfile>>cnt;
			ae.labels.resize(cnt);
			for(size_t i = 0; i < cnt; i++)
			{
				detectfile>>
					ae.labels[i].rc.x>>
					ae.labels[i].rc.y>>
					ae.labels[i].rc.w>>
					ae.labels[i].rc.h>>
					ae.labels[i].score;
			}
			m_annot.push_back(ae);
		}
		while(std::getline(detectfile, filepath));
	}
	detectfile.close();
	StringCchCopy(m_szDetectFileName, MAX_PATH, szFileName);
	if(!m_annot.size())
	{
		::MessageBox(NULL, _T("File contains invalid content!"),
		szFileName, MB_ICONERROR);
		return false;
	}
	return true;
}

PTSTR SelectOutputDlg()
{
	OPENFILENAME ofn = {0};
	static TCHAR szFilter[] =	TEXT("Text Files (*.txt)\0*.txt\0")
								TEXT("All Files (*.*)\0*.*\0\0");
	ofn.lStructSize       = sizeof(OPENFILENAME) ;
	ofn.hwndOwner         = NULL;
	ofn.hInstance         = NULL;
	ofn.lpstrFilter       = szFilter;
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter    = 0;
	ofn.nFilterIndex      = 0;
	ofn.lpstrFile         = NULL;
	ofn.nMaxFile          = MAX_PATH ;
	ofn.lpstrFileTitle    = NULL;
	ofn.nMaxFileTitle     = MAX_PATH ;
	ofn.lpstrInitialDir   = NULL;
	ofn.lpstrTitle        = NULL;
	ofn.Flags             = 0;
	ofn.nFileOffset       = 0;
	ofn.nFileExtension    = 0;
	ofn.lpstrDefExt       = TEXT("txt") ;
	ofn.lCustData         = 0L;
	ofn.lpfnHook          = NULL;
	ofn.lpTemplateName    = NULL;

	PTSTR pszFullPathName = new TCHAR[MAX_PATH];
	PTSTR pszFileName = new TCHAR[MAX_PATH];

	memset(pszFullPathName, 0, sizeof(MAX_PATH));
	memset(pszFileName, 0, sizeof(MAX_PATH));

	ofn.lpstrFile         = pszFullPathName;
	ofn.lpstrFileTitle    = pszFileName;
	ofn.Flags             = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_EXPLORER;
	if(GetOpenFileName(&ofn))
	{
		delete pszFileName;
		return pszFullPathName;
	}
	else
	{
		delete pszFullPathName;
		delete pszFileName;
		return NULL;
	}
}

bool AppData::Output()
{
	PTSTR szFileName = SelectOutputDlg();
	if(!szFileName)
		return false;
	std::ofstream outfile(szFileName);
	if(!outfile)
	{
		::MessageBox(NULL, szFileName, 
			_T("Error: Cannot open file for writing!"), MB_ICONERROR);
		return false;
	}
	outfile<<ST_OUTPUT_MARKING<<'\n';
	for(size_t i = 0; i < m_annot.size(); i++)
	{
		outfile<<m_annot[i].filepath<<'\n'<<m_annot[i].labels.size()<<'\n';
		for(size_t j = 0; j < m_annot[i].labels.size(); j++)
			outfile<<(size_t)m_annot[i].labels[j].lt<<' '
			<<m_annot[i].labels[j].rc.x<<' '
			<<m_annot[i].labels[j].rc.y<<' '
			<<m_annot[i].labels[j].rc.w<<' '
			<<m_annot[i].labels[j].rc.h<<' '
			<<m_annot[i].labels[j].score<<'\n';
	}
	outfile.close();
	delete szFileName;
	return true;
}