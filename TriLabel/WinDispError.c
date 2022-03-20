/* WinDispError.c

See WinDispError.h

Written by LeOpArD, 2012-10-17
*/

#include"WinDispError.h"

#ifndef WinDispError_Disp_File_Line_c
DWORD WinPerror(LPTSTR lpszDesc) 
{ 
    LPTSTR lpBuf;
	size_t strs;
    DWORD dwRes = GetLastError();
	if(FAILED(StringCchLength(lpszDesc, DESC_STR_LEN_LIMIT, &strs)))
	{
		_ftprintf(stderr, TEXT("WinPerror() Error: description message too long!\n"));
		return dwRes;
	}
	lpBuf = (LPTSTR)LocalAlloc(LMEM_FIXED, 256 * sizeof(TCHAR) + strs);
	if(!lpBuf)
	{
		_ftprintf(stderr, TEXT("WinPerror() Error: memory allocation failed using LocalAlloc()!\n"));
		return dwRes;
	}
	StringCchPrintf(
					lpBuf, 
					LocalSize(lpBuf)/sizeof(TCHAR),
					TEXT("%s sets (or leaves unchanged) error code %d as follows:\n"), 
					lpszDesc, 
					dwRes
					);
    if(!FormatMessage(
					FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
					NULL,
					dwRes,
					WinDispError_Language,
					lpBuf + _tcslen(lpBuf),
					LocalSize(lpBuf)/sizeof(TCHAR) - _tcslen(lpBuf), 
					NULL
				))
		_ftprintf(stderr, TEXT("FormatMessage() failed when outputing error message!\n"));
	else _ftprintf(stderr, lpBuf);
    LocalFree(lpBuf);
	return dwRes;
}

DWORD PopErrorBox(LPTSTR lpszDesc, HWND hWnd) 
{ 
    LPTSTR lpBuf = 0, lpCapBuf;
	size_t strs, txts;
    DWORD dwErrorTmp, dwRes = GetLastError();
	if(FAILED(StringCchLength(lpszDesc, DESC_STR_LEN_LIMIT, &strs)))
	{
		MessageBox(NULL, TEXT("Description message too long!"), 
				TEXT("PopErrorBox() Error!"), MB_ICONERROR);
		return dwRes;
	}
	txts = sizeof(TEXT(" Error!")) + 4;
	lpCapBuf = (LPTSTR)LocalAlloc(LMEM_FIXED, (strs + txts + 1) * sizeof(TCHAR)); // '\0'
	if(!lpCapBuf)
	{
		MessageBox(NULL, TEXT("Memory allocation failed using LocalAlloc()."), 
			TEXT("PopErrorBox() Error!"), MB_ICONERROR);
		return dwRes;
	}
	StringCchCopy(lpCapBuf, strs + txts, lpszDesc); 
	StringCchCat(lpCapBuf, strs + txts, TEXT(" Error!"));
	if(!FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, dwRes, WinDispError_Language, (LPTSTR)&lpBuf, 0, NULL))
	{
		dwErrorTmp = GetLastError();
		MessageBox(hWnd, TEXT("FormatMessage() failed when outputing error message!"), 
				TEXT("FormatMessage() Error!"), MB_ICONERROR);
		SetLastError(dwErrorTmp);
		// Get around the MessageBox() and leave the error code
		// of FormatMessage() to the caller to retrieve more info
	}
	else MessageBox(hWnd, lpBuf, lpCapBuf, MB_ICONERROR);
    LocalFree(lpCapBuf);
	LocalFree(lpBuf);
	return dwRes;
}

//BOOL HRPerror(HRESULT hr, LPTSTR lpszDesc) 
//{ 
//    LPTSTR lpBuf;
//	size_t strs;
//	if(FAILED(StringCchLength(lpszDesc, 4096, &strs)))
//	{
//		_ftprintf(stderr, TEXT("Error: WinPerror() description message too long!\n"));
//		return FALSE;
//	}
//	lpBuf = (LPTSTR)LocalAlloc(LMEM_FIXED, 256*sizeof(TCHAR) + strs);
//	StringCchPrintf(
//					lpBuf, 
//					LocalSize(lpBuf)/sizeof(TCHAR),
//					TEXT("%s returns HRESULT %ld as follows:\n"), 
//					lpszDesc, 
//					hr
//					);
//    if(!FormatMessage(
//					FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
//					NULL,
//					hr,
//					WinDispError_Language,
//					lpBuf + _tcslen(lpBuf),
//					LocalSize(lpBuf)/sizeof(TCHAR) - _tcslen(lpBuf), 
//					NULL
//				))
//		_ftprintf(stderr, TEXT("FormatMessage() failed when outputing error message!\n"));
//	else _ftprintf(stderr, lpBuf);
//    LocalFree(lpBuf);
//	return TRUE;
//}

#else // WinDispError_Disp_File_Line_c

DWORD WinPerror(LPTSTR lpszDesc) 
{ 
    LPTSTR lpBuf;
	size_t strs;
    DWORD dwRes = GetLastError();
	if(FAILED(StringCchLength(lpszDesc, 4096, &strs)))
	{
		_ftprintf(stderr, TEXT("Error: WinPerror() description message too long!\n"));
		return dwRes;
	}
	lpBuf = (LPTSTR)LocalAlloc(LMEM_FIXED, 256 * sizeof(TCHAR) + strs);
	StringCchPrintf(
					lpBuf, 
					LocalSize(lpBuf)/sizeof(TCHAR),
					TEXT("%s sets (or leaves unchanged) error code %d as follows:\n"), 
					lpszDesc, 
					dwRes
					);
    if(!FormatMessage(
					FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
					NULL,
					dwRes,
					WinDispError_Language,
					lpBuf + _tcslen(lpBuf),
					LocalSize(lpBuf)/sizeof(TCHAR) - _tcslen(lpBuf), 
					NULL
				))
		_ftprintf(stderr, TEXT("FormatMessage() failed when outputing error message!\n"));
	else _ftprintf(stderr, lpBuf);
    LocalFree(lpBuf);
	return dwRes;
}

DWORD PopErrorBox(LPTSTR lpszDesc) 
{ 
    LPTSTR lpBuf, lpCapBuf;
	size_t strs, txts;
    DWORD dwRes = GetLastError();
	if(FAILED(StringCchLength(lpszDesc, 256, &strs)))
	{
		MessageBox(NULL, TEXT("Description message too long!"), 
				TEXT("PopErrorBox() Error!"), MB_ICONERROR);
		return dwRes;
	}
	txts = sizeof(TEXT(" Error!")) + 4;
	lpCapBuf = (LPTSTR)LocalAlloc(LMEM_FIXED, strs + txts);
	StringCchCopy(lpCapBuf, strs + txts, lpszDesc); 
	StringCchCat(lpCapBuf, strs + txts, TEXT(" Error!"));

	if(!FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dwRes,
		WinDispError_Language,
        (LPTSTR)&lpBuf,
        0, 
        NULL ))
			MessageBox(NULL, TEXT("FormatMessage() failed when outputing error message!"), 
					TEXT("FormatMessage() Error!"), MB_ICONERROR);
	else MessageBox(NULL, lpBuf, lpCapBuf, MB_ICONERROR);
    LocalFree(lpCapBuf);
	LocalFree(lpBuf);
	return dwRes;
}

#endif // WinDispError_Disp_File_Line_c