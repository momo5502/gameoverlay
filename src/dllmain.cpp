#include <std_include.hpp>
#include "main.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace {

/*****************************************************************************
 *
 ****************************************************************************/

DWORD WINAPI thread_main(LPVOID)
{
	gameoverlay::initialize();
	return 0;
}

}

/*****************************************************************************
 *
 ****************************************************************************/

BOOL WINAPI DllMain(const HINSTANCE handle, const DWORD reason, LPVOID)
{
	if (reason == DLL_PROCESS_ATTACH) {
		DisableThreadLibraryCalls(handle);
		CreateThread(nullptr, 0, thread_main, nullptr, 0, nullptr);
	}

	return TRUE;
}
