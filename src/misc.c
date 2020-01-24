#include "resource.h"

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strsafe.h>
#include <wchar.h>

void ErrorHandling(HWND hwnd, WCHAR* text)
{
	MessageBoxW(hwnd, text, L"Error!", MB_OK | MB_ICONERROR);
}

void PrintLastError(HWND hwnd)
{
	SetDlgItemTextW(hwnd, IDL_LABEL, L"Failed!");
	printf("Error Code: %08x", GetLastError());
}

BOOL FileCopy(HWND hwnd, WCHAR* f, WCHAR* f1, WCHAR* f2, WCHAR* Debuf)
{
	int r = 0;
	int r1 = 0;
	int r2 = 0;
	int r3 = 0;

	SHFILEOPSTRUCTW filecopy;
	ZeroMemory(&filecopy, sizeof(filecopy));
	filecopy.hwnd = hwnd;
	filecopy.wFunc = FO_COPY;
	filecopy.fFlags = FOF_NOCONFIRMATION | FOF_SIMPLEPROGRESS;
	filecopy.lpszProgressTitle = L"Copying files...";
	filecopy.pTo = Debuf;


	if (wcslen(f) != 0)
	{
		filecopy.pFrom = f;
		r = SHFileOperationW(&filecopy);
		if (r != 0)
		{
			ErrorHandling(hwnd, L"Failed to copy bootcd!");
			printf("Error Code: %i", r);
			return FALSE;
		}
		filecopy.pFrom = NULL;
	}

	else if (wcslen(f1) != 0)
	{
		filecopy.pFrom = f1;
		r1 = SHFileOperationW(&filecopy);
		if (r1 != 0)
		{
			ErrorHandling(hwnd, L"Failed to copy livecd!");
			printf("Error Code: %i", r1);
			return FALSE;
		}
		filecopy.pFrom = NULL;
	}

	filecopy.pFrom = f2;
	r2 = SHFileOperationW(&filecopy);
	filecopy.pFrom = NULL;
	if (r2 != 0)
	{
		ErrorHandling(hwnd, L"Failed to copy freeldr.sys!");
		printf("Error Code: %i", r2);
		return FALSE;
	}

	return TRUE;
}

BOOL CreateINI(LPCWSTR lpszPath, LPCWSTR lpszFileName, LPCWSTR lpszSection, LPCWSTR lpszKey, LPCWSTR lpszString, HWND hwnd)
{
	HRESULT hResult = E_FAIL;
	BOOL bSuccess = FALSE;
	WCHAR szIniFileName[MAX_PATH] = { 0 };

	hResult = hResult = StringCchCatW(szIniFileName, _countof(szIniFileName), lpszPath);
	if (FAILED(hResult))
	{
		ErrorHandling(hwnd, L"StringCchCatW() failed!");
		return FALSE;
	}

	hResult = hResult = StringCchCatW(szIniFileName, _countof(szIniFileName), L"\\");
	if (FAILED(hResult))
	{
		ErrorHandling(hwnd, L"StringCchCatW() failed!");
		return FALSE;
	}

	hResult = hResult = StringCchCatW(szIniFileName, _countof(szIniFileName), lpszFileName);
	if (FAILED(hResult))
	{
		ErrorHandling(hwnd, L"StringCchCatW() failed!");
		return FALSE;
	}

	bSuccess = WritePrivateProfileStringW(lpszSection, lpszKey, lpszString, szIniFileName);

	if (!bSuccess)
	{
		ErrorHandling(hwnd, L"WritePrivateProfileStringW() failed!");
		return FALSE;
	}

	return TRUE;
}
