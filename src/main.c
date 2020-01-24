#pragma warning(disable : 4995)
#define _UNICODE / UNICODE

#include "resource.h"

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strsafe.h>
#include <wchar.h>
#include <time.h>

#include "main_dlg.h"
#include "install.h"
#include "volume.h"
#include "misc.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPWSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);

	HANDLE obj;

	obj = CreateMutexW(NULL, FALSE, L"pMutex");

	if (obj)
	{
		DWORD err = GetLastError();

		if (err == ERROR_ALREADY_EXISTS)
		{
			MessageBoxW(NULL, L"Program is already running!", L"Error", MB_OK | MB_ICONSTOP);
			CloseHandle(obj);
			return TRUE;
		}
	}
	
	DialogBoxW(hInstance, MAKEINTRESOURCEW(IDD_MAIN), NULL, MainDlgProc);

	if(obj)
	{ 
		CloseHandle(obj);
	}

	return TRUE;
}
