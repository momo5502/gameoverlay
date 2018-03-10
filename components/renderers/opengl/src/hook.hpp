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

	private:
		static hook* instance;

		std::optional<bool> glew_initialized;

		utils::hook swapBuffers_hook;

		std::function<void(HDC)> frame_callback;

		void frame_handler(HDC hdc);

		BOOL WINAPI swap_buffers(HDC hdc);
		static BOOL WINAPI swap_buffers_stub(HDC hdc);
	};
}
