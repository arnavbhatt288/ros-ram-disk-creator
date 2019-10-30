#pragma warning(disable : 4995)

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strsafe.h>
#include <wchar.h>
#include <time.h>
#include "resource.h"
#include "install.h"
#include "volume.h"
#include "fat.h"
#include "fat32.h"

typedef struct INI_MData
{
	LPCWSTR lpszFileName;
	LPCWSTR lpszSectionName;
	LPCWSTR lpszKeyName;
	LPCWSTR lpszValueString;
} INI_MDATA, *PINI_MData;

INI_MDATA MyMainData[] =
{
	{L"freeldr.ini", L"Display", L"TitleText", L"ReactOS Setup"},
	{L"freeldr.ini", L"FREELOADER", L"TimeOut", L"5"},

	{L"freeldr.ini", L"Display", L"TitleText", L"ReactOS Setup"},
	{L"freeldr.ini", L"Display", L"StatusBarSetup", L"Cyan"},
	{L"freeldr.ini", L"Display", L"StatusBarTextColor", L"Black"},
	{L"freeldr.ini", L"Display", L"BackdropTextColor", L"White"},
	{L"freeldr.ini", L"Display", L"BackdropColor", L"Blue"},
	{L"freeldr.ini", L"Display", L"BackdropFillStyle", L"Medium"},
	{L"freeldr.ini", L"Display", L"TitleBoxTextColor", L"White"},
	{L"freeldr.ini", L"Display", L"TitleBoxColor", L"Red"},
	{L"freeldr.ini", L"Display", L"MessageBoxTextColor", L"White"},
	{L"freeldr.ini", L"Display", L"MessageBoxColor", L"Blue"},
	{L"freeldr.ini", L"Display", L"MenuTextColor", L"Gray"},
	{L"freeldr.ini", L"Display", L"MenuColor", L"Black"},
	{L"freeldr.ini", L"Display", L"TextColor", L"Gray"},
	{L"freeldr.ini", L"Display", L"SelectedTextColor", L"Black"},
	{L"freeldr.ini", L"Display", L"SelectedColor", L"Gray"},
	{L"freeldr.ini", L"Display", L"ShowTime", L"No"},
	{L"freeldr.ini", L"Display", L"MenuBox", L"No"},
	{L"freeldr.ini", L"Display", L"CenterMenu", L"No"},
	{L"freeldr.ini", L"Display", L"MinimalUI", L"Yes"},
	{L"freeldr.ini", L"Display", L"TimeText", L"Seconds until highlighted choice will be started automatically:"}
};

BOOL InstallBootSector(LPCWSTR lpszVolumeType, HWND hwnd);
BOOL CreateINI(LPCWSTR lpszPath, LPCWSTR lpszFileName, LPCWSTR lpszSection, LPCWSTR lpszKey, LPCWSTR lpszString, HWND hwnd);
BOOL FileCopy(HWND hwnd, WCHAR* f, WCHAR* f1, WCHAR* f2, WCHAR* Debuf);

void ErrorHandling(HWND hwnd, WCHAR* text);
void PrintLastError(HWND hwnd);
void Delay(unsigned int mseconds);

BOOL CALLBACK MainDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	WCHAR LogicalDrives[MAX_PATH] = {0};
	WCHAR* dPtr = NULL;
	WCHAR* d1Ptr = NULL;
	DWORD r = GetLogicalDriveStringsW(MAX_PATH, LogicalDrives);
	static WCHAR dBuf[MAX_PATH] = {0};
	static WCHAR fsBuf[MAX_PATH] = {0};
	static WCHAR fBuf[MAX_PATH] = {0};
	static WCHAR f1Buf[MAX_PATH] = {0};
	static WCHAR f2Buf[MAX_PATH] = {0};
	static WCHAR commandline[MAX_PATH] = {0};
	static WCHAR nBuf[MAX_PATH] = { 0 };
	char adBuf[MAX_PATH] = {0};
	int ItemIndex = 0;
	int len = 0;
	int len1 = 0;
	int len2 = 0;
	BOOL fAllow = FALSE;
	BOOL iSuccess = FALSE;
	LPDWORD dFlag = 0;

	OPENFILENAMEW ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST;

	STARTUPINFOW si;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);

	PROCESS_INFORMATION pi;
	ZeroMemory(&pi, sizeof(pi));

	if(r == 0)
	{
		ErrorHandling(hwnd, L"Failed to get disk drives letter!");
		PrintLastError(hwnd);
		return 1;
	}

	switch(Message)
	{
  		case WM_INITDIALOG:
			SendMessageW(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)LoadIconW(NULL, MAKEINTRESOURCEW(IDI_ICON_SMALL)));
			SendMessageW(hwnd, WM_SETICON, ICON_BIG, (LPARAM)LoadIconW(NULL, MAKEINTRESOURCEW(IDI_ICON)));

			if(r > 0 && r <= MAX_PATH)
			{
				WCHAR *SingleDrive = LogicalDrives;
				while(*SingleDrive)
				{
					SendMessageW(GetDlgItem(hwnd, IDL_DRIVE), CB_ADDSTRING, 0, (LPARAM)SingleDrive);
					SingleDrive += wcslen(SingleDrive) + 1;
				}
			
			}
		return TRUE;

		case WM_COMMAND:
			if (HIWORD(wParam) == CBN_SELCHANGE)
			{
				ItemIndex = SendMessageW(GetDlgItem(hwnd, IDL_DRIVE), CB_GETCURSEL, 0, 0);
				if (ItemIndex == CB_ERR)
				{
					ErrorHandling(hwnd, L"Could not get ItemIndex!");
					PrintLastError(hwnd);
					return 1;
				}
				SendMessageW(GetDlgItem(hwnd, IDL_DRIVE), CB_GETLBTEXT, (WPARAM)ItemIndex, (LPARAM)dBuf);
				dBuf[wcslen(dBuf) - 1] = '\0';
				BOOL vCheck = GetVolumeInformationW(dBuf, nBuf, MAX_PATH, NULL, NULL, dFlag, fsBuf, MAX_PATH);
				if (vCheck == FALSE)
				{
					ErrorHandling(hwnd, L"GetVolumeInformationW() failed!");
					PrintLastError(hwnd);;
					return 1;
				}
				else
				{
					SetDlgItemTextW(hwnd, IDL_LABEL_LETTER, dBuf);
					SetDlgItemTextW(hwnd, IDL_LABEL_FS, fsBuf);
					SetDlgItemTextW(hwnd, IDL_LABEL_NAME, nBuf);
				}
			}

			switch(LOWORD(wParam))
			{
				case IDB_START:
					len = wcslen(fBuf);
					len1 = wcslen(f1Buf);
					len2 = wcslen(f2Buf);

					if (wcslen(dBuf) == 0)
					{
						ErrorHandling(hwnd, L"Select the drive!");
						return 0;
					}

					if(len == 0 && len1 == 0)
					{
						ErrorHandling(hwnd, L"Select either bootcd or livecd or both!");
						return 0;
					}


					else if(len2 == 0)
					{
						ErrorHandling(hwnd, L"Select the freeldr.sys file!");
						return 0;
					}

					wcstombs(adBuf, dBuf, MAX_PATH);
					dPtr = wcsrchr(fBuf, L'\\');
					d1Ptr = wcsrchr(f1Buf, L'\\');
					dPtr++;
					d1Ptr++;

					if (SendDlgItemMessage(hwnd, IDC_FORMAT, BM_GETCHECK, 0, 0))
					{
						int msgboxID_F = MessageBoxW(hwnd, L"Selecting format drive will lose all of your data! Are you sure to format your USB?", L"Warning", MB_YESNO | MB_ICONSTOP);
						switch (msgboxID_F)
						{
						case IDYES:
							fAllow = TRUE;
							break;

						case IDNO:
							return 0;
						}
					}

					int msgboxID_M = MessageBoxW(hwnd, L"Are you sure that you have selected correct settings?", L"Warning", MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2);
					switch(msgboxID_M)
					{
						case IDYES:
							if (fAllow == TRUE)
							{
								SetDlgItemTextW(hwnd, IDL_LABEL, L"Formatting The Drive...");
								StringCbPrintfW(commandline, MAX_PATH, L"cmd /C format %S /fs:%ws /Q /V:REACTOS", adBuf, fsBuf);
								if (!CreateProcessW(NULL, commandline, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
								{
									ErrorHandling(hwnd, L"CreateProcessW() failed!");
									PrintLastError(hwnd);
									return 1;
								}
								
								WaitForSingleObject(pi.hProcess, INFINITE);
								CloseHandle(pi.hProcess);
								CloseHandle(pi.hThread);
							}

							SetDlgItemTextW(hwnd, IDL_LABEL, L"Installing Boot Sector...");
							OpenVolume(adBuf, hwnd);
							BOOL bsAllow = InstallBootSector(fsBuf, hwnd);
							if (bsAllow = TRUE)
							{
								CloseVolume();
							}

							else
							{
								ErrorHandling(hwnd, L"InstallBootSector() failed");
								PrintLastError(hwnd);
								return 1;
							}

							BOOL cSuccess = FileCopy(hwnd, fBuf, f1Buf, f2Buf, dBuf);
							if (cSuccess == TRUE)
							{
								SetDlgItemTextW(hwnd, IDL_LABEL, L"Generating INI File...");
								for (int i = 0; i < _countof(MyMainData); ++i)
								{
									CreateINI(dBuf, MyMainData[i].lpszFileName, MyMainData[i].lpszSectionName, MyMainData[i].lpszKeyName, MyMainData[i].lpszValueString, hwnd);
								}

								WCHAR dArr[MAX_PATH] = {0};
								WCHAR d1Arr[MAX_PATH] = {0};

								if (len1 == 0)
								{
									StringCbPrintfW(dArr, MAX_PATH, L"/RDPATH=%s", dPtr);
									iSuccess = CreateINI(dBuf, L"freeldr.ini", L"Operating Systems", L"Setup", L"\"Setup\"", hwnd);
									iSuccess = CreateINI(dBuf, L"freeldr.ini", L"Setup", L"BootType", L"ReactOSSetup", hwnd);
									iSuccess = CreateINI(dBuf, L"freeldr.ini", L"Setup", L"SystemPath", L"ramdisk(0)", hwnd);
									iSuccess = CreateINI(dBuf, L"freeldr.ini", L"Setup", L"Options", dArr, hwnd);
								}

								else if (len == 0)
								{
									StringCbPrintfW(d1Arr, MAX_PATH, L"/MININT /RDPATH=%s /RDEXPORTASCD", d1Ptr);
									iSuccess = CreateINI(dBuf, L"freeldr.ini", L"Operating Systems", L"LiveCD", L"\"LiveCD\"", hwnd);
									iSuccess = CreateINI(dBuf, L"freeldr.ini", L"LiveCD", L"BootType", L"Windows2003", hwnd);
									iSuccess = CreateINI(dBuf, L"freeldr.ini", L"LiveCD", L"SystemPath", L"ramdisk(0)\\reactos", hwnd);
									iSuccess = CreateINI(dBuf, L"freeldr.ini", L"LiveCD", L"Options", d1Arr, hwnd);
								}

								else
								{
									StringCbPrintfW(dArr, MAX_PATH, L"/RDPATH=%s", dPtr);
									StringCbPrintfW(d1Arr, MAX_PATH, L"/MININT /RDPATH=%s /RDEXPORTASCD", d1Ptr);
									iSuccess = CreateINI(dBuf, L"freeldr.ini", L"Operating Systems", L"Setup", L"\"Setup\"", hwnd);
									iSuccess = CreateINI(dBuf, L"freeldr.ini", L"Operating Systems", L"LiveCD", L"\"LiveCD\"", hwnd);
									iSuccess = CreateINI(dBuf, L"freeldr.ini", L"Setup", L"BootType", L"ReactOSSetup", hwnd);
									iSuccess = CreateINI(dBuf, L"freeldr.ini", L"Setup", L"SystemPath", L"ramdisk(0)", hwnd);
									iSuccess = CreateINI(dBuf, L"freeldr.ini", L"Setup", L"Options", dArr, hwnd);
									iSuccess = CreateINI(dBuf, L"freeldr.ini", L"LiveCD", L"BootType", L"Windows2003", hwnd);
									iSuccess = CreateINI(dBuf, L"freeldr.ini", L"LiveCD", L"SystemPath", L"ramdisk(0)\\reactos", hwnd);
									iSuccess = CreateINI(dBuf, L"freeldr.ini", L"LiveCD", L"Options", d1Arr, hwnd);
								}

							}
							
							else
							{
								SetDlgItemTextW(hwnd, IDL_LABEL, L"Failed!");
								return 1;
							}
							
							if (iSuccess == TRUE)
							{
								SetDlgItemTextW(hwnd, IDL_LABEL, L"Success!");

								Delay(3500);
								
								memset(dBuf, 0, sizeof dBuf);
								memset(adBuf, 0, sizeof adBuf);
								memset(fBuf, 0, sizeof fBuf); 
								memset(f1Buf, 0, sizeof f1Buf);
								memset(f2Buf, 0, sizeof f2Buf);
								memset(fsBuf, 0, sizeof fsBuf);
								memset(nBuf, 0, sizeof nBuf);
								
								SetDlgItemTextW(hwnd, IDL_DRIVE, L"");
								SetDlgItemTextW(hwnd, IDL_LABEL_LETTER, L"");
								SetDlgItemTextW(hwnd, IDL_LABEL_FS, L"");
								SetDlgItemTextW(hwnd, IDL_LABEL_NAME, L"");
								SetDlgItemTextW(hwnd, IDE_BOOTCD, L"");
								SetDlgItemTextW(hwnd, IDE_LIVECD, L"");
								SetDlgItemTextW(hwnd, IDE_FREELDRS, L"");

								SetDlgItemTextW(hwnd, IDL_LABEL, L"Standby");
								return 0;
							}
							
							else
							{
								SetDlgItemTextW(hwnd, IDL_LABEL, L"Failed!");
								return 1;
							}
						break;

						case IDNO:
							return 0;
					}

				break;

				case IDB_EXIT:
					memset(dBuf, 0, sizeof dBuf);
					memset(adBuf, 0, sizeof adBuf);
					memset(fBuf, 0, sizeof fBuf);
					memset(f1Buf, 0, sizeof f1Buf); 
					memset(f2Buf, 0, sizeof f2Buf);
					memset(LogicalDrives, 0, sizeof LogicalDrives);
					memset(fsBuf, 0, sizeof fsBuf);
					memset(nBuf, 0, sizeof nBuf); 
					EndDialog(hwnd, 0);
				break;

				case IDB_SELECTA:
					ofn.lpstrFilter = L"ISO File\0*.iso\0";
					ofn.lpstrFile = fBuf;
					GetOpenFileNameW(&ofn);
					SetDlgItemTextW(hwnd, IDE_BOOTCD, fBuf);
					ofn.lpstrFile = NULL;
					ofn.lpstrFilter = NULL;
				break;

				case IDB_SELECTB:
					ofn.lpstrFilter = L"ISO File\0*.iso\0";
					ofn.lpstrFile = f1Buf;
					GetOpenFileNameW(&ofn);
					SetDlgItemTextW(hwnd, IDE_LIVECD, f1Buf);
					ofn.lpstrFile = NULL;
					ofn.lpstrFilter = NULL;
				break;

				case IDB_SELECTC:
					ofn.lpstrFilter = L"SYS File\0*.sys\0";
					ofn.lpstrFile = f2Buf;
					GetOpenFileNameW(&ofn);
					SetDlgItemTextW(hwnd, IDE_FREELDRS, f2Buf);
					ofn.lpstrFile = NULL;
					ofn.lpstrFilter = NULL;
				break;
			}
		break;

		case WM_CLOSE:
			memset(dBuf, 0, sizeof dBuf);
			memset(adBuf, 0, sizeof adBuf);
			memset(fBuf, 0, sizeof fBuf);
			memset(LogicalDrives, 0, sizeof LogicalDrives);
			memset(fsBuf, 0, sizeof fsBuf);
			memset(nBuf, 0, sizeof nBuf);
			EndDialog(hwnd, 0);
		break;

		default:
			return FALSE;
    }
    return TRUE;
}

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
			return 0;
		}
	}
	
	DialogBoxW(hInstance, MAKEINTRESOURCEW(IDD_MAIN), NULL, MainDlgProc);

	if(obj)
	{ 
		CloseHandle(obj);
	}

	return 0;
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
			printf("%i", r);;
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
			printf("%i", r1);
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
		printf("%i", r2);
		return FALSE;
	}

	return TRUE;
}

BOOL CreateINI(LPCWSTR lpszPath, LPCWSTR lpszFileName, LPCWSTR lpszSection, LPCWSTR lpszKey, LPCWSTR lpszString, HWND hwnd)
{
	HRESULT hResult = E_FAIL;
	BOOL bSuccess = FALSE;
	WCHAR szIniFileName[MAX_PATH] =  {0};

	hResult = hResult = StringCchCatW(szIniFileName, _countof(szIniFileName), lpszPath);
	if (FAILED(hResult))
	{
		ErrorHandling(hwnd, L"StringCchCatW() failed!");
		printf("%i", GetLastError());
		return FALSE;
	}

	hResult = hResult = StringCchCatW(szIniFileName, _countof(szIniFileName), L"\\");
	if (FAILED(hResult))
	{
		ErrorHandling(hwnd, L"StringCchCatW() failed!");
		printf("%i", GetLastError());
		return FALSE;
	}

	hResult = hResult = StringCchCatW(szIniFileName, _countof(szIniFileName), lpszFileName);
	if (FAILED(hResult))
	{
		ErrorHandling(hwnd, L"StringCchCatW() failed!");
		printf("%i", GetLastError());
		return FALSE;
	}

	bSuccess = WritePrivateProfileStringW(lpszSection, lpszKey, lpszString, szIniFileName);

	if (!bSuccess)
	{
		ErrorHandling(hwnd, L"WritePrivateProfileStringW() failed!");
		printf("%i", GetLastError());
		return FALSE;
	}

	return TRUE;
}

BOOL InstallBootSector(LPCWSTR lpszVolumeType, HWND hwnd)
{
    BYTE    BootSectorBuffer[512];

    if (!ReadVolumeSector(0, BootSectorBuffer))
    {
        return FALSE;
    }

    if (_wcsicmp(lpszVolumeType, L"FAT") == 0)
    {
        memcpy((fat_data+3), (BootSectorBuffer+3), 59);

        if (!WriteVolumeSector(0, fat_data))
        {
            return FALSE;
        }
    }
    else if (_wcsicmp(lpszVolumeType, L"FAT32") == 0)
    {

        memcpy((fat32_data+3), (BootSectorBuffer+3), 87);

        if (!WriteVolumeSector(0, fat32_data))
        {
            return FALSE;
        }

        if (!WriteVolumeSector(14, (fat32_data+512)))
        {
            return FALSE;
        }
    }
    else
    {
		ErrorHandling(hwnd, L"Unknown filesystem type!");
        return FALSE;
    }

    SetDlgItemTextW(hwnd, IDL_LABEL, L"Copying Files...");

    return TRUE;
}

void ErrorHandling(HWND hwnd, WCHAR* text)
{
	MessageBoxW(hwnd, text, L"Error!", MB_OK | MB_ICONERROR);
}

void PrintLastError(HWND hwnd)
{
	printf("%i", GetLastError());
	SetDlgItemTextW(hwnd, IDL_LABEL, L"Failed!");
}

void Delay(unsigned int mseconds)
{
	clock_t goal = mseconds + clock();
	while (goal > clock());
}
