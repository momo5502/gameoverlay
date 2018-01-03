#include "std_include.hpp"
#include <MinHook.h>

BOOL APIENTRY DllMain(HMODULE /*module*/, DWORD callReason, LPVOID /*reserved*/)
{
	if (callReason == DLL_PROCESS_ATTACH) MH_Initialize();
	else if (callReason == DLL_PROCESS_DETACH) MH_Uninitialize();

	return TRUE;
}
