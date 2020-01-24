
BOOL CreateINI(LPCWSTR lpszPath, LPCWSTR lpszFileName, LPCWSTR lpszSection, LPCWSTR lpszKey, LPCWSTR lpszString, HWND hwnd);
BOOL FileCopy(HWND hwnd, WCHAR* f, WCHAR* f1, WCHAR* f2, WCHAR* Debuf);

void ErrorHandling(HWND hwnd, WCHAR* text);
void PrintLastError(HWND hwnd);