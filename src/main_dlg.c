
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
#include "ini_data.h"

BOOL CALLBACK MainDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	WCHAR LogicalDrives[MAX_PATH] = { 0 };
	WCHAR* dPtr = NULL;
	WCHAR* d1Ptr = NULL;
	static WCHAR dBuf[MAX_PATH] = { 0 };
	static WCHAR fsBuf[MAX_PATH] = { 0 };
	static WCHAR fBuf[MAX_PATH] = { 0 };
	static WCHAR f1Buf[MAX_PATH] = { 0 };
	static WCHAR f2Buf[MAX_PATH] = { 0 };
	static WCHAR commandline[MAX_PATH] = { 0 };
	static WCHAR nBuf[MAX_PATH] = { 0 };
	char adBuf[MAX_PATH] = { 0 };
	int ItemIndex = 0;
	int len = 0;
	int len1 = 0;
	int len2 = 0;
	BOOL fAllow = FALSE;
	BOOL iSuccess = FALSE;
	LPDWORD dFlag = NULL;

	ULARGE_INTEGER ulF;
	ZeroMemory(&ulF, sizeof(ulF));

	ULARGE_INTEGER ulT;
	ZeroMemory(&ulT, sizeof(ulT));

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

	DWORD usbdrives = GetLogicalDriveStringsW(MAX_PATH, LogicalDrives);
	if (usbdrives == 0)
	{
		ErrorHandling(hwnd, L"Failed to get disk drives letter!");
		PrintLastError(hwnd);
		return FALSE;
	}

	switch (Message)
	{
	case WM_INITDIALOG:;
		SendMessageW(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)LoadIconW(NULL, MAKEINTRESOURCEW(IDI_ICON_SMALL)));
		SendMessageW(hwnd, WM_SETICON, ICON_BIG, (LPARAM)LoadIconW(NULL, MAKEINTRESOURCEW(IDI_ICON)));

		if (usbdrives > 0 && usbdrives <= MAX_PATH)
		{
			WCHAR* SingleDrive = LogicalDrives;
			while (*SingleDrive)
			{
				if (GetDriveTypeW(SingleDrive) == 2)
				{
					SendMessageW(GetDlgItem(hwnd, IDL_DRIVE), CB_ADDSTRING, 0, (LPARAM)SingleDrive);
				}
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

				return FALSE;
			}

			SendMessageW(GetDlgItem(hwnd, IDL_DRIVE), CB_GETLBTEXT, (WPARAM)ItemIndex, (LPARAM)dBuf);
			dBuf[wcslen(dBuf) - 1] = '\0';

			BOOL sCheck = GetDiskFreeSpaceExW(dBuf, NULL, &ulT, &ulF);
			if (sCheck == FALSE)
			{
				ErrorHandling(hwnd, L"GetDiskFreeSpaceExW() failed!");
				PrintLastError(hwnd);
				return FALSE;
			}

			else
			{
				UINT64 freeSpace = (UINT64)ulF.QuadPart / 1000000;
				UINT64 totalSpace = (UINT64)ulT.QuadPart / 1000000;
				if (totalSpace < 127)
				{
					ErrorHandling(hwnd, L"drive is smaller than minimum requirement!");
					return TRUE;
				}

				else if (freeSpace < totalSpace)
				{
					int msgboxID_LS = MessageBoxW(hwnd, L"The space of your drive is less than actual space. It is required to reformat it to FAT32. Continue?", L"Warning", MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2);
					switch (msgboxID_LS)
					{
					case IDYES:
						memset(fsBuf, 0, sizeof fsBuf);
						StringCbCopyW(fsBuf, MAX_PATH, L"FAT32");
						CheckDlgButton(hwnd, IDC_FORMAT, BST_CHECKED);

						SetDlgItemTextW(hwnd, IDL_LABEL_LETTER, dBuf);
						SetDlgItemTextW(hwnd, IDL_LABEL_FS, fsBuf);
						SetDlgItemTextW(hwnd, IDL_LABEL_NAME, nBuf);
						break;

					case IDNO:
						break;
					}
				}
			}

			BOOL vCheck = GetVolumeInformationW(dBuf, nBuf, MAX_PATH, NULL, NULL, dFlag, fsBuf, MAX_PATH);
			if (vCheck == FALSE)
			{
				ErrorHandling(hwnd, L"GetVolumeInformationW() failed!");
				PrintLastError(hwnd);
				return FALSE;
			}

			else
			{
				if (wcscmp(fsBuf, L"FAT") != 0 && wcscmp(fsBuf, L"FAT32") != 0)
				{
					int msgboxID_IC = MessageBoxW(hwnd, L"The filesystem of your drive is incompatible to the software. It is required to reformat it to FAT32. Continue?", L"Warning", MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2);

					switch (msgboxID_IC)
					{
					case IDYES:
						memset(fsBuf, 0, sizeof fsBuf);
						StringCbCopyW(fsBuf, MAX_PATH, L"FAT32");
						CheckDlgButton(hwnd, IDC_FORMAT, BST_CHECKED);

						SetDlgItemTextW(hwnd, IDL_LABEL_LETTER, dBuf);
						SetDlgItemTextW(hwnd, IDL_LABEL_FS, fsBuf);
						SetDlgItemTextW(hwnd, IDL_LABEL_NAME, nBuf);
						break;

					case IDNO:
						break;
					}
				}

				else
				{
					SetDlgItemTextW(hwnd, IDL_LABEL_LETTER, dBuf);
					SetDlgItemTextW(hwnd, IDL_LABEL_FS, fsBuf);
					SetDlgItemTextW(hwnd, IDL_LABEL_NAME, nBuf);
				}
			}

		}

		switch (LOWORD(wParam))
		{
		case IDC_DRIVE:
			SendDlgItemMessage(hwnd, IDL_DRIVE, CB_RESETCONTENT, 0, 0);

			if (SendDlgItemMessage(hwnd, IDC_DRIVE, BM_GETCHECK, 0, 0))
			{
				DWORD drives = GetLogicalDriveStringsW(MAX_PATH, LogicalDrives);
				if (drives == 0)
				{
					ErrorHandling(hwnd, L"Failed to get disk drives letter!");
					PrintLastError(hwnd);
					return FALSE;
				}

				if (drives > 0 && drives <= MAX_PATH)
				{
					WCHAR* SingleDrive = LogicalDrives;
					while (*SingleDrive)
					{
						SendMessageW(GetDlgItem(hwnd, IDL_DRIVE), CB_ADDSTRING, 0, (LPARAM)SingleDrive);
						SingleDrive += wcslen(SingleDrive) + 1;
					}
				}
			}

			else
			{
				DWORD usbdrives = GetLogicalDriveStringsW(MAX_PATH, LogicalDrives);
				if (usbdrives == 0)
				{
					ErrorHandling(hwnd, L"Failed to get disk drives letter!");
					PrintLastError(hwnd);
					return FALSE;
				}

				if (usbdrives > 0 && usbdrives <= MAX_PATH)
				{
					WCHAR* SingleDrive = LogicalDrives;
					while (*SingleDrive)
					{
						if (GetDriveTypeW(SingleDrive) == 2)
						{
							SendMessageW(GetDlgItem(hwnd, IDL_DRIVE), CB_ADDSTRING, 0, (LPARAM)SingleDrive);
						}
						SingleDrive += wcslen(SingleDrive) + 1;
					}
				}
			}
			break;

		case IDB_START:
			len = wcslen(fBuf);
			len1 = wcslen(f1Buf);
			len2 = wcslen(f2Buf);

			if (wcslen(dBuf) == 0)
			{
				ErrorHandling(hwnd, L"Select the drive!");
				return TRUE;
			}

			if (len == 0 && len1 == 0)
			{
				ErrorHandling(hwnd, L"Select either bootcd or livecd or both!");
				return TRUE;
			}


			else if (len2 == 0)
			{
				ErrorHandling(hwnd, L"Select the freeldr.sys file!");
				return TRUE;
			}

			wcstombs(adBuf, dBuf, MAX_PATH);
			dPtr = wcsrchr(fBuf, L'\\');
			d1Ptr = wcsrchr(f1Buf, L'\\');
			dPtr++;
			d1Ptr++;

			if (SendDlgItemMessage(hwnd, IDC_FORMAT, BM_GETCHECK, 0, 0))
			{
				int msgboxID_F = MessageBoxW(hwnd, L"Selecting format drive will lose all of your data! Are you sure to format your USB?", L"Warning", MB_YESNO | MB_ICONSTOP | MB_DEFBUTTON2);
				switch (msgboxID_F)
				{
				case IDYES:
					fAllow = TRUE;
					break;

				case IDNO:
					break;
				}
			}

			int msgboxID_M = MessageBoxW(hwnd, L"Are you sure that you have selected correct settings?", L"Warning", MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2);
			switch (msgboxID_M)
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
						return FALSE;
					}

					WaitForSingleObject(pi.hProcess, INFINITE);
					CloseHandle(pi.hProcess);
					CloseHandle(pi.hThread);
				}

				SetDlgItemTextW(hwnd, IDL_LABEL, L"Installing Boot Sector...");
				OpenVolume(adBuf, hwnd);
				BOOL bsAllow = InstallBootSector(fsBuf, hwnd);
				if (bsAllow == TRUE)
				{
					SetDlgItemTextW(hwnd, IDL_LABEL, L"Copying Files...");
					CloseVolume();
				}

				else
				{
					ErrorHandling(hwnd, L"InstallBootSector() failed");
					PrintLastError(hwnd);
					return FALSE;
				}

				BOOL cSuccess = FileCopy(hwnd, fBuf, f1Buf, f2Buf, dBuf);
				if (cSuccess == TRUE)
				{
					SetDlgItemTextW(hwnd, IDL_LABEL, L"Generating INI File...");
					for (int i = 0; i < _countof(MyMainData); ++i)
					{
						iSuccess = CreateINI(dBuf, MyMainData[i].lpszFileName, MyMainData[i].lpszSectionName, MyMainData[i].lpszKeyName, MyMainData[i].lpszValueString, hwnd);
						if (iSuccess == FALSE)
						{
							PrintLastError(hwnd);
							return FALSE;
						}
					}

					WCHAR dArr[MAX_PATH] = { 0 };
					WCHAR d1Arr[MAX_PATH] = { 0 };

					if (len1 == 0)
					{
						StringCbPrintfW(dArr, MAX_PATH, L"/RDPATH=%s", dPtr);
						iSuccess = CreateINI(dBuf, L"freeldr.ini", L"Operating Systems", L"Setup", L"\"Setup\"", hwnd);
						iSuccess = CreateINI(dBuf, L"freeldr.ini", L"Setup", L"BootType", L"ReactOSSetup", hwnd);
						iSuccess = CreateINI(dBuf, L"freeldr.ini", L"Setup", L"SystemPath", L"ramdisk(0)", hwnd);
						iSuccess = CreateINI(dBuf, L"freeldr.ini", L"Setup", L"Options", dArr, hwnd);
					}

					if (len == 0)
					{
						StringCbPrintfW(d1Arr, MAX_PATH, L"/MININT /RDPATH=%s /RDEXPORTASCD", d1Ptr);
						iSuccess = CreateINI(dBuf, L"freeldr.ini", L"Operating Systems", L"LiveCD", L"\"LiveCD\"", hwnd);
						iSuccess = CreateINI(dBuf, L"freeldr.ini", L"LiveCD", L"BootType", L"Windows2003", hwnd);
						iSuccess = CreateINI(dBuf, L"freeldr.ini", L"LiveCD", L"SystemPath", L"ramdisk(0)\\reactos", hwnd);
						iSuccess = CreateINI(dBuf, L"freeldr.ini", L"LiveCD", L"Options", d1Arr, hwnd);
					}

					if (len == 0 && len1 == 0)
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
					PrintLastError(hwnd);
					return FALSE;
				}

				if (iSuccess == TRUE)
				{
					SetDlgItemTextW(hwnd, IDL_LABEL, L"Success!");

					Sleep(3500);

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
					CheckDlgButton(hwnd, IDC_FORMAT, BST_UNCHECKED);

					SetDlgItemTextW(hwnd, IDL_LABEL, L"Standby");
					return TRUE;
				}

				else
				{
					PrintLastError(hwnd);
					return FALSE;
				}
				break;

			case IDNO:
				break;
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