/* WinDispError.h

This files contains a series of prototypes for displaying Windows API errors.
Implemented with strsafe functions, all functions here support
ANSI & UNICODE, detection of system language, buffer safety, and thread safety.

I. The GetLastError() family
(including Windows Sockets APIs)

DWORD WinStrError(LPTSTR*);		// the Windows API version strerror(); 
								// the string generated is stored in the private heap of the calling
								// thread and should be freed by LocalFree()
// For console applications:
DWORD WinPerror(LPTSTR);		// the Windows API version perror(); returns error number
DWORD WinPerrorNZ(LPTSTR);		// prints error message to console only when the error number is not 0; 
								// returns error number
// For windows applications:
DWORD PopErrorBox(LPTSTR, HWND);	// pop out a message box containing the error information
DWORD PopErrorBoxNZ(LPTSTR, HWND);	// pop out a message box only when the error number is not 0; 
									// returns error number

2. (To Be Included!) The HRESULT family (with the same functionality as the previous family)
(including DirectX APIs)

DWORD HRStrError(LPTSTR*);	
DWORD HRPerror(LPTSTR);	
DWORD HRPerrorNZ(LPTSTR);
DWORD HRPopErrorBox(LPTSTR);
DWORD HRPopErrorBoxNZ(LPTSTR);

Language Macro Options:

WinDispError_Lang_En		: English (user default,does not neccessarily refer to American English)
WinDispError_Lang_Chs		: Simplifided Chinese
(default)					: the function will search for the error message 
							for languages in the following order:
							(as in FormatMessage())
							1. Language neutral 
							2. Thread LANGID, based on the thread's locale value 
							3. User default LANGID, based on the user's default locale value 
							4. System default LANGID, based on the system default locale value 
							5. US English 
							6. Any available

Debug Macro Options:

WinDispError_Disp_File_Line	: Display the file name and the line number where the error occurs

Minimum client requirement: Windows XP with SP2

Adapted from MSDN
Written by LeOpArD, 2012-10-17
*/

#ifndef WinDispError_h
#define WinDispError_h

#ifdef __cplusplus   
extern "C"
{
#endif

#include<stdio.h>
#include<tchar.h>
#include<windows.h>
#include<strsafe.h>

#define DESC_STR_LEN_LIMIT	4096

#if defined(WinDispError_Lang_En)
	#define WinDispError_Language	MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT)
#elif defined(WinDispError_Lang_Chs)
	#define WinDispError_Language	MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED)
#else 
	#define WinDispError_Language	0
#endif

#ifndef WinDispError_Disp_File_Line

__inline DWORD WinStrError(LPTSTR *lpszBuf) 
{
    DWORD dwRes = GetLastError();
	*lpszBuf = 0;
	if(!FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dwRes,
		WinDispError_Language,
        (LPTSTR) lpszBuf,
        0, 
        NULL ))
		if(*lpszBuf)
		{
			LocalFree(*lpszBuf);
			*lpszBuf = 0;
		}
	return dwRes;
}

DWORD WinPerror(LPTSTR lpszDesc);

__inline DWORD WinPerrorNZ(LPTSTR lpszDesc) 
{ 
    if(GetLastError())
		return WinPerror(lpszDesc);
	else return 0;
}

DWORD PopErrorBox(LPTSTR lpszDesc, HWND hWnd);

__inline DWORD PopErrorBoxNZ(LPTSTR lpszDesc, HWND hWnd) 
{ 
    if(GetLastError())
		return PopErrorBox(lpszDesc, hWnd);
	else return 0;
}

#else // WinDispError_Disp_File_Line

#define WinDispError_Disp_File_Line_c

DWORD WinStrError(LPTSTR *lpszBuf, LPTSTR lpszFile, int nLine);

DWORD WinPerror(LPTSTR lpszDesc, LPTSTR lpszFile, int nLine);

__inline DWORD WinPerrorNZ(LPTSTR lpszDesc, LPTSTR lpszFile, int nLine) 
{ 
    if(GetLastError())
		return WinPerror(lpszDesc, LPTSTR lpszFile, int nLine);
	else return 0;
}

DWORD PopErrorBox(LPTSTR lpszDesc, LPTSTR lpszFile, int nLine);

__inline DWORD PopErrorBoxNZ(LPTSTR lpszDesc, LPTSTR lpszFile, int nLine) 
{ 
    if(GetLastError())
		return PopErrorBox(lpszDesc, LPTSTR lpszFile, int nLine);
	else return 0;
}

#define WinStrError(lpszBuf)		WinStrError(lpszBuf, __FILE__, __LINE__)
#define WinPerror(lpszDesc)			WinPerror(lpszDesc, __FILE__, __LINE__)
#define WinPerrorNZ(lpszDesc)		WinPerrorNZ(lpszDesc, __FILE__, __LINE__)
#define PopErrorBox(lpszDesc)		PopErrorBox(lpszDesc, __FILE__, __LINE__)
#define PopErrorBoxNZ(lpszDesc)		PopErrorBoxNZ(lpszDesc, __FILE__, __LINE__)

#endif // WinDispError_Disp_File_Line

//BOOL HRPerror(HRESULT hr, LPTSTR lpszDesc);
#ifdef __cplusplus   
} // extern "C"
#endif

#endif // WinDispError_h