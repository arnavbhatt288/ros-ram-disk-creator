
typedef struct INI_MData
{
	LPCWSTR lpszFileName;
	LPCWSTR lpszSectionName;
	LPCWSTR lpszKeyName;
	LPCWSTR lpszValueString;
} INI_MDATA, * PINI_MData;

INI_MDATA MyMainData[] =
{
	{L"freeldr.ini", L"FREELOADER", L"DefaultOS", L"Setup"},
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