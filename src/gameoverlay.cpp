#include <std_include.hpp>
#include <thread>

#include "backend_registry.hpp"

#include "utils/hook.hpp"

namespace gameoverlay
{
	namespace
	{
		utils::hook::detour destroy_window_hook;

		/*****************************************************************************
		 *
		 ****************************************************************************/

		BOOL WINAPI destroy_window_stub(const HWND window)
		{
			backend_registry::on_window_destruction(window);
			return destroy_window_hook.invoke_stdcall<BOOL>(window);
		}

		/*****************************************************************************
		 *
		 ****************************************************************************/

		void initialize()
		{
			destroy_window_hook.create(::DestroyWindow, destroy_window_stub);
			backend_registry::initialize();
		}

		/*****************************************************************************
		 *
		 ****************************************************************************/

		struct initializer
		{
			initializer()
			{
				std::thread([]
				{
					std::this_thread::sleep_for(std::chrono::seconds(1));
					initialize();
				}).detach();
			}
		} _;
	}
}
