#include "std_include.hpp"

#include <irenderer.hpp>

BOOL APIENTRY DllMain(HMODULE /*module*/, DWORD callReason, LPVOID /*reserved*/)
{
	switch (callReason)
	{
		case DLL_PROCESS_ATTACH:
		{

			break;
		}

		case DLL_PROCESS_DETACH:
		{
			break;
		}

		default:
		{
			break;
		}
	}

	return TRUE;
}
