#include "std_include.hpp"
#include "overlay.hpp"

BOOL APIENTRY DllMain(HMODULE /*module*/, DWORD callReason, LPVOID /*reserved*/)
{
	switch (callReason)
	{
		case DLL_PROCESS_ATTACH:
		{
			// The current thread holds the loader lock.
			// However, DXGI can not operate while the loader lock is held.
			// Calling CreateThread is technically illegal as well, while the loader lock is held,
			// but nevertheless, it is safe and it works to maintain code-execution after the loader
			// lock has been unlocked.
			CreateThread(0, 0, static_cast<DWORD(__stdcall *)(void*)>([](void*)
			{
				gameoverlay::overlay::initialize();
				return 0ul;
			}), 0, 0, 0);
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
