
#include <windows.h>
#include <wchar.h>
#include <stdio.h>

#include "install.h"
#include "volume.h"
#include "misc.h"
#include "fat.h"
#include "fat32.h"

BOOL InstallBootSector(LPCWSTR lpszVolumeType, HWND hwnd)
{
	BYTE    BootSectorBuffer[512];

	if (!ReadVolumeSector(0, BootSectorBuffer))
	{
		return FALSE;
	}
	
	if (_wcsicmp(lpszVolumeType, L"FAT") == 0)
	{
		memcpy((fat_data + 3), (BootSectorBuffer + 3), 59);

		if (!WriteVolumeSector(0, fat_data))
		{
			return FALSE;
		}
	}
	else if (_wcsicmp(lpszVolumeType, L"FAT32") == 0)
	{

		memcpy((fat32_data + 3), (BootSectorBuffer + 3), 87);

		if (!WriteVolumeSector(0, fat32_data))
		{
			return FALSE;
		}

		if (!WriteVolumeSector(14, (fat32_data + 512)))
		{
			return FALSE;
		}
	}

	else
	{
		ErrorHandling(hwnd, L"Failed to install bootcode!");
		return FALSE;
	}

	return TRUE;
}
