#include "std_include.hpp"

#include "overlay.hpp"

BOOL APIENTRY DllMain(HMODULE /*module*/, DWORD callReason, LPVOID /*reserved*/)
{
	switch (callReason)
	{
		case DLL_PROCESS_ATTACH:
		{
			gameoverlay::overlay::initialize();
			break;
		}

		case DLL_PROCESS_DETACH:
		{
			gameoverlay::overlay::uninitialize();
			break;
		}

		default:
		{
			break;
		}
	}

	return TRUE;
}
