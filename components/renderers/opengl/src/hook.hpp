#pragma once

#include <hook.hpp>

namespace gameoverlay
{
	class hook
	{
	public:
		hook();
		~hook();

		void on_frame(std::function<void(HDC)> callback);
		void unhook();

	private:
		static hook* instance;

		utils::hook swapBuffers_hook;

		std::function<void(HDC)> frame_callback;

		void frame_handler(HDC hdc);

		static BOOL WINAPI swapBuffers_stub(HDC hdc);
	};
}
