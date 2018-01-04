#include <dummy_window.hpp>

namespace gameoverlay
{
	namespace utils
	{
		dummy_window::dummy_window()
		{
			ZeroMemory(&this->window_class, sizeof(this->window_class));
			this->window_class.lpfnWndProc = DefWindowProc;
			this->window_class.hInstance = GetModuleHandle(nullptr);

			char name[100];
			_snprintf_s(name, sizeof(name), "dummy_window_%d%d", rand(), rand());
			this->window_class.lpszClassName = name;

			RegisterClassA(&this->window_class);
			this->window = CreateWindowExA(0, this->window_class.lpszClassName, "", WS_OVERLAPPED, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, this->window_class.hInstance, NULL);
		}

		dummy_window::~dummy_window()
		{
			DestroyWindow(this->window);
			UnregisterClassA(this->window_class.lpszClassName, this->window_class.hInstance);
		}

		HWND dummy_window::get()
		{
			return this->window;
		}

		dummy_window::operator HWND()
		{
			return this->get();
		}
	}
}