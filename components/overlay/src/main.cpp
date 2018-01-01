#include "std_include.hpp"

#include <backend.hpp>

#include <literally/io.hpp>
#include <literally/library.hpp>

using namespace literally;

namespace main
{
	void set_environment()
	{
		wchar_t exe_name[512];
		GetModuleFileName(GetModuleHandle(nullptr), exe_name, sizeof(exe_name) / 2);

		wchar_t* exe_base_name = wcsrchr(exe_name, L'\\');
		exe_base_name[0] = L'\0';

		SetCurrentDirectory(exe_name);
	}

	void initialize()
	{
		set_environment();

		std::vector<dynlib> libraries;
		for (auto& lib : "./backend/"_files.filter(".*\\.dll"))
		{
			libraries.push_back({ lib }); // Load the library
		}

		for (auto& lib : libraries)
		{
			auto create_backend = lib.get<gameoverlay::create_backend>("create_backend");
			if (create_backend)
			{
				gameoverlay::backend* backend = create_backend();
				if (backend->is_available())
				{
					// ..
				}
			}
		}
	}
}

BOOL APIENTRY DllMain(HMODULE /*module*/, DWORD callReason, LPVOID /*reserved*/)
{
	switch (callReason)
	{
		case DLL_PROCESS_ATTACH:
		{
			main::initialize();
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
