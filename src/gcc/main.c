#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include "resource.h"
#include "install.h"
#include "volume.h"
#include "fat.h"
#include "fat32.h"

BOOL InstallBootSector(LPCWSTR lpszVolumeType, HWND hwnd);

int FileCopy(HWND hwnd, WCHAR* f, WCHAR* f1, WCHAR* f2, WCHAR* f3, WCHAR* Debuf);

void ErrorHandling(HWND hwnd, WCHAR* text);

BOOL CALLBACK MainDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	WCHAR LogicalDrives[MAX_PATH] = {0};
	DWORD r = GetLogicalDriveStringsW(MAX_PATH, LogicalDrives);
	static WCHAR dBuf[MAX_PATH] = {0};
	static WCHAR *fsBuf = NULL;
	static WCHAR fBuf[MAX_PATH] = {0};
	static WCHAR f1Buf[MAX_PATH] = {0};
	static WCHAR f2Buf[MAX_PATH] = {0};
	static WCHAR f3Buf[MAX_PATH] = {0};
	char adBuf[MAX_PATH] = {0};
	int ItemIndex = 0;
	int len = 0;
	int len1 = 0;
	int len2 = 0;
	int len3 = 0;
	int success = 0;

	OPENFILENAMEW ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST;

	if(r == 0)
	{
		ErrorHandling(hwnd, L"Failed to get disk drives letter!");
		return 1;
	}

	switch(Message)
	{
  		case WM_INITDIALOG:
			SendMessageW(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)LoadIconW(NULL, MAKEINTRESOURCEW(IDI_ICON_SMALL)));
			SendMessageW(hwnd, WM_SETICON, ICON_BIG, (LPARAM)LoadIconW(NULL, MAKEINTRESOURCEW(IDI_APPLICATION)));

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
			if(HIWORD(wParam) == CBN_SELCHANGE)
			{
				ItemIndex = SendMessageW(GetDlgItem(hwnd, IDL_DRIVE), CB_GETCURSEL, 0, 0);
				if (ItemIndex == CB_ERR)
				{
					ErrorHandling(hwnd, L"Could not get ItemIndex!");
					return 1;
				}
				SendMessageW(GetDlgItem(hwnd, IDL_DRIVE), CB_GETLBTEXT, (WPARAM)ItemIndex, (LPARAM)dBuf);
			}

			switch(LOWORD(wParam))
			{
				case IDB_START:
					len = GetWindowTextLength(GetDlgItem(hwnd, IDE_BOOTCD));
					len1 = GetWindowTextLength(GetDlgItem(hwnd, IDE_LIVECD));
					len2 = GetWindowTextLength(GetDlgItem(hwnd, IDE_FREELDRI));
					len3 = GetWindowTextLength(GetDlgItem(hwnd, IDE_FREELDRS));
					if(!IsDlgButtonChecked(hwnd, IDR_FAT) && !IsDlgButtonChecked(hwnd, IDR_FAT32))
					{
						ErrorHandling(hwnd, L"Select the file system option!");
						return 0;
					}
					
					else if(len == 0 && len1 == 0)
					{
						ErrorHandling(hwnd, L"Select either bootcd or livecd or both!");
						return 0;
					}

					else if(len2 == 0)
					{
						ErrorHandling(hwnd, L"Select the freeldr.ini file!");
						return 0;
					}

					else if(len3 == 0)
					{
						ErrorHandling(hwnd, L"Select the freeldr.sys file!");
						return 0;
					}

					wcstombs(adBuf, dBuf, MAX_PATH);
					memmove(&adBuf[2], &adBuf[2 + 1], strlen(adBuf) - 2);

					int msgboxID = MessageBoxW(hwnd, L"Are you sure you have selected correct filesystem and files?", L"Warning", MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2);
					switch(msgboxID)
					{
						case IDYES:
							SetDlgItemTextW(hwnd, IDL_LABEL, L"Installing Boot Sector...");
							OpenVolume(adBuf, hwnd);
							InstallBootSector(fsBuf, hwnd);
							CloseVolume();
							
							FileCopy(hwnd, fBuf, f1Buf, f2Buf, f3Buf, dBuf);

							if(success == 0)
								SetDlgItemTextW(hwnd, IDL_LABEL, L"Success!");

							else
								SetDlgItemTextW(hwnd, IDL_LABEL, L"Failed!");
						break;

						case IDNO:
							return 0;
					}

				break;

				case IDB_EXIT:
					memset(dBuf, 0, sizeof dBuf);
					memset(adBuf, 0, sizeof adBuf);
					memset(fBuf, 0, sizeof fBuf);
					memset(LogicalDrives, 0, sizeof LogicalDrives);
					fsBuf = NULL;
					EndDialog(hwnd, 0);
				break;

				case IDR_FAT:
					CheckDlgButton(hwnd, IDR_FAT, BST_CHECKED);
					fsBuf = L"fat";
					CheckDlgButton(hwnd, IDR_FAT32, BST_UNCHECKED);
				break;

				case IDR_FAT32:
					CheckDlgButton(hwnd, IDR_FAT32, BST_CHECKED);
					fsBuf = L"fat32";
					CheckDlgButton(hwnd, IDR_FAT, BST_UNCHECKED);
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
					ofn.lpstrFilter = L"INI File\0*.ini\0";
					ofn.lpstrFile = f2Buf;
					GetOpenFileNameW(&ofn);
					SetDlgItemTextW(hwnd, IDE_FREELDRI, f2Buf);
					ofn.lpstrFile = NULL;
					ofn.lpstrFilter = NULL;
				break;

				case IDB_SELECTD:
					ofn.lpstrFilter = L"SYS File\0*.sys\0";
					ofn.lpstrFile = f3Buf;
					GetOpenFileNameW(&ofn);
					SetDlgItemTextW(hwnd, IDE_FREELDRS, f3Buf);
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
			fsBuf = NULL;
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

int FileCopy(HWND hwnd, WCHAR* f, WCHAR* f1, WCHAR* f2, WCHAR* f3, WCHAR* Debuf)
{
	WCHAR str[MAX_PATH];
	WCHAR str1[MAX_PATH];
	WCHAR str2[MAX_PATH];
	WCHAR str3[MAX_PATH];
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
	
	filecopy.pFrom = f;
	r = SHFileOperationW(&filecopy);
	filecopy.pFrom = NULL;
	if (r != 0)
	{
		ErrorHandling(hwnd, L"Failed to copy bootcd!");
		swprintf(str, MAX_PATH , L"Error code: %i", r);
		ErrorHandling(hwnd, str);
		return 1;
	}

	filecopy.pFrom = f1;
	r1 = SHFileOperationW(&filecopy);
	filecopy.pFrom = NULL;
	if (r1 != 0)
	{
		ErrorHandling(hwnd, L"Failed to copy livecd!");
		swprintf(str1, MAX_PATH , L"Error code: %i", r1);
		ErrorHandling(hwnd, str1);
		return 1;
	}

	filecopy.pFrom = f2;
	r2 = SHFileOperationW(&filecopy);
	filecopy.pFrom = NULL;
	if (r2 != 0)
	{
		ErrorHandling(hwnd, L"Failed to copy freeldr.ini!");
		swprintf(str2, MAX_PATH, L"Error code: %i", r2);
		ErrorHandling(hwnd, str2);
		return 1;
	}

	filecopy.pFrom = f3;
	r3 = SHFileOperationW(&filecopy);
	filecopy.pFrom = NULL;
	if (r3 != 0)
	{
		ErrorHandling(hwnd, L"Failed to copy freeldr.sys!");
		swprintf(str3, MAX_PATH, L"Error code: %i", r3);
		ErrorHandling(hwnd, str3);
		return 1;
	}

	return 0;
}

BOOL InstallBootSector(LPCWSTR lpszVolumeType, HWND hwnd)
{
    BYTE    BootSectorBuffer[512];

    if (!ReadVolumeSector(0, BootSectorBuffer))
    {
        return FALSE;
    }

    if (_wcsicmp(lpszVolumeType, L"fat") == 0)
    {
        memcpy((fat_data+3), (BootSectorBuffer+3), 59);

        if (!WriteVolumeSector(0, fat_data))
        {
            return FALSE;
        }
    }
    else if (_wcsicmp(lpszVolumeType, L"fat32") == 0)
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
