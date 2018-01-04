#pragma once

#include <Windows.h>
#include <stdio.h>

namespace gameoverlay
{
	namespace utils
	{
		class dummy_window
		{
		public:
			dummy_window();
			~dummy_window();

			HWND get();
			operator HWND();

		private:
			HWND window;
			WNDCLASSA window_class;
		};
	}
}