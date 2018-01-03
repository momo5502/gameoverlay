#include "std_include.hpp"

#include "renderer.hpp"

namespace gameoverlay
{
	renderer::renderer()
	{
		MessageBoxA(0, 0, 0, 0);
	}

	renderer::~renderer()
	{
		MessageBoxA(0, 0, 0, 0);
	}

	bool renderer::is_available()
	{
		return false;
	}

	HWND renderer::get_window()
	{
		return nullptr;
	}

	canvas* renderer::get_canvas()
	{
		return nullptr;
	}

	void renderer::register_frame_callback(std::function<void()> callback)
	{

	}

	extern "C" __declspec(dllexport) renderer* create_interface()
	{
		static renderer _renderer;
		return &_renderer;
	}
}