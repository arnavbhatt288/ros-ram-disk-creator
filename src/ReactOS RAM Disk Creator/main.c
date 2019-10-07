#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include "resource.h"
#include "install.h"
#include "volume.h"
#include "fat.h"
#include "fat32.h"

BOOL InstallBootSector(LPCWSTR lpszVolumeType, HWND hwnd);

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

	OPENFILENAMEW ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFilter = L"All Files (*.*)\0*.*\0";
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST;

	SHFILEOPSTRUCTW filecopy;
	ZeroMemory(&filecopy, sizeof(filecopy));
	filecopy.hwnd = hwnd;
	filecopy.wFunc = FO_COPY;
	filecopy.fFlags = FOF_NOCONFIRMATION | FOF_SIMPLEPROGRESS;
	filecopy.lpszProgressTitle = L"Copying files...";

	if(r == 0)
	{
		MessageBoxW(hwnd, L"Failed to get disk drives letter!", L"Error!", MB_OK | MB_ICONSTOP);
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
                	MessageBoxW(hwnd, L"Could not get ItemIndex!", L"Error!", MB_OK | MB_ICONSTOP);
                	return 0;
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
						MessageBoxW(hwnd, L"Select the file system option!", L"Error!", MB_OK | MB_ICONSTOP);
						return 0;
					}
					
					else if(len == 0 && len1 == 0)
					{
						MessageBoxW(hwnd, L"Select either bootcd or livecd!", L"Error!", MB_OK | MB_ICONSTOP);
						return 0;
					}

					else if(len2 == 0)
					{
						MessageBoxW(hwnd, L"Select the specified file!", L"Error!", MB_OK | MB_ICONSTOP);
						return 0;
					}

					else if(len3 == 0)
					{
						MessageBoxW(hwnd, L"Select the specified file!", L"Error!", MB_OK | MB_ICONSTOP);
						return 0;
					}

					filecopy.pTo = dBuf;
					wcstombs(adBuf, dBuf, MAX_PATH);
					memmove(&adBuf[2], &adBuf[2 + 1], strlen(adBuf) - 2);

					int msgboxID = MessageBox(hwnd, "Are you sure you have selected correct filesystem and files?","Warning", MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2);
					switch(msgboxID)
					{
						case IDYES:
							SetDlgItemText(hwnd, IDL_LABEL, "Installing Boot Sector...");
							OpenVolume(adBuf, hwnd);
							InstallBootSector(fsBuf, hwnd);
							CloseVolume();
							
							if(len != 0)
							{
								filecopy.pFrom = fBuf;
								int r = SHFileOperationW(&filecopy);
								if(r != 0)
								{
									MessageBoxW(hwnd, L"Failed to copy bootcd!", L"Error!", MB_OK | MB_ICONSTOP);
									char str[MAX_PATH];
									sprintf(str, "Error code: %i", r);
									MessageBoxA(hwnd, str, "Error!", MB_OK | MB_ICONSTOP);
									return 1;
								}
							}

							if(len1 != 0)
							{
								filecopy.pFrom = f1Buf;
								int r1 = SHFileOperationW(&filecopy);
								if(r1 != 0)
								{
									MessageBoxW(hwnd, L"Failed to copy bootcd!", L"Error!", MB_OK | MB_ICONSTOP);
									char str1[MAX_PATH];
									sprintf(str1, "Error code: %i", r1);
									MessageBoxA(hwnd, str1, "Error!", MB_OK | MB_ICONSTOP);
									return 1;
								}
							}

							filecopy.pFrom = f2Buf;
							int r2 = SHFileOperationW(&filecopy);
							if(r2 != 0)
							{
								MessageBoxW(hwnd, L"Failed to copy bootcd!", L"Error!", MB_OK | MB_ICONSTOP);
								char str2[MAX_PATH];
								sprintf(str2, "Error code: %i", r2);
								MessageBoxA(hwnd, str2, "Error!", MB_OK | MB_ICONSTOP);
								return 1;
							}
							
							filecopy.pFrom = f3Buf;
							int r3 = SHFileOperationW(&filecopy);
							if(r3 != 0)
							{
								MessageBoxW(hwnd, L"Failed to copy bootcd!", L"Error!", MB_OK | MB_ICONSTOP);
								char str3[MAX_PATH];
								sprintf(str3, "Error code: %i", r3);
								MessageBoxA(hwnd, str3, "Error!", MB_OK | MB_ICONSTOP);
								return 1;
							}

							SetDlgItemText(hwnd, IDL_LABEL, "Success!");
						break;

						case IDNO:
							return 0;
						break;
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
					ofn.lpstrFile = fBuf;
					GetOpenFileNameW(&ofn);
					SetDlgItemTextW(hwnd, IDE_BOOTCD, fBuf);
					ofn.lpstrFile = NULL;
				break;

				case IDB_SELECTB:
					ofn.lpstrFile = f1Buf;
					GetOpenFileNameW(&ofn);
					SetDlgItemTextW(hwnd, IDE_LIVECD, f1Buf);
					ofn.lpstrFile = NULL;
				break;

				case IDB_SELECTC:
					ofn.lpstrFile = f2Buf;
					GetOpenFileNameW(&ofn);
					SetDlgItemTextW(hwnd, IDE_FREELDRI, f2Buf);
					ofn.lpstrFile = NULL;
				break;

				case IDB_SELECTD:
					ofn.lpstrFile = f3Buf;
					GetOpenFileNameW(&ofn);
					SetDlgItemTextW(hwnd, IDE_FREELDRS, f3Buf);
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

    return DialogBoxW(hInstance, MAKEINTRESOURCEW(IDD_MAIN), NULL, MainDlgProc);
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
        MessageBoxW(hwnd, L"Unknown file type.", L"Error!", MB_OK | MB_ICONSTOP);
        return FALSE;
    }

    SetDlgItemText(hwnd, IDL_LABEL, "Copying Files...");

    return TRUE;
}
