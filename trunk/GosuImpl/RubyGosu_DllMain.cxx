#include <windows.h>
#include <string>
using namespace std;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	// Append .so parent folder to PATH to make extensions (e.g. FMOD.dll) loadable.
	
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		TCHAR buffer[MAX_PATH+1];
		if (GetModuleFileName(hinstDLL, buffer, MAX_PATH+1))
		{
			// Make last backslash a \0 (get parent directory)
			TCHAR* ptr = buffer;
			while (*ptr) ++ptr;
			while (*ptr != '\\' && *ptr != '/')
				if (ptr < buffer)
					return TRUE;
				else
					--ptr;
			*ptr = 0;

			wstring oldPath = _wgetenv(L"PATH");
			wstring envString = L"PATH=" + oldPath + L';' + buffer;
			_wputenv(envString.c_str());
		}
	}
	return TRUE;
}